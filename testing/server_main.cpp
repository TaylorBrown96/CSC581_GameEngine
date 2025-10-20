// server_main.cpp - Example GameServer usage
#include "Networking/GameServer.h"
#include <iostream>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include "csmain.h"
#include <ctime>
#include <SDL3/SDL_main.h>


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

int main(int argc, char** argv) {
    int nEntities = 8;
    if (argc > 1)
        nEntities = std::stoi(argv[1]);

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
    server.byteSerialize = false;
    // Set up the player entity factory - developers can customize this
    // This allows the engine to remain game-agnostic while letting developers
    // specify their own player entity class
    // server.RegisterTypeId("TestEntityBare");

    server.SetPlayerEntityFactory([&server](SDL_Renderer* renderer) -> Entity* {
        return new TestEntityBare(100, 100, server.GetRootTimeline(), renderer, "TestEntityBare");
    });
    srand(time(0));
    for (int n = 0; n < nEntities; n++) {
        float f1 = std::rand() / (float)RAND_MAX;
        float f2 = std::rand() / (float)RAND_MAX;
        float x = 1.0 + (f1) * 800.0;
        float y = 1.0 + (f2) * 800.0;
        std::cout<<f1<<" "<<f2<<"\n";
        DynamicEntityBare* dEntity = new DynamicEntityBare(x, y, server.GetRootTimeline(), server.GetRenderer(), "DynamicEntityBare");
        server.GetEntityManager()->AddEntity(dEntity);
    }
    // Platform *platform1 = new Platform(300, 800, 300, 75, false, server.GetRootTimeline(), server.GetRenderer());
    // // platform1 has collision enabled but no physics (static platform)
    // server.GetEntityManager()->AddEntity(platform1);

    // Platform *platform2 = new Platform(800, 650, 300, 75, true, server.GetRootTimeline(), server.GetRenderer());
    // // platform2 has collision and physics enabled (moving platform)
    // server.GetEntityManager()->AddEntity(platform2);
    
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