#pragma once
#include <zmq.hpp>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <unordered_map>

// Reusable client-side template class a game can subclass/use.
// Responsibilities:
//  - Binds a local PUB on random port (data out)
//  - Handshakes with tracker to get peer id
//  - Subscribes to tracker JOIN to learn others' PUB endpoints
//  - Connects a SUB to peers (data in)
//  - Provides minimal publish()/onMessage() hooks for your game layer
class P2PNode {
public:
    struct Config {
        std::string trackerRep;
        std::string trackerPub;
        int recvTimeoutMsTracker;
        int recvTimeoutMsPeers;
        int sendTimeoutMs;
        
        Config() : trackerRep("tcp://127.0.0.1:6000"), 
                   trackerPub("tcp://127.0.0.1:6001"),
                   recvTimeoutMsTracker(100),
                   recvTimeoutMsPeers(10),
                   sendTimeoutMs(10) {}
    };

    using MessageHandler = std::function<void(const std::string&)>;

    P2PNode();
    ~P2PNode();

    int otherId() const { return otherId_; }

    bool start(const Config& cfg = Config{});
    void stop();

    // Publish a string payload to all peers (via this node's PUB)
    void publish(const std::string& payload);

    // Receive callback for peer messages (set before start)
    void setOnPeerMessage(MessageHandler cb);

    // Identity
    int  peerId() const { return id_; }
    bool isRunning() const { return running_; }
    bool isAuthority() const { return amAuthority_; } // lowest id of first two peers

private:
    void trackerThreadFunc();
    void peerRxThreadFunc();

    // sockets/context
    std::unique_ptr<zmq::context_t> ctx_;
    std::unique_ptr<zmq::socket_t> req_;
    std::unique_ptr<zmq::socket_t> subTracker_;
    std::unique_ptr<zmq::socket_t> subPeers_;
    std::unique_ptr<zmq::socket_t> pubPeers_;

    // threading
    std::thread tTracker_;
    std::thread tPeerRx_;
    std::atomic<bool> running_{false};

    // identity
    int id_{-1};
    int otherId_{-1};
    bool amAuthority_{false}; // True if id_ < otherId_
    std::string myEndpoint_;

    // config & callback
    Config cfg_;
    std::mutex cbMtx_;
    MessageHandler onPeerMessage_;
};
