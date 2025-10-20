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
    rootTimeline = std::make_unique<Timeline>(1.0f, nullptr);
}

GameServer::~GameServer() {
    StopServer();
}

bool GameServer::StartServer(int pubPort, int pullPort) {
    
    publisherPort = pubPort;
    this->pullPort = pullPort;
    publisherAddress = "tcp://*:" + std::to_string(publisherPort);
    pullAddress = "tcp://*:" + std::to_string(this->pullPort);
    
    // Set high water mark on publisher socket to only keep latest messages
    publisherSocket->set(zmq::sockopt::sndhwm, 1);  // Send HWM: only buffer 1 message
    publisherSocket->set(zmq::sockopt::rcvhwm, 0);  // Receive HWM: no receive buffering needed
    
    // Bind publisher socket
    publisherSocket->bind(publisherAddress);
    std::cout << "Publisher bound to: " << publisherAddress << std::endl;
    
    // Set high water mark on pull socket
    pullSocket->set(zmq::sockopt::rcvhwm, 10);  // Receive HWM: buffer up to 10 client messages
    pullSocket->set(zmq::sockopt::sndhwm, 0);   // Send HWM: no send buffering needed
    
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

    for (int i = 0; i < threadlogs.size(); i++) {
        std::cout<<threadlogs[i]<<"\n";
    }
}

void GameServer::HandleClientConnections() {
    // This is now handled by the message processor and worker threads
}

void GameServer::BroadcastGameState(const std::string& gameState) {
    if (!isServerRunning || !publisherSocket || gameState.empty()) {
        return;
    }
    
    zmq::message_t message(gameState.size());
    memcpy(message.data(), gameState.c_str(), gameState.size());
    publisherSocket->send(message, zmq::send_flags::dontwait);
    
}

void GameServer::ProcessClientMessages() {

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

void GameServer::SetPlayerEntityFactory(std::function<Entity*(SDL_Renderer*)> factory) {
    playerEntityFactory = factory;
}

Entity* GameServer::SpawnPlayerEntity(const std::string& clientId) {
    if (!playerEntityFactory) {
        std::cerr << "Warning: No player entity factory set. Cannot spawn player for client: " << clientId << std::endl;
        return nullptr;
    }
    
    // Get the renderer from the game engine
    SDL_Renderer* renderer = GetRenderer();
    
    Entity* playerEntity = playerEntityFactory(renderer);
    if (playerEntity) {
        std::lock_guard<std::mutex> lock(entityMapMutex);
        clientToEntityMap[clientId] = playerEntity;
        GetEntityManager()->AddEntity(playerEntity);
        std::cout << "Spawned player entity for client: " << clientId << std::endl;
    }
    return playerEntity;
}

void GameServer::DespawnPlayerEntity(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(entityMapMutex);
    auto it = clientToEntityMap.find(clientId);
    if (it != clientToEntityMap.end()) {
        Entity* playerEntity = it->second;
        GetEntityManager()->RemoveEntity(playerEntity);
        delete playerEntity; // Clean up the entity
        clientToEntityMap.erase(it);
        std::cout << "Despawned player entity for client: " << clientId << std::endl;
    }
}

Entity* GameServer::GetPlayerEntity(const std::string& clientId) {
    std::lock_guard<std::mutex> lock(entityMapMutex);
    auto it = clientToEntityMap.find(clientId);
    return (it != clientToEntityMap.end()) ? it->second : nullptr;
}

void GameServer::MessageProcessorThread() {
    while (!shouldStop) {
        zmq::message_t message;
        auto result = pullSocket->recv(message, zmq::recv_flags::dontwait);
        
        if (result) {
            std::string messageStr(static_cast<char*>(message.data()), message.size());
            // std::cout << "Server received message: " << messageStr << std::endl;
            
            // Add message to queue for worker threads to process
            {
                std::lock_guard<std::mutex> lock(messageQueueMutex);
                messageQueue.push(messageStr);
            }
            messageCondition.notify_one();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Small delay
    }
}

void GameServer::WorkerThreadFunction() {
    double time = 0.0;
    int iters = 0;
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
            auto t1 = std::chrono::high_resolution_clock::now();
            ProcessMessage(message);
            auto t2 = std::chrono::high_resolution_clock::now();
            time += (double)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            iters++;
        }
    }
    std::stringstream ss;
    if (iters > 0) {
        ss<<"[Worker Thread \t"<<std::this_thread::get_id()<<"] Message Processing Time: "<<(time / (double)iters)<<" ms (processed "<<iters<<" messages)";
    } else {
        ss<<"[Worker Thread \t"<<std::this_thread::get_id()<<"] Message Processing Time: N/A (no messages processed)";
    }
    std::lock_guard<std::mutex> lk(threadLogMutex);
    threadlogs.push_back(ss.str());
}

