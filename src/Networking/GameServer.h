// GameServer.h
#pragma once
#include "Core/GameEngine.h"
#include <zmq.hpp>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <queue>

// GameServer class that inherits from GameEngine
class GameServer : public GameEngine {
private:
    // ZeroMQ context and sockets
    std::unique_ptr<zmq::context_t> zmqContext;
    std::unique_ptr<zmq::socket_t> publisherSocket;  // PUB for broadcasting to clients
    std::unique_ptr<zmq::socket_t> pullSocket;       // PULL for receiving from clients
    
    // Server-specific members
    bool isServerRunning;
    int publisherPort;
    int pullPort;
    std::string publisherAddress;
    std::string pullAddress;
    
    // Connection management
    std::vector<std::string> connectedClients;
    std::mutex clientsMutex;
    
    // Client-to-entity mapping
    std::unordered_map<std::string, Entity*> clientToEntityMap;
    std::mutex entityMapMutex;
    
    // Player entity factory - allows developers to specify their own player entity class
    // Parameters: renderer
    std::function<Entity*(SDL_Renderer*)> playerEntityFactory;
    
    // Simple thread pool for processing client messages
    std::vector<std::thread> workerThreads;
    std::queue<std::string> messageQueue;
    std::mutex messageQueueMutex;
    std::condition_variable messageCondition;
    std::atomic<bool> shouldStopWorkers;
    
    // Message processing thread
    std::thread messageProcessorThread;
    
    // Thread control
    bool shouldStop;

public:
    GameServer();
    ~GameServer();

    // Server-specific methods
    bool StartServer(int pubPort, int pullPort);
    void StopServer();
    void BroadcastGameState(const std::string& gameState);
    
    // Connection management
    void AddClient(const std::string& clientId);
    void RemoveClient(const std::string& clientId);
    std::vector<std::string> GetConnectedClients();
    
    // Player entity management
    void SetPlayerEntityFactory(std::function<Entity*(SDL_Renderer*)> factory);
    Entity* SpawnPlayerEntity(const std::string& clientId);
    void DespawnPlayerEntity(const std::string& clientId);
    Entity* GetPlayerEntity(const std::string& clientId);
    
    // Override base class methods if needed
    bool Initialize(const char* title, int resx, int resy);
    void Run();
    void Shutdown();
    
    // Signal handling
    void RequestStop() { shouldStop = true; }

private:
    void MessageProcessorThread();
    void WorkerThreadFunction();
    void ProcessMessage(const std::string& message);
    void ProcessClientActions(const std::string& clientId, const std::string& actionsData);
    std::string SerializeEntityVector(const std::vector<Entity*>& entities);
};
