
#pragma once
#include "GameEngine.h"
#include "main.h"
#include "./p2p/P2PNode.h"
#include <SDL3/SDL.h>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <set>

class P2PSkellyGame {
public:
    P2PSkellyGame() : engine_(false) {}
    ~P2PSkellyGame() { stop(); }

    bool Boot(const char* title = "P2P Skelly (Authority)",
              int w = 1800, int h = 1000, float timeScale = 1.0f,
              const std::string& trackerRep = "tcp://127.0.0.1:6000",
              const std::string& trackerPub = "tcp://127.0.0.1:6001");

    void RunLoop();
    void stop();

    void SetAuthorityAnimMultiplier(float mul) { authorityAnimMul_ = mul; }

private:
    // Game Engine
    GameEngine engine_;
    
    // Networking
    P2PNode node_;
    bool authority_ = false;
    int myId_ = -1;
    int otherId_ = -1;

    bool worldSpawned_ = false;

    // Authority-side
    std::mutex peersMtx_;
    std::vector<int> connectedPeers_;
    std::unordered_map<int, Entity*> peerToEntity_;
    std::unordered_map<int, std::vector<std::string>> inputs_;

    // Client-side
    std::unordered_map<std::string, std::function<Entity*()>> factory_;
    std::unordered_map<int, Entity*> idToEntity_;
    struct Smooth { float tx=0, ty=0, tvx=0, tvy=0; uint64_t stamp=0; };
    std::unordered_map<int, Smooth> smooth_;

    // RX handoff
    std::mutex stateMtx_;
    std::string pendingState_;
    bool hasPendingState_ = false;

    // Config
    float authorityAnimMul_ = 4.0f;
    float smoothingHz_ = 45.0f;

    // Helpers
    void onPeerMessage(const std::string& s);

    // Spawns (authority)
    Entity* spawnPlayerFor(int peerId, float x, float y);
    void despawnPlayerFor(int peerId);
    void spawnStaticWorld();

    // Client sync
    void processState(const std::string& payload);
    Entity* ensureEntityFor(const std::string& type, int remoteId);

    // Timing
    uint64_t lastBroadcastMs_ = 0;
    uint64_t lastInputSendMs_ = 0;
};
