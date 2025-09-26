
#include <zmq.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <Entity.h>
#include <vec2.h>
#include <vector>
#include <packetdef.h>
#include <GameEngine.h>


#define THREADS 1

class Server : public Entity {
    int loaded_map;
    zmq::context_t* context;
    zmq::socket_t sock;

    zmq::socket_t inp_sock;

    // zmq::socket_t* sock;
    std::string endpoint;
    std::string port;
    std::string inp_port;
    // InputManager* im;
    
    int connected_clients;
    int proc_clients;
    public:

    // void SetManagersToSync(InputManager* p_im, EntityManager* p_em) {
    //     im = p_im;
    //     em = p_em;
    // }

    Server(std::string p_endpoint, std::string p_port, std::string p_inp_port, int map_type) : endpoint(p_endpoint), port(p_port), inp_port(p_inp_port), loaded_map(map_type) {
        SetOverseer();
        connected_clients = 0;
        proc_clients = 0;
    }
    
    
    void Initialize() {
        context = new zmq::context_t(THREADS);
        sock = zmq::socket_t(*context, zmq::socket_type::pub);
        sock.bind(endpoint+":"+port);

        inp_sock = zmq::socket_t(*context, zmq::socket_type::rep);
        inp_sock.bind(endpoint+":"+inp_port);
    }
    

    void RecvInputOrConnectionPackets() {
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // std::cout<<"Started input connection:\n";
        // while (true) {
            zmq::message_t req(sizeof(rr_packet));
            
            rr_packet* p = (rr_packet*)req.data();
            p->packet_type = -2;
            inp_sock.recv(req, zmq::recv_flags::dontwait);
            if (p->packet_type != 2) {
                rr_packet* p_request = (rr_packet*)req.data();

                if (p_request->packet_type == P_CLIENT_HELLO) {
                    // std::cout<<"Client connected\n";
                    zmq::message_t rep(sizeof(rr_packet));
                    rr_packet* rp = (rr_packet*)rep.data();
                    rp->packet_type = loaded_map;
                    inp_sock.send(rep, zmq::send_flags::dontwait);
                    // send map type so that they could load all the same objects loaded here

                }

                if (p_request->packet_type == P_CLIENT_INPUT) {

                    // apply input engine side
                }
            }
    }

    void PublishUpdates(EntityManager* em) {
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // std::cout<<"Started Update connection:\n";
        // while (true) {
            std::vector<Entity*>& evref = em->getEntityVectorRef();
            for (int i = 0; i < evref.size(); i++) {
                zmq::message_t em_packet = zmq::message_t(sizeof(ps_packet));
                ps_packet* data = (ps_packet*)em_packet.data();

                *data = evref[i]->Packetize(P_ENTITY_UPDATE_FETCH);
                data->entity_id = evref[i]->entity_id;
                sock.send(em_packet, zmq::send_flags::sndmore);
                // std::cout<<"SendingUpdate "<<(int)i<<"\n";
            }
            
            zmq::message_t end_packet = zmq::message_t(sizeof(ps_packet));
            ps_packet* data = (ps_packet*)end_packet.data();
            data->packet_type = P_ENTITY_UPDATE_FETCH;
            sock.send(end_packet, zmq::send_flags::none);
            // std::cout<<"EndUpdate\n";
        

    }

    virtual void Update(float dt, InputManager* im, EntityManager* em) override {
        RecvInputOrConnectionPackets();
        PublishUpdates(em);
    }

};

class Platform : public Entity {
 public:
  Platform(float x, float y, float w = 200, float h = 20, bool moving = false)
      : Entity(x, y, w, h) {
    isStatic = true;
    hasPhysics = false;
    affectedByGravity = false;
    isOneWay = true;  // <- key line
    velocity.x = moving ? -100.0f : 0.0f;
    velocity.y = 0.0f;

  }

  void Update(float dt, InputManager *input,
              EntityManager *entitySpawner) override {
    (void)input;
    (void)entitySpawner;
    // Horizontal-only motion for the moving platform
    position = add(position, mul(dt, velocity));
    if (position.x < 0) {
      position.x = 0;
      velocity.x *= -1;
      
    } else if (position.x + dimensions.x > 1920) {
      position.x = 1920 - dimensions.x;
      velocity.x *= -1;
    }
  }
};

void loadMap(GameEngine* ge, int map_type) {
    switch(map_type) {
        case P_MAP_TYPE_A:
            Platform *platform1 = new Platform(300, 800, 300, 75, true);
            platform1->entity_id = 101;
            platform1->hasPhysics = false;         // no integration
            platform1->affectedByGravity = false;  // no gravity
            platform1->isStatic = true;

            Platform *platform2 = new Platform(800, 650, 300, 75, true);
            platform1->entity_id = 102;
            platform2->hasPhysics = true;  // we want horizontal motion we code ourselves
            platform2->affectedByGravity = false;  // but no falling
            platform2->isStatic = true;  // treat as static for collisions if you have special handling

            // Add entities to the engine
            ge->GetEntityManager()->AddEntity(platform1);
            ge->GetEntityManager()->AddEntity(platform2);

            
            SDL_Texture *platformTexture =
                LoadTexture(ge->GetRenderer(),
                            "media/cartooncrypteque_platform_basicground_idle.bmp");
            if (platformTexture) {
                platform1->SetTexture(platformTexture);
                platform2->SetTexture(platformTexture);
            }
            break;
        
    }
}

int server_main(GameEngine* eng, int map_type) {
    if (!eng->initialized)
        return -1;
    
    eng->GetInput()->disable();
    eng->GetRenderSystem()->disable();

    Server* serv = new Server("tcp://*", "5555", "5556", P_MAP_TYPE_A);
    serv->Initialize();
    loadMap(eng, map_type);
    eng->GetEntityManager()->AddEntity(serv);
    // serv.SetManagersToSync(eng->GetInput(), eng->GetEntityManager());
    
    // std::thread RRThread(&Server::RecvInputOrConnectionPackets, &serv);
    // std::thread PubThread(&Server::PublishUpdates, &serv);

    eng->Run();

}


int main() {
    GameEngine eng;
    eng.Initialize("Server", 500, 500);
    server_main(&eng, P_MAP_TYPE_A);

}