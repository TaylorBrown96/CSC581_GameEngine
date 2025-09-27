#pragma once

#include <zmq.hpp>
// #include <iostream>
#include <string>
// #include <chrono>
// #include <thread>
// #include <Entity.h>
// #include <vec2.h>
// #include <vector>
#include <packetdef.h>
#include <GameEngine.h>


#define THREADS 1

class Server : public Entity {
    int loaded_map;
    zmq::context_t* context;
    zmq::socket_t sock;

    zmq::socket_t inp_sock;

   
    std::string endpoint;
    std::string port;
    std::string inp_port;
    
    
    int connected_clients;
    int proc_clients;

    std::vector<rr_packet> inpPackets;
    
    public:

    Server(std::string p_endpoint, std::string p_port, std::string p_inp_port, int map_type) : endpoint(p_endpoint), port(p_port), inp_port(p_inp_port), loaded_map(map_type) {
        SetOverseer();
        connected_clients = 0;
        proc_clients = 0;
    }
    
    void Initialize();

    void RecvInputOrConnectionPackets();

    void PublishUpdates(EntityManager* em);
    
    virtual void Update(float dt, InputManager* im, EntityManager* em) override {
        RecvInputOrConnectionPackets();
        PublishUpdates(em);

        /**
         * 
         * while (inpPackets.size() > 0) {
         * 
         *  rr_packet top = inpPackets.back();
         *  for (int i = 0; i < numConnectedPlayers; i++) {
         *  if (playerVector[i]->client_id == top.client_id) {
         *      if (top.keystate == K_KEYDOWN) {
         *          playerVector[i]->virtualKeyboard->SetKey(top.keycode, true);
         *      }
         *      else if (top.keystate == K_KEYUP) {
         *          playerVector[i]->virtualKeyboard->SetKey(top.keycode, false);
         *      }
         *  }
         * 
         *  inpPackets.pop_back();
         * }
         */
    }

};

