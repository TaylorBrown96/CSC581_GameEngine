#include <GameEngine.h>
#include <engine_server/Server.h>
#include <iostream>
#include <SDL3/SDL_main.h>

#include "mapdef.h"


class Player : public PlayerEntity {

    int p = 0;
    
    public:

    Player(float x, float y, float w, float h) : PlayerEntity(x, y, w, h) {};
    virtual void ServerUpdate(float dt, InputManager* im, EntityManager* em) override {
        
        if (im->IsKeyPressed(SDL_SCANCODE_W))
            std::cout<<"Player "<<(int)client_id<<" pressed W "<<(int)p++<<"\n";
    } 

};

int server_main(GameEngine* eng, int map_type) {
    if (!eng->initialized)
        return -1;
    
    eng->GetInput()->disable();
    eng->GetRenderSystem()->disable();

    Server<Player>* serv = new Server<Player>("tcp://*", "5555", "5556", P_MAP_TYPE_A);
    serv->entity_id = 1;
    serv->SpawnPrototypeOnJoin = new Player(0, 0, 128, 128);
    serv->Initialize();

    loadMap(eng, map_type);
    eng->GetEntityManager()->AddEntity(serv);
    
    eng->Run();

    std::cout<<"Shutting down.\n";

    delete serv;
    eng->Shutdown();

}


int main(int argc, char** argv) {
    GameEngine eng;
    eng.Initialize("Server", 500, 500);
    server_main(&eng, P_MAP_TYPE_A);

}