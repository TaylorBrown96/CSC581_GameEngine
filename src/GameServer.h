// GameServer.h
#pragma once
#include "GameEngine.h"
#include <zmq.hpp>
#include <thread>
#include <vector>
#include <string>
#include <mutex>

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
    void HandleClientConnections();
    void BroadcastGameState(const std::string& gameState);
    void ProcessClientMessages();
    
    // Connection management
    void AddClient(const std::string& clientId);
    void RemoveClient(const std::string& clientId);
    std::vector<std::string> GetConnectedClients();
    
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
