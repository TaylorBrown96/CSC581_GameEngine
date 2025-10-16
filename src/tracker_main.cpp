#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <mutex>

int main() {
    zmq::context_t ctx{1};
    zmq::socket_t rep(ctx, ZMQ_REP);
    zmq::socket_t pub(ctx, ZMQ_PUB);

    const int repPort = 6000, pubPort = 6001;
    rep.bind("tcp://*:" + std::to_string(repPort));
    pub.bind("tcp://*:" + std::to_string(pubPort));

    rep.set(zmq::sockopt::rcvtimeo, 100);
    std::cout << "[P2PTracker] REP tcp://*:" << repPort << "  PUB tcp://*:" << pubPort << "\n";
    std::unordered_map<int,std::string> peers;
    int nextId = 1000;
    std::mutex m;

    // heartbeat thread (optional)
    std::thread hb([&]{
        while (true) {
            try { pub.send(zmq::buffer("TRACKER_HEARTBEAT"), zmq::send_flags::none); }
            catch (...) {}
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    hb.detach();

    while (true) {
        zmq::message_t req;
        auto ok = rep.recv(req, zmq::recv_flags::none);
        if (!ok) continue;

        std::istringstream iss(std::string(static_cast<const char*>(req.data()), req.size()));
        std::string cmd, endpoint;
        iss >> cmd >> endpoint;

        if (cmd != "HELLO") {
            rep.send(zmq::buffer(std::string("ERROR expected_HELLO")), zmq::send_flags::none);
            continue;
        }

        int assigned;
        std::ostringstream reply;
        {
            std::lock_guard<std::mutex> lk(m);
            assigned = nextId++;
            // snapshot: N <id1> <ep1> ...
            reply << "ASSIGN " << assigned << " " << peers.size();
            for (auto& kv : peers) reply << " " << kv.first << " " << kv.second;
            peers[assigned] = endpoint;
        }
        rep.send(zmq::buffer(reply.str()), zmq::send_flags::none);

        std::ostringstream join; join << "JOIN " << assigned << " " << endpoint;
        pub.send(zmq::buffer(join.str()), zmq::send_flags::none);
        std::cout << "[P2PTracker] Assigned " << assigned << " -> " << endpoint << "\n";
    }
    return 0;
}
