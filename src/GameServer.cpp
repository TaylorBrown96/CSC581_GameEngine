// GameServer.cpp
#include "GameServer.h"
#include <iostream>
#include <chrono>
#include <SDL3/SDL.h>
#include <algorithm>
#include <sstream>
using namespace std;

// GameServer Implementation
GameServer::GameServer() : GameEngine(true), isServerRunning(false), publisherPort(0), pullPort(0), shouldStop(false), shouldStopWorkers(false) {
    // Initialize ZeroMQ context
    zmqContext = std::make_unique<zmq::context_t>(1);
    publisherSocket = std::make_unique<zmq::socket_t>(*zmqContext, ZMQ_PUB);
    pullSocket = std::make_unique<zmq::socket_t>(*zmqContext, ZMQ_PULL);
}

GameServer::~GameServer() {
    StopServer();
}

bool GameServer::StartServer(int pubPort, int pullPort) {
    try {
        publisherPort = pubPort;
        this->pullPort = pullPort;
        publisherAddress = "tcp://*:" + std::to_string(publisherPort);
        pullAddress = "tcp://*:" + std::to_string(this->pullPort);
        
        // Bind publisher socket
        publisherSocket->bind(publisherAddress);
        std::cout << "Publisher bound to: " << publisherAddress << std::endl;
        
        // Bind pull socket
        pullSocket->bind(pullAddress);
        std::cout << "Pull socket bound to: " << pullAddress << std::endl;
        
        isServerRunning = true;
        shouldStop = false;
        shouldStopWorkers = false;
        
        // Start message processor thread
        messageProcessorThread = std::thread(&GameServer::MessageProcessorThread, this);
        
        // Start worker threads (simple thread pool)
        int numWorkers = 4;
        
        for (int i = 0; i < numWorkers; ++i) {
            workerThreads.emplace_back(&GameServer::WorkerThreadFunction, this);
        }
        
        std::cout << "GameServer started successfully on ports " << pubPort << " (pub) and " << pullPort << " (pull)" << std::endl;
        std::cout << "Started " << numWorkers << " worker threads" << std::endl;
        return true;
    } catch (const zmq::error_t& e) {
        std::cerr << "Failed to start server: " << e.what() << std::endl;
        return false;
    }
}

void GameServer::StopServer() {
    shouldStop = true;
    shouldStopWorkers = true;
    isServerRunning = false;
    
    // Notify all worker threads to stop
    messageCondition.notify_all();
    
    // Wait for message processor thread to finish
    if (messageProcessorThread.joinable()) {
        messageProcessorThread.join();
    }
    
    // Wait for all worker threads to finish
    for (auto& workerThread : workerThreads) {
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }
    workerThreads.clear();
    
    // Close sockets
    if (publisherSocket) {
        publisherSocket->close();
    }
    if (pullSocket) {
        pullSocket->close();
    }
    
    std::cout << "GameServer stopped" << std::endl;
}

void GameServer::HandleClientConnections() {
    // This is now handled by the message processor and worker threads
}

void GameServer::BroadcastGameState(const std::string& gameState) {
    if (!isServerRunning || !publisherSocket || gameState.empty()) {
        return;
    }
    
    try {
        zmq::message_t message(gameState.size());
        memcpy(message.data(), gameState.c_str(), gameState.size());
        publisherSocket->send(message, zmq::send_flags::dontwait);
    } catch (const zmq::error_t& e) {
        std::cerr << "Failed to broadcast game state: " << e.what() << std::endl;
    }
}

void GameServer::ProcessClientMessages() {
    // This method is now handled by individual client threads
    // This is kept for compatibility but does nothing
}

void GameServer::AddClient(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    connectedClients.push_back(clientId);
    std::cout << "Client connected: " << clientId << std::endl;
}

void GameServer::RemoveClient(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    connectedClients.erase(
        std::remove(connectedClients.begin(), connectedClients.end(), clientId),
        connectedClients.end()
    );
    std::cout << "Client disconnected: " << clientId << std::endl;
}

std::vector<std::string> GameServer::GetConnectedClients() {
    std::lock_guard<std::mutex> lock(clientsMutex);
    return connectedClients;
}

