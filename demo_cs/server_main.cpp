// server_main.cpp - Example GameServer usage
#include "Networking/GameServer.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include "main.h"

// Simple snapshot types for server-side world replay
struct WorldEntityState {
  float x;
  float y;
  float vx;
  float vy;
  int   currentFrame;
  int   currentTextureState;
};

using WorldSnapshot = std::vector<WorldEntityState>;

// Global server pointer for signal handling
GameServer* g_server = nullptr;

// Signal handler function
void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nReceived SIGINT (Ctrl+C). Shutting down server gracefully..." << std::endl;
        if (g_server) {
            g_server->RequestStop();
        }
    }
}

int main() {
    std::cout << "Starting GameServer..." << std::endl;
    
    // Register signal handler for Ctrl+C
    signal(SIGINT, signalHandler);
    
    GameServer server;
    g_server = &server;  // Set global pointer for signal handler
    
    // Initialize the server with headless game engine
    if (!server.Initialize("Game Server", 320, 240)) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    // Set up the player entity factory - developers can customize this
    // This allows the engine to remain game-agnostic while letting developers
    // specify their own player entity class
    server.SetPlayerEntityFactory([&server](SDL_Renderer* renderer) -> Entity* {
        return new TestEntity(100, 100, server.GetRootTimeline(), renderer);
    });

    // platform1 has collision enabled but no physics (static platform)
    Platform *platform1 = new Platform(300, 800, 300, 75, false, server.GetRootTimeline(), server.GetRenderer());
    server.GetEntityManager()->AddEntity(platform1);
    // platform2 has collision and physics enabled (moving platform)
    Platform *platform2 = new Platform(800, 650, 500, 75, false, server.GetRootTimeline(), server.GetRenderer());
    server.GetEntityManager()->AddEntity(platform2);
    
    // Start the server with publisher on port 5555 and pull socket on port 5556
    if (!server.StartServer(5555, 5556)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    Platform *platform3 = new Platform(1300, 500, 500, 75, false, server.GetRootTimeline(), server.GetRenderer());
    server.GetEntityManager()->AddEntity(platform3);

    Platform *platform4 = new Platform(1800, 500, 500, 75, false, server.GetRootTimeline(), server.GetRenderer());
    server.GetEntityManager()->AddEntity(platform4);

    ScrollBoundary *scrollBoundary = new ScrollBoundary(950, 500, 1000, 200, server.GetRootTimeline(), server.GetRenderer());
    server.GetEntityManager()->AddEntity(scrollBoundary);

    // ----------------------------
    // Server-side world replay track
    // ----------------------------
    if (auto* replay = server.GetReplaySystem()) {
        replay->RegisterTrack<WorldSnapshot>(
            "world",
            [&server]() -> WorldSnapshot {
                WorldSnapshot snap;
                auto* em = server.GetEntityManager();
                auto& entities = em->getEntityVectorRef();
                snap.reserve(entities.size());
                for (Entity* e : entities) {
                    if (!e) continue;
                    WorldEntityState s{};
                    SDL_FRect bounds = e->GetBounds();
                    s.x  = bounds.x;
                    s.y  = bounds.y;
                    s.vx = e->GetVelocityX();
                    s.vy = e->GetVelocityY();

                    // Animation state per entity
                    s.currentFrame        = e->rendering.currentFrame;
                    s.currentTextureState = e->rendering.currentTextureState;

                    snap.push_back(s);
                }
                return snap;
            },
            // Push that snapshot back onto entities in the same order
            [&server](const WorldSnapshot& snap) {
                auto* em = server.GetEntityManager();
                auto& entities = em->getEntityVectorRef();
                const size_t n = std::min(snap.size(), entities.size());
                for (size_t i = 0; i < n; ++i) {
                    Entity* e = entities[i];
                    if (!e) continue;
                    const auto& s = snap[i];
                    e->SetPosition(s.x, s.y);
                    e->SetVelocity(s.vx, s.vy);

                    // Restore animation frame on server
                    e->rendering.currentFrame        = s.currentFrame;
                    e->rendering.currentTextureState = s.currentTextureState;
                }
            }
        );
    }

    
    std::cout << "Server started successfully!" << std::endl;
    std::cout << "Publisher port: 5555" << std::endl;
    std::cout << "Pull socket port: 5556" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Run the server (game loop + networking)
    server.Run();
    
    // Shutdown the server
    server.Shutdown();
    std::cout << "Server shutdown complete" << std::endl;
    
    return 0;
}