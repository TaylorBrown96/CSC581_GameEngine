#include "P2PTrackerV2.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

P2PTrackerV2::P2PTrackerV2() {
    ctx_ = std::make_unique<zmq::context_t>(1);
    rep_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_REP);
    pub_ = std::make_unique<zmq::socket_t>(*ctx_, ZMQ_PUB);
}
P2PTrackerV2::~P2PTrackerV2(){ stop(); }

bool P2PTrackerV2::start(int repPort, int pubPort) {
    if (running_) return true;
    try {
        const std::string repAddr = "tcp://*:" + std::to_string(repPort);
        const std::string pubAddr = "tcp://*:" + std::to_string(pubPort);
        rep_->bind(repAddr);
        pub_->bind(pubAddr);
        rep_->set(zmq::sockopt::rcvtimeo, 100);
        rep_->set(zmq::sockopt::sndtimeo, 100);
        pub_->set(zmq::sockopt::sndtimeo, 50);
        running_ = true;
        repThread_ = std::thread(&P2PTrackerV2::repLoop, this);
        heartbeatThread_ = std::thread(&P2PTrackerV2::heartbeatLoop, this);
        std::cout << "[P2PTrackerV2] REP " << repAddr << "  PUB " << pubAddr << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[P2PTrackerV2] start error: " << e.what() << std::endl;
        return false;
    }
}

void P2PTrackerV2::stop() {
    if (!running_) return;
    running_ = false;
    try { rep_->close(); } catch (...) {}
    try { pub_->close(); } catch (...) {}
    if (repThread_.joinable()) repThread_.join();
    if (heartbeatThread_.joinable()) heartbeatThread_.join();
}

void P2PTrackerV2::repLoop() {
    while (running_) {
        try {
            zmq::message_t req;
            auto ok = rep_->recv(req, zmq::recv_flags::none);
            if (!ok) continue;

            std::istringstream iss(std::string(static_cast<const char*>(req.data()), req.size()));
            std::string cmd, endpoint; iss >> cmd >> endpoint;
            if (cmd != "HELLO") {
                rep_->send(zmq::buffer(std::string("ERROR expected_HELLO")), zmq::send_flags::none);
                continue;
            }

            int assigned = -1;
            std::vector<std::pair<int,std::string>> snapshot;
            {
                std::lock_guard<std::mutex> lk(mtx_);
                assigned = nextId_++;
                snapshot.reserve(peers_.size());
                for (auto& kv : peers_) snapshot.emplace_back(kv.first, kv.second);
                peers_[assigned] = endpoint; // record new peer
            }

            // reply with snapshot
            std::ostringstream reply;
            reply << "ASSIGN " << assigned << " " << snapshot.size();
            for (auto& p : snapshot) reply << " " << p.first << " " << p.second;
            rep_->send(zmq::buffer(reply.str()), zmq::send_flags::none);

            // announce join
            std::ostringstream join;
            join << "JOIN " << assigned << " " << endpoint;
            pub_->send(zmq::buffer(join.str()), zmq::send_flags::none);

            std::cout << "[P2PTrackerV2] Assigned " << assigned << " -> " << endpoint << std::endl;
        } catch (const std::exception& e) {
            if (running_) std::cerr << "[P2PTrackerV2] repLoop error: " << e.what() << std::endl;
        }
    }
}

void P2PTrackerV2::heartbeatLoop() {
    using namespace std::chrono_literals;
    while (running_) {
        try { pub_->send(zmq::buffer(std::string("TRACKER_HEARTBEAT")), zmq::send_flags::none); }
        catch (...) {}
        std::this_thread::sleep_for(1000ms);
    }
}
