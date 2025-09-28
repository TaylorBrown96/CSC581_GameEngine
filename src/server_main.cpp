// server_main.cpp - Example GameServer usage
#include "GameServer.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "main.h"

/*
 * Example of how developers can create their own custom player entity class:
 * 
 * class MyCustomPlayerEntity : public Entity {
 * public:
 *     MyCustomPlayerEntity(const std::string& clientId, SDL_Renderer* renderer) 
 *         : Entity(100, 100, 64, 64) { // Default spawn position
 *         entityType = "CustomPlayer_" + clientId;
 *         hasPhysics = true;
 *         affectedByGravity = true;
 *         
 *         // Load custom texture using the renderer
 *         if (renderer) {
 *             SDL_Texture* playerTexture = LoadTexture(renderer, "media/player_texture.bmp");
 *             if (playerTexture) {
 *                 Texture tex = {
 *                     .sheet = playerTexture,
 *                     .num_frames_x = 4,
 *                     .num_frames_y = 1,
 *                     .frame_width = 64,
 *                     .frame_height = 64,
 *                     .loop = true
 *                 };
 *                 SetTexture(0, &tex);
 *             }
 *         }
 *     }
 *     
 *     void Update(float deltaTime, InputManager* input, EntityManager* entityMgr) override {
 *         // Add custom player logic here
 *         Entity::Update(deltaTime, input, entityMgr);
 *     }
 * };
 * 
 * Then in the factory function, you would use:
 * return new MyCustomPlayerEntity(clientId, renderer);
 */

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
    server.SetPlayerEntityFactory([](SDL_Renderer* renderer) -> Entity* {
        return new TestEntity(100, 100, renderer);
    });

    Platform *platform1 = new Platform(300, 800, 300, 75, false, server.GetRenderer());
    platform1->hasPhysics = false;         // no integration
    platform1->affectedByGravity = false;  // no gravity
    platform1->isStatic = true;
    server.GetEntityManager()->AddEntity(platform1);

    Platform *platform2 = new Platform(800, 650, 300, 75, true, server.GetRenderer());
    platform2->hasPhysics = true;  // we want horizontal motion we code ourselves
    platform2->affectedByGravity = false;  // but no gravity
    platform2->isStatic =
        true;  // treat as static for collisions if you have special handling
    server.GetEntityManager()->AddEntity(platform2);
    
    // Start the server with publisher on port 5555 and pull socket on port 5556
    if (!server.StartServer(5555, 5556)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server started successfully!" << std::endl;
    std::cout << "Publisher port: 5555" << std::endl;
    std::cout << "Pull socket port: 5556" << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    auto entityManager = server.GetEntityManager();
    
    // Run the server (this will run the game loop with networking)
    // The server's Run() method will check shouldStop internally
    server.Run();
    
    // Shutdown the server
    server.Shutdown();
    std::cout << "Server shutdown complete" << std::endl;
    
    return 0;
}