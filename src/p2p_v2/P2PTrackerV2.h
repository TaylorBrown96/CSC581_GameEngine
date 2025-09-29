#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
#include <zmq.hpp>

// Discovery tracker that replies with a snapshot of current peers.
// REQ: "HELLO <pub_endpoint>"
// REP: "ASSIGN <id> <N> <id1> <ep1> ..."
// PUB: "JOIN <id> <pub_endpoint>"
class P2PTrackerV2 {
public:
    P2PTrackerV2();
    ~P2PTrackerV2();

    bool start(int repPort = 6000, int pubPort = 6001);
    void stop();

private:
    void repLoop();
    void heartbeatLoop();

    std::unique_ptr<zmq::context_t> ctx_;
    std::unique_ptr<zmq::socket_t> rep_;
    std::unique_ptr<zmq::socket_t> pub_;
    std::thread repThread_;
    std::thread heartbeatThread_;
    std::atomic<bool> running_{false};

    std::mutex mtx_;
    int nextId_{1000};
    std::unordered_map<int, std::string> peers_; // id -> endpoint
};
