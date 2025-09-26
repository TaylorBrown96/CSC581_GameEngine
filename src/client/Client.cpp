
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

    
    Client() : Entity() {
        SetOverseer();
    }
    void initClient(std::string p_endpoint, std::string port, std::string inp_port) {
        con = zmq::context_t(1);
        sock = zmq::socket_t(con, zmq::socket_type::req);
        upd_sock = zmq::socket_t(con, zmq::socket_type::sub);
        update_endpoint = p_endpoint + ":" + port;
        input_endpoint = p_endpoint + ":" + inp_port;
    
    }

    void ConnectInit(int* map_type) {
        sock.connect(input_endpoint);

        rr_packet rp;
        rp.packet_type = P_CLIENT_HELLO;
        
        sendRRPacket(&rp);

        rr_packet rep;
        RecvRRPacket(&rep);
        *map_type = rep.packet_type;

        upd_sock.connect(update_endpoint);
        char topic = P_ENTITY_UPDATE_FETCH;
        upd_sock.setsockopt(ZMQ_SUBSCRIBE, &topic, sizeof(char));
    }

    void sendRRPacket(rr_packet* pack) {
        zmq::message_t msg = zmq::message_t(sizeof(rr_packet));
        memcpy(msg.data(), pack, sizeof(rr_packet));
        sock.send(msg, zmq::send_flags::none);
    }

    void RecvRRPacket(rr_packet* pack) {
        zmq::message_t msg = zmq::message_t(sizeof(rr_packet));
        sock.recv(msg, zmq::recv_flags::none);
        memcpy(pack, msg.data(), sizeof(rr_packet));
    }

    void QueueUpdatePackets() {
        
        while (true) {
            
            zmq::message_t nmsg = zmq::message_t(sizeof(ps_packet));
            upd_sock.recv(nmsg, zmq::recv_flags::none);
            
            if (!upd_sock.get(zmq::sockopt::rcvmore))
                break;

            ps_packet ps;

            memcpy(&ps, nmsg.data(), sizeof(ps_packet));
            
            if (ps.packet_type != P_STREAM_DONE)
                packQueue.push_back(ps);    

            
        }
    }

    void ApplyQueueUpdatePacketsToState(EntityManager* em) {
        std::vector<Entity*>& evref = em->getEntityVectorRef();

        while (packQueue.size() > 0) {
            // std::cout<<"Applying\n";
            ps_packet pack_top = packQueue.back();
            bool found = false;
            for (int i = 0; i < evref.size() - 1 && !found; i++) {
                if (evref[i]->entity_id == pack_top.entity_id) {
                    evref[i]->Unpacketize(&pack_top);
                    found = true;
                }
            }

            packQueue.pop_back();
        }

    }

    virtual void Update(float dt, InputManager* in, EntityManager* em) override {
        QueueUpdatePackets();
        ApplyQueueUpdatePacketsToState(em);
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

int client_main(GameEngine* eng) {
    if (!eng->initialized)
        return -1;
    
    Client* cl = new Client();
    cl->initClient("tcp://localhost", "5555", "5556");
    int map;
    cl->ConnectInit(&map);
    
    loadMap(eng, map);

    eng->GetEntityManager()->AddEntity(cl);
    eng->Run();
    return 1;
}

int main() {
    GameEngine eng;
    eng.Initialize("Client", 1800, 1000);
    eng.GetInput()->enable();
    eng.GetRenderSystem()->enable();
    int k = client_main(&eng);
}