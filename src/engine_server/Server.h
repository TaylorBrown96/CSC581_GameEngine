#pragma once

#include <zmq.hpp>
#include <iostream>
#include <string>
// #include <chrono>
// #include <thread>
// #include <Entity.h>
// #include <vec2.h>
// #include <vector>
#include <packetdef.h>
#include <GameEngine.h>


#define THREADS 1


class PlayerEntity : public Entity {
    InputManager* virtualInputManager = nullptr;
public:
    InputManager* getVirtInputManager() {
        return virtualInputManager;
    }
    
    // PlayerEntity(PlayerEntity* other) = default;

    PlayerEntity(float x, float y, float w, float h) : Entity(x, y, w, h) {
        
        // virtualInputManager = new InputManager(keys);
    }

    virtual void ServerUpdate(float dt, InputManager* in, EntityManager* en) {
    }
    virtual void Update(float dt, InputManager* in, EntityManager* en) override {
        ServerUpdate(dt, virtualInputManager, en);
    }
    
    void SetNumKeys(int numkeys) {
        if (virtualInputManager) {
            delete(virtualInputManager);
            virtualInputManager = nullptr;
        }
        virtualInputManager = new InputManager(numkeys);
    }
};

template <typename PlayerType>
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

    std::vector<PlayerType*> playerVector;
    int numConnectedPlayers;

    public:
    PlayerType* SpawnPrototypeOnJoin;
    
    // template <typename PlayerType>
    ~Server() {
        std::cout<<"Closing.\n";
        sock.close();
        inp_sock.close();
        context->close();
    }
    
    
    // template <typename PlayerType>
    Server(std::string p_endpoint, std::string p_port, std::string p_inp_port, int map_type) : endpoint(p_endpoint), port(p_port), inp_port(p_inp_port), loaded_map(map_type) {
        SetOverseer();
        numConnectedPlayers = 0;
    }
    
    void Initialize() {
        context = new zmq::context_t(THREADS);
        sock = zmq::socket_t(*context, zmq::socket_type::pub);
        sock.bind(endpoint+":"+port);

        inp_sock = zmq::socket_t(*context, zmq::socket_type::rep);
        inp_sock.bind(endpoint+":"+inp_port);
    }

    void RecvInputOrConnectionPackets(EntityManager* em) {
         zmq::message_t req(sizeof(rr_packet));
        
        rr_packet* p = (rr_packet*)req.data();
        p->packet_type = -2;
        inp_sock.recv(req, zmq::recv_flags::dontwait);

        if (p->packet_type != -2) {
            rr_packet* p_request = (rr_packet*)req.data();

            if (p_request->packet_type == P_CLIENT_HELLO) {
                
                PlayerType* newplayer = new PlayerType(*SpawnPrototypeOnJoin);
                if (dynamic_cast<PlayerEntity*>(newplayer)) {
                    
                    newplayer->client_id = p_request->client_id;
                    
                    newplayer->SetNumKeys(p_request->numkeys);
                    
                    em->AddEntity(newplayer);
                    
                    playerVector.push_back(newplayer);
                    
                    numConnectedPlayers = playerVector.size();
                }
                // std::cout<<"B\n";
                // send map to the client
                
                zmq::message_t rep(sizeof(rr_packet));
                rr_packet* rp = (rr_packet*)rep.data();
                rp->packet_type = loaded_map;
                inp_sock.send(rep, zmq::send_flags::dontwait);
                // send other connected clients

                // 
            }

            if (p_request->packet_type == P_CLIENT_INPUT) {
                
            
                // queue up input packets
                rr_packet pushb;
                memcpy(&pushb, p_request, sizeof(rr_packet));
                if (pushb.keycode >= 0)
                    inpPackets.push_back(pushb);
                // std::cout<<(int)pushb.keycode<<" "<<(int)pushb.keystate<<"\n";

                if (inp_sock.get(zmq::sockopt::rcvmore)) {
                    
                    bool recvmore = true;

                    while (recvmore) {
                        // std::cout<<"recv_keys\n";
                        zmq::message_t nmsg = zmq::message_t(sizeof(rr_packet));
                        inp_sock.recv(nmsg, zmq::recv_flags::none);

                        if (!inp_sock.get(zmq::sockopt::rcvmore)) {
                            recvmore = false;
                            continue;
                        }

                        rr_packet pushb;
                        memcpy(&pushb, p_request, sizeof(rr_packet));
                        if (pushb.keycode >= 0 && pushb.packet_type == P_CLIENT_INPUT)
                            inpPackets.push_back(pushb);
                    }
                }
                
                zmq::message_t rep = zmq::message_t(sizeof(rr_packet));
                rr_packet* p = (rr_packet*)rep.data();
                p->packet_type = 0;
                inp_sock.send(rep, zmq::send_flags::none);
            }
        }
    }

    void PublishUpdates(EntityManager* em) {
        std::vector<Entity*>& evref = em->getEntityVectorRef();
        // std::cout<<evref.size()<<"\n";
        for (int i = 0; i < evref.size(); i++) {
            
            if (evref[i]->entity_id == entity_id)
                continue;

            zmq::message_t em_packet = zmq::message_t(sizeof(ps_packet));
            ps_packet* data = (ps_packet*)em_packet.data();

            *data = evref[i]->Packetize(P_ENTITY_UPDATE_FETCH);
            // std::cout<<(int)evref[i]->entity_id<<"\n";
            data->entity_id = evref[i]->entity_id;
            sock.send(em_packet, zmq::send_flags::sndmore);
            
        }
        
        zmq::message_t end_packet = zmq::message_t(sizeof(ps_packet));
        ps_packet* data = (ps_packet*)end_packet.data();
        data->packet_type = P_STREAM_DONE;
        sock.send(end_packet, zmq::send_flags::none);
    
    }
    
    
    // template <typename PlayerType>
    virtual void Update(float dt, InputManager* im, EntityManager* em) override {
        for (int i  = 0; i < numConnectedPlayers; i++) {
            playerVector[i]->getVirtInputManager()->PreservePrevState();
        }

        RecvInputOrConnectionPackets(em);
        PublishUpdates(em);

        for (int p = 0; p < (int)inpPackets.size(); p++) {
        
            rr_packet top = inpPackets[p];
            for (int i = 0; i < numConnectedPlayers; i++) {
                // std::cout<<(int)inpPackets.size()<<" "<<(int)i<<" "<<(int)numConnectedPlayers<<" "<<(int)playerVector[i]->client_id<<" "<<(int)top.client_id<<"\n"; 

                if (playerVector[i]->client_id == top.client_id) {
                    // std::cout<<"In here "<<(int)numConnectedPlayers<<"\n"; 
                    if (top.keystate == K_KEYDOWN) {
                        playerVector[i]->getVirtInputManager()->SetKeyUnsafe((SDL_Scancode)top.keycode, true);
                        
                    }
                    else if (top.keystate == K_KEYUP) {
                        playerVector[i]->getVirtInputManager()->SetKeyUnsafe((SDL_Scancode)top.keycode, false);
                    }
                }
            }
        }

        inpPackets.clear();
    }

};