void GameServer::ProcessMessage(const std::string& message) {
    // Parse and handle different message types
    if (message.find("CONNECT:") == 0) {
        std::string clientId;

        if (byteSerialize) {
            std::stringstream ss(message);
            std::string part;
            std::vector<std::string> actionVector;
            bool clientproc = false;
            bool actionproc = false;

            while (std::getline(ss, part, ':')) {
                if (part == "CONNECT") {
                    if (actionproc == true) actionproc = false;
                    clientproc = true;
                    continue;
                }
                else if (part == "ACT") {
                    if (clientproc == true) clientproc = false;
                    actionproc = true;
                    continue;
                }

                if (clientproc) {
                    clientId = part;
                }
                else if (actionproc) {
                    actionVector.push_back(part);
                }
            }
            InputManager* im = GetInput();
            std::lock_guard<std::mutex> lk(im->actionsMutex);
            im->SetAllActions(actionVector);
        }
        else {
            clientId = message.substr(8); // Remove "CONNECT:" prefix
        }
        AddClient(clientId);
        SpawnPlayerEntity(clientId);
    }
    else if (message.find("DISCONNECT:") == 0) {
        std::string clientId = message.substr(11); // Remove "DISCONNECT:" prefix
        DespawnPlayerEntity(clientId); // Despawn the player entity
        RemoveClient(clientId);
    }
    else if (message.find("ACTIONS:") == 0) {
        // Handle action message from client
        // Format: "ACTIONS:ClientId:MOVE_UP,MOVE_LEFT,JUMP"
        size_t firstColon = message.find(':');
        size_t secondColon = message.find(':', firstColon + 1);
        std::string clientId;
        
        if (byteSerialize) {
            // FORMAT:
            // ACTIONS:Clientid:(bytearraysize):(bytearray)
            if (firstColon != std::string::npos && secondColon != std::string::npos) 
                clientId = message.substr(firstColon + 1, secondColon - firstColon - 1);

            int* bufptr = (int*)&(message.c_str()[secondColon + 1]);
            int sz = bufptr[0];
            ProcessClientActionsByte(clientId, &(bufptr[1]), sz);
        }
        else {
            if (firstColon != std::string::npos && secondColon != std::string::npos) {
                clientId = message.substr(firstColon + 1, secondColon - firstColon - 1);

                std::string actionsData = message.substr(secondColon + 1);
                
                // std::cout << "Received actions from " << clientId << ": " << actionsData << std::endl;
                
                // Process actions and update game state
                ProcessClientActions(clientId, actionsData);
            }
        }
    }
    else {
        // Handle other message types
        std::cout << "Unknown message type: " << message << std::endl;
    }
}

