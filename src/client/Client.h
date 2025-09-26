
#pragma once
#include <zmq.hpp>

#include <packetdef.h>
#include <GameEngine.h>
#include <iostream>


class Client : public Entity {
    zmq::context_t con;

    // sends input
    zmq::socket_t sock;
    std::string update_endpoint;

    // gets update
    zmq::socket_t upd_sock;
    std::string input_endpoint;

    std::vector<ps_packet> packQueue;
    public:

    
    Client();

    void initClient(std::string p_endpoint, std::string port, std::string inp_port);

    void ConnectInit(int* map_type);

    void sendRRPacket(rr_packet* pack);

    void RecvRRPacket(rr_packet* pack);

    void QueueUpdatePackets();

    void ApplyQueueUpdatePacketsToState(EntityManager* em);

    virtual void Update(float dt, InputManager* in, EntityManager* em) override;
    

};

