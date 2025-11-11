// client_main.cpp - Example GameClient usage
#include "Networking/GameClient.h"
#include <iostream>
#include <string>
#include "main.h"

struct ClientDummyReplayState {
  int dummy;
};

int main() {
    std::cout << "Starting GameClient..." << std::endl;
    
    GameClient client;
    
    // Initialize the client
    if (!client.Initialize("Game Client", 1000, 1000, 1.0f)) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }

    // Register entity factory functions
    client.RegisterEntity("TestEntity", [&client]() -> Entity* {
        return new TestEntity(100, 100, client.GetRootTimeline(), client.GetRenderer());
    });
    client.RegisterEntity("Platform", [&client]() -> Entity* {
        return new Platform(0, 0, 0, 0, false, client.GetRootTimeline(), client.GetRenderer());
    });
    
    // Connect to the server (assuming server is running on localhost)
    std::string serverAddress = "localhost";
    int publisherPort = 5555;
    int pullPort      = 5556;
    client.GetInput()->AddAction("MOVE_LEFT",  SDL_SCANCODE_A);
    client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
    client.GetInput()->AddAction("JUMP",       SDL_SCANCODE_SPACE);
    client.GetInput()->AddAction("REPLAY",     SDL_SCANCODE_R);

    std::cout << "Connecting to server at " << serverAddress << ":" << publisherPort
              << "/" << pullPort << std::endl;
    
    if (!client.ConnectToServer(serverAddress, publisherPort, pullPort)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    // ----------------------------
    // Dummy client-side replay track (for visuals only)
    // ----------------------------
    if (auto* replay = client.GetReplaySystem()) {
        replay->RegisterTrack<ClientDummyReplayState>(
            "client_dummy",
            []() -> ClientDummyReplayState {
                ClientDummyReplayState s{};
                s.dummy = 42;   // any value; we don't use it
                return s;
            },
            [](const ClientDummyReplayState&) {
                // Do nothing on apply; we only care about replay mode
            }
        );
    }

    std::cout << "Client connected successfully!" << std::endl;
    std::cout << "Client ID: " << client.GetClientId() << std::endl;
    std::cout << "Use A/D keys to move, SPACE to jump, ESC to exit" << std::endl;
    std::cout << "Press R to trigger a 5-second replay on ALL clients" << std::endl;
    
    // Run the client (this will handle input, networking, and rendering)
    client.Run();
    
    client.Shutdown();
    std::cout << "Client shutdown complete" << std::endl;
    
    return 0;
}