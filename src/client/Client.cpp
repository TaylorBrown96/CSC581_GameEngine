
#include <zmq.hpp>

#include <packetdef.h>
#include <GameEngine.h>
#include <iostream>

class Client {
    zmq::context_t con;
    zmq::socket_t sock;
    std::string endpoint;
    public:
    Client(std::string p_endpoint) {
        con = zmq::context_t(1);
        sock = zmq::socket_t(con, zmq::socket_type::req);
        endpoint = p_endpoint;

    }

    void connect(int* map_type) {
        sock.connect(endpoint);
        zmq::message_t reqm(sizeof(packet_def));
        packet_def* pd = (packet_def*)reqm.data();
        pd->packet_type = P_CLIENT_HELLO;
        sock.send(reqm, zmq::send_flags::none);

        zmq::message_t repm(sizeof(packet_def));
        sock.recv(repm, zmq::recv_flags::none);
        pd = (packet_def*)repm.data();
        *map_type = pd->packet_type;
    }

    void sendPacket(packet_def* pack) {
        zmq::message_t reqm(sizeof(packet_def));
        packet_def* pd = (packet_def*)reqm.data();
        memcpy(pd, pack, sizeof(packet_def));
        sock.send(reqm, zmq::send_flags::none);
    }

    void recvPacket(packet_def* pack) {
        zmq::message_t repm(sizeof(packet_def));
        sock.recv(repm, zmq::recv_flags::none);
        memcpy(pack, (packet_def*)repm.data(), sizeof(packet_def));
    }

};


Client client("tcp://localhost:5555");

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

    packet_def pdata, pdata_recv;
    pdata = this->Packetize(P_ENTITY_UPDATE_PUT);
    client.sendPacket(&pdata);
    client.recvPacket(&pdata_recv); // ack

    packet_def pdata_fetch = this->Packetize(P_ENTITY_UPDATE_FETCH);
    client.sendPacket(&pdata_fetch); // req for updated pos
    client.recvPacket(&pdata_recv); // res
    Unpacketize(&pdata_recv);
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

int main() {
   
    
    
    int map_index;

    client.connect(&map_index);
    

    GameEngine engine;
    if (!engine.Initialize("Game Engine", 1800, 1000)) {
        return 1;
    }

    loadMap(&engine, map_index);

    engine.Run();
}