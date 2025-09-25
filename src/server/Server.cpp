
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <Entity.h>
#include <vec2.h>
#include <vector>
#include <packetdef.h>

#define THREADS 1

class Server {
    int loaded_map;
    zmq::context_t* context;
    zmq::socket_t sock;
    // zmq::socket_t* sock;
    std::string endpoint;
    EntityManager* entityMap;
    int connected_clients;
    int proc_clients;
    public:
    Server(std::string p_endpoint, int map_type) : endpoint(p_endpoint), loaded_map(map_type) {
        entityMap = new EntityManager();
        connected_clients = 0;
        proc_clients = 0;
    }
    
    
    void Initialize() {
        context = new zmq::context_t(THREADS);
        sock = zmq::socket_t(*context, zmq::socket_type::rep);
        sock.bind(endpoint);
    }
    
    void Run() {
        while (true) {
            zmq::message_t request;
            sock.recv(request, zmq::recv_flags::none);
            packet_def* packet = (packet_def*)request.data();
            
            if (packet->packet_type == P_CLIENT_HELLO) {
                connected_clients++;
                // send packet denoting map (rep)
                zmq::message_t rep(sizeof(packet_def));
                packet_def* packet_map = (packet_def*)rep.data();
                packet_map->packet_type = loaded_map;
                // packet_map->client_id = en->clientid;
                
                sock.send(rep, zmq::send_flags::none);
            }

            if (packet->packet_type == P_ENTITY_UPDATE_PUT) {
                
                std::vector<Entity*>& evref = entityMap->getEntityVectorRef();
                bool obj_found = false;
                int index = 0;
                
                for (int i = 0; i < evref.size(); i++) {
                    if (evref[i]->entity_id == packet->entity_id) {
                        obj_found = true;
                        index = i;
                        break;
                    }
                }

                if (obj_found) {
                    // already exists, correct and send the current positions
                   
                    evref[index]->position = packet->position;
                    evref[index]->velocity = packet->velocity;
                    evref[index]->force = packet->force;
                    evref[index]->dimensions = packet->dimensions;
                
                    // evref[index]->position = mul(0.5f, add(evref[index]->position, packet->position));
                    // evref[index]->velocity = mul(0.5f, add(evref[index]->velocity, packet->velocity));
                    // evref[index]->force = mul(0.5f, add(evref[index]->force, packet->force));
                    // evref[index]->dimensions = mul(0.5f, add(evref[index]->dimensions, packet->dimensions));

                    packet_def npdef;
                    zmq::message_t rep(sizeof(packet_def));
                    memcpy(rep.data(), &npdef, sizeof(packet_def));
                    sock.send(rep, zmq::send_flags::none);
                    
                }
                else {
                    Entity* en = new Entity(packet->position.x,
                    packet->position.y, 
                    packet->dimensions.x,
                    packet->dimensions.y);
                    en->velocity = packet->velocity;
                    en->force = packet->force;
                    en->entity_id = packet->entity_id;
                    entityMap->AddEntity(en);
                    sock.send(request, zmq::send_flags::none);
                    
                }
            }

            if (packet->packet_type == P_ENTITY_UPDATE_FETCH) {
                std::vector<Entity*>& evref = entityMap->getEntityVectorRef();
                bool obj_found = false;
                int index = 0;

                for (int i = 0; i < evref.size(); i++) {
                    if (evref[i]->entity_id == packet->entity_id) {
                        obj_found = true;
                        index = i;
                        break;
                    }
                
                }

                if (obj_found) {
                    zmq::message_t rep(sizeof(packet_def));
                    packet_def npdef;
                    npdef.entity_id = evref[index]->entity_id;
                    npdef.position = evref[index]->position;
                    npdef.velocity = evref[index]->velocity;
                    npdef.force = evref[index]->force;
                    npdef.dimensions = evref[index]->dimensions;
                    memcpy(rep.data(), &npdef, sizeof(packet_def));
                    sock.send(rep, zmq::send_flags::none);

                }
                else {
                    zmq::message_t rep(sizeof(packet_def));
                    packet_def* pd = (packet_def*)rep.data();
                    pd->packet_type = -1;
                    sock.send(rep, zmq::send_flags::none);
                }
            }
            
        }

    } 

};

int main() {
    std::cout<<"A\n";
    Server serv("tcp://*:5555", P_MAP_TYPE_A);
    std::cout<<"A\n";
    serv.Initialize();
    std::cout<<"A\n";
    serv.Run();
}