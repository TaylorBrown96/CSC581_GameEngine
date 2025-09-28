#include "P2PNode.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

P2PNode::P2PNode() {
    ctx_ = std::make_unique<zmq::context_t>(1);
    req_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_REQ);
    subTracker_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_SUB);
    subPeers_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_SUB);
    pubPeers_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_PUB);
}
P2PNode::~P2PNode() { stop(); }

void P2PNode::setOnPeerMessage(MessageHandler cb) {
    std::lock_guard<std::mutex> lk(cbMtx_);
    onPeerMessage_ = std::move(cb);
}

bool P2PNode::start(const Config& cfg) {
    if (running_) return true;
    cfg_ = cfg;
    try {
        // bind pub on random port
        pubPeers_->bind("tcp://*:0");
        auto last = pubPeers_->get(zmq::sockopt::last_endpoint);
        myEndpoint_ = std::string(static_cast<const char*>(last.data()), last.size());
        if (auto pos = myEndpoint_.find("0.0.0.0"); pos != std::string::npos)
            myEndpoint_.replace(pos, 7, "127.0.0.1");

        // timeouts
        req_->set(zmq::sockopt::rcvtimeo, 1000);
        req_->set(zmq::sockopt::sndtimeo, cfg_.sendTimeoutMs);
        subTracker_->set(zmq::sockopt::subscribe, "");
        subTracker_->set(zmq::sockopt::rcvtimeo, cfg_.recvTimeoutMsTracker);
        subPeers_->set(zmq::sockopt::rcvtimeo, cfg_.recvTimeoutMsPeers);
        pubPeers_->set(zmq::sockopt::sndtimeo, cfg_.sendTimeoutMs);

        // connect tracker
        subTracker_->connect(cfg_.trackerPub);
        req_->connect(cfg_.trackerRep);

        // HELLO -> ASSIGN <id> <N> <id1> <ep1> <id2> <ep2> ...
        const std::string hello = "HELLO " + myEndpoint_;
        req_->send(zmq::buffer(hello), zmq::send_flags::none);

        zmq::message_t reply;
        auto ok = req_->recv(reply, zmq::recv_flags::none);
        if (!ok) { std::cerr << "[P2PNode] tracker no reply\n"; return false; }

        std::istringstream iss(std::string(static_cast<const char*>(reply.data()), reply.size()));
        std::string tag; int id; size_t nPeers = 0;
        iss >> tag >> id >> nPeers;
        if (tag != "ASSIGN") { std::cerr << "[P2PNode] bad assign\n"; return false; }
        id_ = id;

        // Connect to existing peers from snapshot (2-player: take the first)
        for (size_t i = 0; i < nPeers; ++i) {
            int pid; std::string ep;
            iss >> pid >> ep;
            if (pid == id_) continue;
            if (otherId_ == -1) {
                subPeers_->set(zmq::sockopt::subscribe, "");
                subPeers_->connect(ep);
                otherId_ = pid;
            }
        }
        if (otherId_ != -1) amAuthority_ = (id_ < otherId_);

        running_ = true;
        tTracker_ = std::thread(&P2PNode::trackerThreadFunc, this);
        tPeerRx_  = std::thread(&P2PNode::peerRxThreadFunc, this);

        std::cout << "[P2PNode] id=" << id_ << " endpoint=" << myEndpoint_
                  << (otherId_!=-1? " (connected to "+std::to_string(otherId_)+")": "")
                  << " authority=" << (amAuthority_?1:0) << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[P2PNode] start error: " << e.what() << std::endl;
        return false;
    }
}


void P2PNode::stop() {
    if (!running_) return;
    running_ = false;
    try { req_->close(); } catch (...) {}
    try { subTracker_->close(); } catch (...) {}
    try { subPeers_->close(); } catch (...) {}
    try { pubPeers_->close(); } catch (...) {}
    if (tTracker_.joinable()) tTracker_.join();
    if (tPeerRx_.joinable())  tPeerRx_.join();
}

void P2PNode::publish(const std::string& payload) {
    try { pubPeers_->send(zmq::buffer(payload), zmq::send_flags::none); }
    catch (...) {}
}

void P2PNode::trackerThreadFunc() {
    while (running_) {
        try {
            zmq::message_t msg;
            auto got = subTracker_->recv(msg, zmq::recv_flags::none);
            if (!got) continue;
            const std::string s(static_cast<const char*>(msg.data()), msg.size());

            if (s.rfind("JOIN ", 0) == 0) {
                std::istringstream is(s);
                std::string tag, endpoint; int pid;
                is >> tag >> pid >> endpoint;
                if (pid != id_ && otherId_ == -1) {
                    // connect to first seen peer
                    subPeers_->set(zmq::sockopt::subscribe, "");
                    subPeers_->connect(endpoint);
                    otherId_ = pid;
                    amAuthority_ = (id_ < otherId_);
                    std::cout << "[P2PNode] Connected to peer " << otherId_
                              << " (authority=" << amAuthority_ << ")\n";
                }
            }
        } catch (const std::exception& e) {
            if (running_) std::cerr << "[P2PNode] trackerThread error: " << e.what() << std::endl;
        }
    }
}
void P2PNode::peerRxThreadFunc() {
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
            if (running_) std::cerr << "[P2PNode] peerRx error: " << e.what() << std::endl;
        }
    }
}
