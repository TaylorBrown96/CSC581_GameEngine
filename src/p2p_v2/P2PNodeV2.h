#pragma once
#include <zmq.hpp>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

class P2PNodeV2 {
public:
    struct Config {
        std::string trackerRep = "tcp://127.0.0.1:6000";
        std::string trackerPub = "tcp://127.0.0.1:6001";
        int recvTimeoutMsTracker = 100;
        int recvTimeoutMsPeers   = 10;
        int sendTimeoutMs        = 10;
    };

    using MessageHandler = std::function<void(const std::string&)>;

    P2PNodeV2();
    ~P2PNodeV2();

    bool start(const Config& cfg = Config{});
    void stop();

    void publish(const std::string& payload);
    void setOnPeerMessage(MessageHandler cb);

    // identity
    int  peerId() const { return id_; }
    int  otherId() const { return otherId_; }
    bool isRunning() const { return running_; }

private:
    void trackerThreadFunc();
    void peerRxThreadFunc();

    std::unique_ptr<zmq::context_t> ctx_;
    std::unique_ptr<zmq::socket_t> req_;
    std::unique_ptr<zmq::socket_t> subTracker_;
    std::unique_ptr<zmq::socket_t> subPeers_;
    std::unique_ptr<zmq::socket_t> pubPeers_;

    std::thread tTracker_;
    std::thread tPeerRx_;
    std::atomic<bool> running_{false};

    int id_{-1};
    int otherId_{-1};
    std::string myEndpoint_;

    Config cfg_;
    std::mutex cbMtx_;
    MessageHandler onPeerMessage_;
};
