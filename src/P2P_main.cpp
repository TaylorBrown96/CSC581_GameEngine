// 
#include "Core/GameEngine.h"
#include "Networking/P2PHandler.h"
#include "main.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <cmath>


void p_makeMap(GameEngine* eng) {

    Platform* platform1 = new Platform(300, 800, 300, 75, false, eng->GetRootTimeline(), eng->GetRenderer());
    // platform1 has collision enabled but no physics (static platform)
    eng->GetEntityManager()->AddEntity(platform1);

    Platform* platform2 = new Platform(800, 650, 300, 75, true, eng->GetRootTimeline(), eng->GetRenderer());
    // platform2 has collision and physics enabled (moving platform)
    eng->GetEntityManager()->AddEntity(platform2);

    SDL_Texture* platformTexture = LoadTexture(eng->GetRenderer(),
        "media/cartooncrypteque_platform_basicground_idle.bmp");
    if (platformTexture) {
        Texture tex = { platformTexture, 1, 1, 200, 20, true };
        platform1->SetTexture(0, &tex);
        platform2->SetTexture(0, &tex);
    }
    std::cout << "[P2PMain] World spawned (2 platforms)\n";
}

Entity* p_makePlayer(GameEngine* eng, int peerId, int myId, bool auth) {

    Timeline* tl = (peerId == myId && auth)
                 ? new Timeline(4.0, eng->GetRootTimeline())
                 : eng->GetRootTimeline();

    TestEntity* e = new TestEntity(100, 100, tl, eng->GetRenderer());
    // TestEntity already enables physics in its constructor

    SDL_Texture* entityTexture = LoadTexture(eng->GetRenderer(),
        "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
    if (entityTexture) {
        Texture tex = { entityTexture, 8, 0, 512, 512, true };
        e->SetTexture(0, &tex);
    }

    eng->GetEntityManager()->AddEntity(e);
    return e;
}

int main() {
    P2PHandler handler;
    handler.InitializeEngine("P2P", 1800, 1000, 1.0);

    handler.spawnMap = p_makeMap;
    handler.spawnPlayer = p_makePlayer;
    // Factories used by clients to reconstruct entities from STATE
    auto makePlayer = [](GameEngine* ge) -> Entity* {
        auto* e = new TestEntity(100, 100, ge->GetRootTimeline(), ge->GetRenderer());
        // TestEntity already enables physics in its constructor
        SDL_Texture* t = LoadTexture(ge->GetRenderer(),
            "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
        if (t) { Texture tex = { t, 8, 0, 512, 512, true }; e->SetTexture(0, &tex); }
        return e;
    };
    handler.factory_["TestEntity"] = makePlayer;
    handler.factory_["Player"]     = makePlayer;
    handler.factory_["Skelly"]     = makePlayer;
    handler.factory_["Skeleton"]   = makePlayer;

    handler.factory_["Platform"] = [](GameEngine* eng) -> Entity* {
        Platform* p = new Platform(0,0,200,20,false, eng->GetRootTimeline(), eng->GetRenderer());
        // Platform has collision enabled but no physics (static platform)
        SDL_Texture* t = LoadTexture(eng->GetRenderer(), "media/cartooncrypteque_platform_basicground_idle.bmp");
        if (t) { Texture tex = { t, 1, 1, 200, 20, true }; p->SetTexture(0, &tex); }
        return p;
    };

    handler.GetEngine()->GetInput()->AddAction("MOVE_LEFT",  SDL_SCANCODE_A);
    handler.GetEngine()->GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
    handler.GetEngine()->GetInput()->AddAction("JUMP",       SDL_SCANCODE_SPACE);

    handler.Boot();

    handler.Run();

}