void GameServer::ProcessClientActionsByte(const std::string& clientId, int* actionsData, int actionsSize) {
    InputManager* im = GetInput();
    std::vector<std::string> act = im->GetAllActions();

    if (actionsSize == 0) {
        GetPlayerEntity(clientId)->OnActivity("");
    }
    else {
        for (int i = 0; i < actionsSize; i++) {
            GetPlayerEntity(clientId)->OnActivity(act[actionsData[i]]);
        }
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
    if (actions.size() == 0) {
        // for idle action
        GetPlayerEntity(clientId)->OnActivity("");
    }
    else {
        for (const auto& actionName : actions) {
            GetPlayerEntity(clientId)->OnActivity(actionName);
        }
    }
}

bool GameServer::Initialize(const char* title, int resx, int resy) {
    // Call base class initialization with small window size for headless operation
    if (!GameEngine::Initialize("GameServer (Headless)", 320, 240, 1.0f)) {
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
    
    auto t1 = std::chrono::high_resolution_clock::now(), 
        t2 = std::chrono::high_resolution_clock::now();
    bool startedcount = false;
    int it = 0;

    double tt = 0.0;
    while (!shouldStop) {  
        if (!startedcount && connectedClients.size() > 0) {
            startedcount = true;
        }

        if (startedcount && connectedClients.size() == 0) {
            startedcount = false;
        }

        
        it += startedcount * 1;
        t1 = std::chrono::high_resolution_clock::now();

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
        GetRootTimeline()->Update(deltaTime / 1000.0f);
        // Update the game engine (physics, collisions, etc.)
        Update(deltaTime, entities);
        
        // Process client messages (handled by worker threads)
        HandleClientConnections();
        
        // Check if 10ms have passed since last broadcast
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastBroadcast = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBroadcast);
        
        if (timeSinceLastBroadcast.count() >= 10) {
            std::string gameState = SerializeEntityVector(entities);
            BroadcastGameState(gameState);
            lastBroadcast = now;
        }

        float delay = std::max(0.0, 1000.0 / 60.0 - deltaTime);
       
        
        t2 = std::chrono::high_resolution_clock::now();
        double t = startedcount * (double)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        tt += t;

        SDL_Delay(delay); 
    }
    // auto t = t2 - t1;
    // double T = (double)std::chrono::duration_cast<std::chrono::milliseconds>(t).count() / (double)it;
    std::cout<<"Time: "<<tt / (double)it<<"\n";
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
        float velX = 0.0f, velY = 0.0f;
        if (entity->physicsEnabled) {
            auto& physics = entity->getComponent<PhysicsComponent>("physics");
            velX = physics.velocity.x;
            velY = physics.velocity.y;
        }

        if (byteSerialize) {

            struct {
                int id;
                vec2 position;
                vec2 dimensions;
                vec2 velocity;
                uint8_t current_tex_state;
                uint8_t current_frame;
                uint8_t visible;
                char nl_or_eof = 0; // string end with 0
            } entData;

            
            entData.id = entity->GetId();
            entData.position = entity->position;
            entData.dimensions = entity->dimensions;
            entData.velocity = {.x = velX, .y = velY};
            entData.current_tex_state = (uint8_t)entity->rendering.currentTextureState;
            entData.current_frame = (uint8_t)entity->rendering.currentFrame;
            entData.visible = (entity->rendering.isVisible ? 1 : 0);

            if (i < entities.size() - 1) {
                entData.nl_or_eof = '\n';
            }
            else
                entData.nl_or_eof = 0;
        

            char* strrep = (char*)&entData;
            std::string sEntity(strrep, sizeof(entData));

            ss << entity->entityType << ":" <<
                sEntity;
            
        }
        else {
            // Format: id,x,y,width,height,velocityX,velocityY,textureState,visible
            
            ss << entity->GetId() << ","
            << entity->entityType << ","
            << entity->position.x << ","
            << entity->position.y << ","
            << entity->dimensions.x << ","
            << entity->dimensions.y << ","
            << velX << ","
            << velY << ","
            << entity->rendering.currentTextureState << ","
            << entity->rendering.currentFrame << ","
            << (entity->rendering.isVisible ? 1 : 0);
            
            // Add newline to separate entities
            if (i < entities.size() - 1) {
                ss << "\n";
            }
        }
    }
    
    std::string result = ss.str();
    return result;
}