void GameServer::MessageProcessorThread() {
    while (!shouldStop) {
        try {
            // Receive message from pull socket
            zmq::message_t message;
            auto result = pullSocket->recv(message, zmq::recv_flags::dontwait);
            
            if (result) {
                std::string messageStr(static_cast<char*>(message.data()), message.size());
                std::cout << "Server received message: " << messageStr << std::endl;
                
                // Add message to queue for worker threads to process
                {
                    std::lock_guard<std::mutex> lock(messageQueueMutex);
                    messageQueue.push(messageStr);
                }
                messageCondition.notify_one();
            }
        } catch (const zmq::error_t& e) {
            // No message available or other error - this is normal
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Small delay
    }
}

void GameServer::WorkerThreadFunction() {
    while (!shouldStopWorkers) {
        std::string message;
        
        // Wait for a message to process
        {
            std::unique_lock<std::mutex> lock(messageQueueMutex);
            messageCondition.wait(lock, [this] { 
                return !messageQueue.empty() || shouldStopWorkers; 
            });
            
            if (shouldStopWorkers) break;
            
            if (!messageQueue.empty()) {
                message = messageQueue.front();
                messageQueue.pop();
            }
        }
        
        if (!message.empty()) {
            ProcessMessage(message);
        }
    }
}

void GameServer::ProcessMessage(const std::string& message) {
    // Parse and handle different message types
    if (message.find("CONNECT:") == 0) {
        std::string clientId = message.substr(8); // Remove "CONNECT:" prefix
        AddClient(clientId);
    }
    else if (message.find("DISCONNECT:") == 0) {
        std::string clientId = message.substr(11); // Remove "DISCONNECT:" prefix
        RemoveClient(clientId);
    }
    else if (message.find("ACTIONS:") == 0) {
        // Handle action message from client
        // Format: "ACTIONS:ClientId:MOVE_UP,MOVE_LEFT,JUMP"
        size_t firstColon = message.find(':');
        size_t secondColon = message.find(':', firstColon + 1);
        
        if (firstColon != std::string::npos && secondColon != std::string::npos) {
            std::string clientId = message.substr(firstColon + 1, secondColon - firstColon - 1);
            std::string actionsData = message.substr(secondColon + 1);
            
            std::cout << "Received actions from " << clientId << ": " << actionsData << std::endl;
            
            // Process actions and update game state
            ProcessClientActions(clientId, actionsData);
        }
    }
    else {
        // Handle other message types
        std::cout << "Unknown message type: " << message << std::endl;
    }
}

void GameServer::ProcessClientActions(const std::string& clientId, const std::string& actionsData) {
    // Parse comma-separated actions
    std::vector<std::string> actions;
    std::stringstream ss(actionsData);
    std::string action;
    
    while (std::getline(ss, action, ',')) {
        if (!action.empty()) {
            actions.push_back(action);
        }
    }
    
    // Process each action for the client
    for (const auto& actionName : actions) {
        std::cout << "Processing action '" << actionName << "' for client " << clientId << std::endl;
        
        // TODO: Apply game logic based on the action
        // This is where game-specific logic would go
        // For example:
        // - MOVE_UP: Move player up
        // - JUMP: Make player jump
        // - ATTACK: Trigger attack animation/damage
        // etc.
        
        // For now, just log the action
        // In a real game, this would update the game state based on the action
    }
}

bool GameServer::Initialize(const char* title, int resx, int resy) {
    // Call base class initialization with small window size for headless operation
    if (!GameEngine::Initialize("GameServer (Headless)", 320, 240)) {
        return false;
    }
    
    // Server-specific initialization
    std::cout << "GameServer initialized with headless game engine" << std::endl;
    
    return true;
}

void GameServer::Run() {
    std::cout << "Running headless game engine..." << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Run the game engine loop with networking
    auto lastBroadcast = std::chrono::steady_clock::now();
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    
    while (!shouldStop) {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;
        
        // Get entities from the game engine
        auto entityMgr = GetEntityManager();
        std::vector<Entity *> entities;
        if (entityMgr) {
            entities = entityMgr->getEntityVectorRef();
        }

        // Update the game engine (physics, collisions, etc.)
        for (auto &entity : entities) {
            entity->Update(deltaTime / 1000.0f, GetInput(), entityMgr);
            
            // Apply physics if entity has physics enabled
            if (entity->hasPhysics) {
                GetPhysics()->ApplyPhysics(entity, deltaTime / 1000.0f);
            }
        }
        
        // Process collisions
        GetCollision()->ProcessCollisions(entities);
        
        // Process client messages (handled by worker threads)
        HandleClientConnections();
        
        std::string gameState = SerializeEntityVector(entities);
        BroadcastGameState(gameState);        

        float delay = std::max(0.0, 1000.0 / 60.0 - deltaTime);
        SDL_Delay(delay);
    }
    
    std::cout << "Server loop ended" << std::endl;
}

void GameServer::Shutdown() {
    StopServer();
    // Call base class shutdown
    GameEngine::Shutdown();
}

std::string GameServer::SerializeEntityVector(const std::vector<Entity*>& entities) {
    std::stringstream ss;
    
    for (size_t i = 0; i < entities.size(); ++i) {
        Entity* entity = entities[i];
        
        // Format: id,x,y,width,height,velocityX,velocityY,textureState,visible
        ss << entity->GetId() << ","
           << entity->entityType << ","
           << entity->position.x << ","
           << entity->position.y << ","
           << entity->dimensions.x << ","
           << entity->dimensions.y << ","
           << entity->velocity.x << ","
           << entity->velocity.y << ","
           << entity->currentTextureState << ","
           << entity->currentFrame << ","
           << (entity->isVisible ? 1 : 0);
        
        // Add newline to separate entities
        if (i < entities.size() - 1) {
            ss << "\n";
        }
    }
    
    std::string result = ss.str();
    // std::cout << "Serialized " << entities.size() << " entities: " << result << std::endl;
    return result;
}
