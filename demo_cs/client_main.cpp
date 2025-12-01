// client_main.cpp - Example GameClient usage
#include "Networking/GameClient.h"
#include <iostream>
#include <string>
#include "main.h"

int main() {
    std::cout << "Starting GameClient..." << std::endl;
    
    GameClient client;
    
    // Register entity factory functions

    // Initialize the client
    if (!client.Initialize("Game Client", 1000, 1000, 1.0f)) {
        std::cerr << "Failed to initialize client" << std::endl;
        return 1;
    }

    client.RegisterEntity("TestEntity", [&client]() -> Entity* { return new TestEntity(100, 100, client.GetRootTimeline(), client.GetRenderer()); });
    client.RegisterEntity("Platform", [&client]() -> Entity* { return new Platform(0, 0, 0, 0, false, client.GetRootTimeline(), client.GetRenderer()); });
    
    // Connect to the server (assuming server is running on localhost)
    std::string serverAddress = "localhost";
    int publisherPort = 5555;
    int pullPort = 5556;
    // Register single-key action

    client.GetInput()->AddAction("MOVE_LEFT", SDL_SCANCODE_A);
    client.GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
    client.GetInput()->AddAction("JUMP", SDL_SCANCODE_SPACE);
    
    // Register chord actions (multiple keys pressed simultaneously)
    // MEGA_JUMP: Shift + Space (both must be pressed together)
    // DASH_LEFT: Shift + A (both must be pressed together)

    client.GetInput()->AddChordAction("DASH_LEFT", {SDL_SCANCODE_LSHIFT, SDL_SCANCODE_A});
    // DASH_RIGHT: Shift + D (both must be pressed together)
    client.GetInput()->AddChordAction("DASH_RIGHT", {SDL_SCANCODE_LSHIFT, SDL_SCANCODE_D});
    
    std::cout << "Connecting to server at " << serverAddress << ":" << publisherPort << "/" << pullPort << std::endl;
    
    if (!client.ConnectToServer(serverAddress, publisherPort, pullPort)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    std::cout << "Client connected successfully!" << std::endl;
    std::cout << "Client ID: " << client.GetClientId() << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  A/D - Move left/right" << std::endl;
    std::cout << "  SPACE - Jump" << std::endl;
    std::cout << "  Shift+A - Dash left (chord)" << std::endl;
    std::cout << "  Shift+D - Dash right (chord)" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "The client will send input to server and receive game state updates" << std::endl;
    
    // Run the client (this will handle input, networking, and rendering)
    client.Run();
    
    client.Shutdown();
    std::cout << "Client shutdown complete" << std::endl;
    
    return 0;
}