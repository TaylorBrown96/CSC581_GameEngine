#include <GameEngine.h>
#include <engine_server/Server.h>
#include <iostream>
#include <SDL3/SDL_main.h>

#include "mapdef.h"

int server_main(GameEngine* eng, int map_type) {
    if (!eng->initialized)
        return -1;
    
    eng->GetInput()->disable();
    eng->GetRenderSystem()->disable();

    Server* serv = new Server("tcp://*", "5555", "5556", P_MAP_TYPE_A);
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