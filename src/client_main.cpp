// client_main.cpp - Example GameClient usage
#include "GameClient.h"
#include <iostream>
#include <string>
#include "main.h"

GameClient client;

int main() {
    std::cout << "Starting GameClient..." << std::endl;
    
    
    // Register entity factory functions

    // Initialize the client
    if (!client.Initialize("Game Client", 1000, 1000)) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }

    client.RegisterEntity("TestEntity", []() -> Entity* { return new TestEntity(100, 100, client.GetRenderer()); });
    client.RegisterEntity("Platform", []() -> Entity* { return new Platform(0, 0, 0, 0, false, client.GetRenderer()); });
    
    // Connect to the server (assuming server is running on localhost)
    std::string serverAddress = "localhost";
    int publisherPort = 5555;
    int pullPort = 5556;
    client.GetInput()->AddAction("MOVE_LEFT", SDL_SCANCODE_A);
    client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
    client.GetInput()->AddAction("JUMP", SDL_SCANCODE_SPACE);
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