#include "P2PNodeV2.h"
#include <iostream>
#include <sstream>

P2PNodeV2::P2PNodeV2() {
    ctx_ = std::make_unique<zmq::context_t>(1);
    req_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_REQ);
    subTracker_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_SUB);
    subPeers_   = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_SUB);
    pubPeers_   = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_PUB);
}
P2PNodeV2::~P2PNodeV2(){ stop(); }

void P2PNodeV2::setOnPeerMessage(MessageHandler cb) {
    std::lock_guard<std::mutex> lk(cbMtx_); onPeerMessage_ = std::move(cb);
}

bool P2PNodeV2::start(const Config& cfg) {
    if (running_) return true;
    cfg_ = cfg;
    try {
        pubPeers_->bind("tcp://*:0");
        auto last = pubPeers_->get(zmq::sockopt::last_endpoint);
        myEndpoint_ = std::string(static_cast<const char*>(last.data()), last.size());
        if (auto pos = myEndpoint_.find("0.0.0.0"); pos != std::string::npos)
            myEndpoint_.replace(pos, 7, "127.0.0.1");

        req_->set(zmq::sockopt::rcvtimeo, 1000);
        req_->set(zmq::sockopt::sndtimeo, cfg_.sendTimeoutMs);
        subTracker_->set(zmq::sockopt::subscribe, "");
        subTracker_->set(zmq::sockopt::rcvtimeo, cfg_.recvTimeoutMsTracker);
        subPeers_->set(zmq::sockopt::rcvtimeo, cfg_.recvTimeoutMsPeers);
        pubPeers_->set(zmq::sockopt::sndtimeo, cfg_.sendTimeoutMs);

        subTracker_->connect(cfg_.trackerPub);
        req_->connect(cfg_.trackerRep);

        // HELLO -> ASSIGN <id> <N> <id1> <ep1> ...
        const std::string hello = "HELLO " + myEndpoint_;
        req_->send(zmq::buffer(hello), zmq::send_flags::none);

        zmq::message_t reply;
        auto ok = req_->recv(reply, zmq::recv_flags::none);
        if (!ok) { std::cerr << "[P2PNodeV2] tracker no reply\n"; return false; }

        std::istringstream iss(std::string(static_cast<const char*>(reply.data()), reply.size()));
        std::string tag; int id; size_t n = 0;
        iss >> tag >> id >> n;
        if (tag != "ASSIGN") { std::cerr << "[P2PNodeV2] bad assign\n"; return false; }
        id_ = id;

        // Connect to existing peers from snapshot (2-player: first is enough)
        for (size_t i=0;i<n;++i) {
            int pid; std::string ep; iss >> pid >> ep;
            if (pid == id_) continue;
            if (otherId_ == -1) {
                subPeers_->set(zmq::sockopt::subscribe, "");
                subPeers_->connect(ep);
                otherId_ = pid;
            }
        }

        running_ = true;
        tTracker_ = std::thread(&P2PNodeV2::trackerThreadFunc, this);
        tPeerRx_  = std::thread(&P2PNodeV2::peerRxThreadFunc, this);

        std::cout << "[P2PNodeV2] id=" << id_ << " endpoint=" << myEndpoint_
                  << (otherId_!=-1? " (connected to "+std::to_string(otherId_)+")": "") << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[P2PNodeV2] start error: " << e.what() << std::endl;
        return false;
    }
}

void P2PNodeV2::stop() {
    if (!running_) return;
    running_ = false;
    try { req_->close(); } catch(...) {}
    try { subTracker_->close(); } catch(...) {}
    try { subPeers_->close(); } catch(...) {}
    try { pubPeers_->close(); } catch(...) {}
    if (tTracker_.joinable()) tTracker_.join();
    if (tPeerRx_.joinable())  tPeerRx_.join();
}

void P2PNodeV2::publish(const std::string& payload) {
    try { pubPeers_->send(zmq::buffer(payload), zmq::send_flags::none); } catch(...) {}
}

void P2PNodeV2::trackerThreadFunc() {
    while (running_) {
        try {
            zmq::message_t msg;
            auto got = subTracker_->recv(msg, zmq::recv_flags::none);
            if (!got) continue;
            std::string s(static_cast<const char*>(msg.data()), msg.size());
            if (s.rfind("JOIN ", 0) == 0) {
                std::istringstream is(s);
                std::string tag, ep; int pid;
                is >> tag >> pid >> ep;
                if (pid != id_ && otherId_ == -1) {
                    subPeers_->set(zmq::sockopt::subscribe, "");
                    subPeers_->connect(ep);
                    otherId_ = pid;
                    std::cout << "[P2PNodeV2] Connected to new peer " << otherId_ << std::endl;
                }
            }
        } catch (const std::exception& e) {
            if (running_) std::cerr << "[P2PNodeV2] tracker error: " << e.what() << std::endl;
        }
    }
}

void P2PNodeV2::peerRxThreadFunc() {
    while (running_) {
        try {
            zmq::message_t m;
            auto g = subPeers_->recv(m, zmq::recv_flags::none);
            if (!g) continue;
            const std::string payload(static_cast<const char*>(m.data()), m.size());
            MessageHandler cb;
            { std::lock_guard<std::mutex> lk(cbMtx_); cb = onPeerMessage_; }
            if (cb) cb(payload);
        } catch (const std::exception& e) {
            if (running_) std::cerr << "[P2PNodeV2] peerRx error: " << e.what() << std::endl;
        }
    }
}
