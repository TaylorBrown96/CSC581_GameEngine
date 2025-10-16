#include "P2PTracker.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

P2PTracker::P2PTracker() {
    ctx_ = std::make_unique<zmq::context_t>(1);
    rep_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_REP);
    pub_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_PUB);
}
P2PTracker::~P2PTracker() { stop(); }

bool P2PTracker::start(int repPort, int pubPort) {
    if (running_) return true;
    try {
        const std::string repAddr = "tcp://*:" + std::to_string(repPort);
        const std::string pubAddr = "tcp://*:" + std::to_string(pubPort);
        rep_->bind(repAddr);
        pub_->bind(pubAddr);

        // Non-blocking-ish with timeouts
        rep_->set(zmq::sockopt::rcvtimeo, 100);
        rep_->set(zmq::sockopt::sndtimeo, 100);
        pub_->set(zmq::sockopt::sndtimeo, 50);

        running_ = true;
        repThread_ = std::thread(&P2PTracker::repLoop, this);
        heartbeatThread_ = std::thread(&P2PTracker::heartbeatLoop, this);
        std::cout << "[P2PTracker] REP " << repAddr << "  PUB " << pubAddr << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[P2PTracker] start error: " << e.what() << std::endl;
        return false;
    }
}
void P2PTracker::stop() {
    if (!running_) return;
    running_ = false;
    try { rep_->close(); } catch (...) {}
    try { pub_->close(); } catch (...) {}
    if (repThread_.joinable()) repThread_.join();
    if (heartbeatThread_.joinable()) heartbeatThread_.join();
}

void P2PTracker::repLoop() {
    while (running_) {
        try {
            zmq::message_t req;
            auto ok = rep_->recv(req, zmq::recv_flags::none);
            if (!ok) continue;
            const std::string msg(static_cast<const char*>(req.data()), req.size());

            std::istringstream iss(msg);
            std::string cmd, endpoint;
            iss >> cmd >> endpoint;
            if (cmd != "HELLO") {
                const std::string err = "ERROR expected_HELLO";
                rep_->send(zmq::buffer(err), zmq::send_flags::none);
                continue;
            }

            int assigned = -1;
            std::vector<std::pair<int,std::string>> snapshot;
            {
                std::lock_guard<std::mutex> lk(mtx_);
                assigned = nextId_++;
                // build snapshot of current peers BEFORE adding the new one
                snapshot.reserve(peers_.size());
                for (auto& kv : peers_) snapshot.emplace_back(kv.first, kv.second);
                // record new peer
                peers_[assigned] = endpoint;
            }

            // reply: ASSIGN <id> <N> <id1> <ep1> <id2> <ep2> ...
            std::ostringstream reply;
            reply << "ASSIGN " << assigned << " " << snapshot.size();
            for (auto& p : snapshot) reply << " " << p.first << " " << p.second;
            rep_->send(zmq::buffer(reply.str()), zmq::send_flags::none);

            // announce the new peer to everyone
            std::ostringstream join;
            join << "JOIN " << assigned << " " << endpoint;
            pub_->send(zmq::buffer(join.str()), zmq::send_flags::none);

            std::cout << "[P2PTracker] Assigned " << assigned << " -> " << endpoint << std::endl;
        } catch (const std::exception& e) {
            if (running_) std::cerr << "[P2PTracker] repLoop error: " << e.what() << std::endl;
        }
    }
}
void P2PTracker::heartbeatLoop() {
    using namespace std::chrono_literals;
    while (running_) {
        try {
            const std::string hb = "TRACKER_HEARTBEAT";
            pub_->send(zmq::buffer(hb), zmq::send_flags::none);
        } catch (...) {}
        std::this_thread::sleep_for(1000ms);
    }
}
