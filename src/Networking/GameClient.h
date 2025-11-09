// GameClient.h
#pragma once
#include "Core/GameEngine.h"
#include "Events/EventSystem.h"
#include "Events/EventTypes.h"
#include "Replay/ReplaySystem.h"

#include <zmq.hpp>
#include <string>
#include <map>
#include <functional>


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
    
    // Camera offset tracking
    int playerEntityId;  // The entity ID that represents this client's player

public:
    GameClient();
    ~GameClient();

    // Client-specific methods
    bool ConnectToServer(const std::string& address, int pubPort, int pullPort, const std::string& clientId = "");
    void DisconnectFromServer();
    void SendMessageToServer(const std::string& message);
    void SendInputToServer();
    std::string GetLastGameState();

    bool StartReplayPlayback(const std::string& filePath);
    void StopReplayPlayback();
    bool IsReplayPlaying() const;
    
    // Connection management
    bool IsConnected() const { return isConnected; }
    std::string GetClientId() const { return clientId; }
    
    // Camera management
    void SetPlayerEntityId(int entityId) { playerEntityId = entityId; }
    int GetPlayerEntityId() const { return playerEntityId; }
    
    // Override base class methods if needed
    bool Initialize(const char* title, int resx, int resy, float timeScale);
    void Run();
    void Shutdown();
    void RegisterEntity(const std::string& entityType, std::function<Entity*()> constructor);

private:
    void ProcessServerMessages();
    void HandleIncomingMessage(const std::string& messageStr);
    void ProcessStringEntityData(const std::string& entityData);
    void SyncEntityWithStringData(Entity* entity, float x, float y, float offSetX, float offSetY, float width, float height, 
                                 float velX, float velY, int textureState, int currentFrame, bool visible);
};