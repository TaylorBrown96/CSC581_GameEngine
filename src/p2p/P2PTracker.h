#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <mutex>
#include <string>
#include <zmq.hpp>
#include <vector>
#include <unordered_map>

// Minimal discovery "tracker" for P2P peer bootstrap.
// Peers send:   "HELLO <pub_endpoint>"
// Tracker replies: "ASSIGN <peer_id>"
// And broadcasts:  "JOIN <peer_id> <pub_endpoint>" on its PUB socket.
class P2PTracker {
public:
    P2PTracker();
    ~P2PTracker();

    // Bind REP (assignments) and PUB (announcements)
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
    // std::unordered_set<int> activePeers_;
    std::unordered_map<int, std::string> peers_;
};
