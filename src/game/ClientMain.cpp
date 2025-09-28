#include <GameEngine.h>
#include <iostream>
#include <engine_client/Client.h>

#include "mapdef.h"

int client_main(GameEngine* eng) {

    if (!eng->initialized)
        return -1;
    eng->GetRenderSystem()->SetScalingMode(ScalingMode::PROPORTIONAL);
    // create client
    Client* cl = new Client();
    // init client
    cl->initClient("tcp://localhost", "5555", "5556");
    cl->entity_id = 2;
    int map;
    // connect client to server and get map
    cl->SetKeyboardSize(eng->GetInput()->Size());
    cl->ConnectInit(&map);
    
    
    // load map
    loadMap(eng, map);

    // add client to entity vector
    eng->GetEntityManager()->AddEntity(cl);

    eng->Run();
    std::cout<<"Shutting down.\n";

    // delete the client
    cl->Disconnect();
    delete cl;
    eng->Shutdown();
    return 1;
}

int main(int argc, char** argv) {
    GameEngine eng;
    eng.Initialize("Client", 1800, 1000);
    eng.GetInput()->enable();
    eng.GetRenderSystem()->enable();
    int k = client_main(&eng);
}