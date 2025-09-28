// GameClient.h
#pragma once
#include "GameEngine.h"
#include <zmq.hpp>
#include <string>

// GameClient class that inherits from GameEngine
class GameClient : public GameEngine {
private:
    // ZeroMQ context and sockets
    std::unique_ptr<zmq::context_t> zmqContext;
    std::unique_ptr<zmq::socket_t> subscriberSocket;  // Connect to server's publisher
    std::unique_ptr<zmq::socket_t> pushSocket;        // Connect to server's pull socket
    
    // Client-specific members
    bool isConnected;
    std::string serverAddress;
    int serverPublisherPort;
    int serverPullPort;
    std::string clientId;
    
    // Game state
    std::string lastReceivedGameState;
    std::map<std::string, std::function<Entity*()>> entityFactory;

public:
    GameClient();
    ~GameClient();

    // Client-specific methods
    bool ConnectToServer(const std::string& address, int pubPort, int pullPort, const std::string& clientId = "");
    void DisconnectFromServer();
    void SendMessageToServer(const std::string& message);
    void SendInputToServer();
    std::string GetLastGameState();
    
    // Connection management
    bool IsConnected() const { return isConnected; }
    std::string GetClientId() const { return clientId; }
    
    // Override base class methods if needed
    bool Initialize(const char* title, int resx, int resy);
    void Run();
    void Shutdown();
    void RegisterEntity(const std::string& entityType, std::function<Entity*()> constructor);

private:
    void ProcessServerMessages();
    void ProcessStringEntityData(const std::string& entityData);
    void SyncEntityWithStringData(Entity* entity, float x, float y, float width, float height, 
                                 float velX, float velY, int textureState, int currentFrame, bool visible);
};