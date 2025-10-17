// server_main.cpp - Example GameServer usage
#include "Networking/GameServer.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "main.h"


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

    Platform *platform1 = new Platform(300, 800, 300, 75, false, server.GetRootTimeline(), server.GetRenderer());
    // platform1 has collision enabled but no physics (static platform)
    server.GetEntityManager()->AddEntity(platform1);

    Platform *platform2 = new Platform(800, 650, 500, 75, false, server.GetRootTimeline(), server.GetRenderer());
    // platform2 has collision and physics enabled (moving platform)
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