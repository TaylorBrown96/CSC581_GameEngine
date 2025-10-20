// client_main.cpp - Example GameClient usage
#include "Networking/GameClient.h"
#include <iostream>
#include <string>
#include "csmain.h"


int main() {
    std::cout<<"This is changed!!!!!!!\n";
    std::cout << "Starting GameClient..." << std::endl;
    
    GameClient client;
    
    // Register entity factory functions

    // Initialize the client
    if (!client.Initialize("Game Client", 1000, 1000, 1.0f)) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }
    
    client.byteSerialize = false;

    client.RegisterEntity("TestEntityBare", [&client]() -> TestEntityBare* {
        return new TestEntityBare(100, 100, client.GetRootTimeline(), client.GetRenderer(), "TestEntityBare"); 
    });

    client.RegisterEntity("DynamicEntityBare", [&client]() -> DynamicEntityBare* {
        return new DynamicEntityBare(1, 1, client.GetRootTimeline(), client.GetRenderer(), "DynamicEntityBare"); 
    });
    
    // Connect to the server (assuming server is running on localhost)
    std::string serverAddress = "localhost";
    int publisherPort = 5555;
    int pullPort = 5556;
    client.GetInput()->AddAction("MOVE_LEFT", SDL_SCANCODE_A);
    client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
    client.GetInput()->AddAction("MOVE_UP", SDL_SCANCODE_W);
    client.GetInput()->AddAction("MOVE_DOWN", SDL_SCANCODE_S);
    // client.GetInput()->AddAction("JUMP", SDL_SCANCODE_SPACE);
    std::cout << "Connecting to server at " << serverAddress << ":" << publisherPort << "/" << pullPort << std::endl;
    
    if (!client.ConnectToServer(serverAddress, publisherPort, pullPort)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    std::cout << "Client connected successfully!" << std::endl;
    std::cout << "Client ID: " << client.GetClientId() << std::endl;
    std::cout << "Use WASD keys to move, SPACE to jump, ESC to exit" << std::endl;
    std::cout << "The client will send input to server and receive game state updates" << std::endl;
    
    // Run the client (this will handle input, networking, and rendering)
    client.Run();
    
    client.Shutdown();
    std::cout << "Client shutdown complete" << std::endl;
    
    return 0;
}