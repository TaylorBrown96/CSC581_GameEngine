#include <GameEngine.h>
#include <engine_server/Server.h>

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


int main(int argc, char** argv) {
    GameEngine eng;
    eng.Initialize("Server", 500, 500);
    server_main(&eng, P_MAP_TYPE_A);

}