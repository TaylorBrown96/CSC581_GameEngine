// GameClient.cpp
#include "GameClient.h"
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>
#include <functional>
#include <set>

using namespace std;

// GameClient Implementation
GameClient::GameClient() : GameEngine(), isConnected(false), serverPublisherPort(0), serverPullPort(0) {
    // Initialize ZeroMQ context
    zmqContext = std::make_unique<zmq::context_t>(1);
    subscriberSocket = std::make_unique<zmq::socket_t>(*zmqContext, ZMQ_SUB);
    pushSocket = std::make_unique<zmq::socket_t>(*zmqContext, ZMQ_PUSH);
    
    // Generate a unique client ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    clientId = "Client_" + std::to_string(dis(gen));
}

GameClient::~GameClient() {
    DisconnectFromServer();
}

bool GameClient::ConnectToServer(const std::string& address, int pubPort, int pullPort, const std::string& clientId) {

    if (!clientId.empty()) {
        this->clientId = clientId;
    }
    
    serverAddress = address;
    serverPublisherPort = pubPort;
    serverPullPort = pullPort;
    
    // Connect subscriber socket to server's publisher (receive game state)
    std::string subscriberAddress = "tcp://" + address + ":" + std::to_string(pubPort);
    subscriberSocket->connect(subscriberAddress);
    subscriberSocket->set(zmq::sockopt::subscribe, ""); // Subscribe to all messages
    
    // Set high water mark to control message buffering
    subscriberSocket->set(zmq::sockopt::rcvhwm, 0);  // Receive HWM: buffer up to 10 messages
    subscriberSocket->set(zmq::sockopt::sndhwm, 1);  // Send HWM: buffer up to 10 messages
    
    std::cout << "Connected to server publisher at: " << subscriberAddress << std::endl;
    
    // Connect push socket to server's pull socket (send messages to server)
    std::string pushAddress = "tcp://" + address + ":" + std::to_string(pullPort);
    pushSocket->connect(pushAddress);
    std::cout << "Connected to server pull socket at: " << pushAddress << std::endl;
    
    isConnected = true;
    
    // Clear any existing entities to start fresh
    auto entityMgr = GetEntityManager();
    if (entityMgr) {
        entityMgr->ClearAllEntities();
        std::cout << "Cleared existing entities for fresh sync" << std::endl;
    }
    
    // Send initial connection message
    SendMessageToServer("CONNECT:" + this->clientId);
    
    std::cout << "GameClient " << this->clientId << " connected successfully to server" << std::endl;
    return true;
}

void GameClient::DisconnectFromServer() {
    // Send disconnect message before closing
    if (isConnected) {
        SendMessageToServer("DISCONNECT:" + clientId);
    }
    
    // Close sockets
    if (subscriberSocket) {
        subscriberSocket->close();
    }
    if (pushSocket) {
        pushSocket->close();
    }
    
    isConnected = false;
    std::cout << "GameClient " << clientId << " disconnected from server" << std::endl;
}

void GameClient::SendMessageToServer(const std::string& message) {
    if (!isConnected || !pushSocket) {
        return;
    }
    zmq::message_t zmqMessage(message.size());
    memcpy(zmqMessage.data(), message.c_str(), message.size());
    pushSocket->send(zmqMessage, zmq::send_flags::dontwait);
}

void GameClient::SendInputToServer() {
    if (!isConnected) {
        return;
    }
    
    // Get the input manager
    auto inputManager = GetInput();
    if (!inputManager) {
        return;
    }
    
    // Get all currently active actions
    std::vector<std::string> activeActions = inputManager->GetActiveActions();
    
    // Create action message
    std::stringstream actionMessage;
    actionMessage << "ACTIONS:" << clientId << ":";
    
    // Add all active actions
    for (size_t i = 0; i < activeActions.size(); ++i) {
        if (i > 0) actionMessage << ",";
        actionMessage << activeActions[i];
    }
    
    SendMessageToServer(actionMessage.str());
}

std::string GameClient::GetLastGameState() {
    return lastReceivedGameState;
}

void GameClient::ProcessServerMessages() {
    if (!isConnected || !subscriberSocket) {
        return;
    }
    
    zmq::message_t message;
    auto result = subscriberSocket->recv(message, zmq::recv_flags::dontwait);
    
    if (result) {
        std::string messageStr(static_cast<char*>(message.data()), message.size());
        std::cout << "Client " << clientId << " received message: " << messageStr << std::endl;
        // Process simple string entity data
        ProcessStringEntityData(messageStr);
    }
    
}

bool GameClient::Initialize(const char* title, int resx, int resy, float timeScale) {
    // Call base class initialization
    if (!GameEngine::Initialize(title, resx, resy, timeScale)) {
        return false;
    }
    
    // Client-specific initialization
    std::cout << "GameClient " << clientId << " initialized" << std::endl;
    
    return true;
}

void GameClient::Run() {
    // Override base class run to include network input sending
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    Uint32 lastInputSend = SDL_GetTicks();
    
    // Get access to base class members through public methods
    auto inputManager = GetInput();
    auto entityMgr = GetEntityManager();

    // We need to track running state ourselves since it's private in base class
    bool clientRunning = true;

    while (clientRunning) {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (inputManager && inputManager->IsKeyPressed(SDL_SCANCODE_ESCAPE))) {
                clientRunning = false;
            }
        }
        
        // Process server messages (non-blocking)
        ProcessServerMessages();
        std::vector<Entity *> entities;
        if (entityMgr) {
            entities = entityMgr->getEntityVectorRef();
        }

        // Update input
        if (inputManager) {
            inputManager->Update();
        }
        
        // Send input to server periodically (every 50ms)
        if (isConnected && (currentTime - lastInputSend) > 50) {
            SendInputToServer();
            lastInputSend = currentTime;
        }
        Render(entities);
        float delay = std::max(0.0, 1000.0 / 60.0 - deltaTime);
        std::cout << "delay: " << delay << std::endl;
        SDL_Delay(delay);
    }
}

void GameClient::Shutdown() {
    DisconnectFromServer();
    // Call base class shutdown
    GameEngine::Shutdown();
}

void GameClient::ProcessStringEntityData(const std::string& entityData) {
    if (entityData.empty()) {
        return;
    }
    
    std::cout << "Client " << clientId << " processing entity data: " << entityData << std::endl;
    
    // Split by newlines to get individual entity strings
    std::vector<std::string> entityLines;
    std::stringstream ss(entityData);
    std::string line;
    
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            entityLines.push_back(line);
        }
    }
        
    // Get the entity manager and sync entities
    auto entityMgr = GetEntityManager();
    if (!entityMgr) {
        return;
    }
    
    std::vector<Entity*> localEntities = entityMgr->getEntityVectorRef();
    std::cout << "Client " << clientId << " has " << localEntities.size() << " local entities" << std::endl;
    
    // Track which server entities we've seen
    std::set<int> serverEntityIds;
    
    // For each entity from server, find or create corresponding local entity
    for (const std::string& entityLine : entityLines) {
        // Parse entity data: id,x,y,width,height,velocityX,velocityY,textureState,visible
        std::vector<std::string> parts;
        std::stringstream entityStream(entityLine);
        std::string part;
        
        while (std::getline(entityStream, part, ',')) {
            parts.push_back(part);
        }
        
        if (parts.size() < 9) {
            std::cerr << "Invalid entity data: " << entityLine << std::endl;
            continue;
        }
        
        // Parse values
        int id = std::stoi(parts[0]);
        std::string entityType = parts[1];
        float x = std::stof(parts[2]);
        float y = std::stof(parts[3]);
        float width = std::stof(parts[4]);
        float height = std::stof(parts[5]);
        float velX = std::stof(parts[6]);
        float velY = std::stof(parts[7]);
        int textureState = std::stoi(parts[8]);
        int currentFrame = std::stoi(parts[9]);
        bool visible = (std::stoi(parts[10]) == 1);
        
        // Track this server entity
        serverEntityIds.insert(id);
        
        // Find existing entity with this ID
        Entity* localEntity = nullptr;
        for (Entity* entity : localEntities) {
            if (entity->GetId() == id) {
                localEntity = entity;
                break;
            }
        }
        
        if (localEntity) {
            std::cout << "Client " << clientId << " updating existing entity " << id << std::endl;
            SyncEntityWithStringData(localEntity, x, y, width, height, velX, velY, textureState, currentFrame, visible);
        } else {
            std::cout << "Client " << clientId << " creating new entity " << id << " of type " << entityType << std::endl;
            // Try to find a registered entity factory for this type
            auto it = this->entityFactory.find(entityType);
            if (it != this->entityFactory.end()) {
                localEntity = it->second();  // Call the factory function
            } else {
                localEntity = new Entity(x, y, width, height);
            }
            // CRITICAL: Override the auto-generated ID with server ID
            localEntity->SetId(id);
            localEntity->entityType = entityType;
            entityMgr->AddEntity(localEntity);
            // Sync with server data (this will override any factory defaults)
            SyncEntityWithStringData(localEntity, x, y, width, height, velX, velY, textureState, currentFrame, visible);
        }
    }
    
    // Remove any local entities that are no longer on the server
    auto& entities = entityMgr->getEntityVectorRef();
    for (auto it = entities.begin(); it != entities.end();) {
        if (serverEntityIds.find((*it)->GetId()) == serverEntityIds.end()) {
            std::cout << "Client " << clientId << " removing entity " << (*it)->GetId() << " (no longer on server)" << std::endl;
            delete *it;
            it = entities.erase(it);
        } else {
            ++it;
        }
    }
}

void GameClient::SyncEntityWithStringData(Entity* entity, float x, float y, float width, float height, 
                                         float velX, float velY, int textureState, int currentFrame, bool visible) {
    // Update entity properties from string data
    entity->SetPosition(x, y);
    entity->dimensions = {.x = width, .y = height};
    entity->velocity = {.x = velX, .y = velY};
    entity->currentTextureState = textureState;
    entity->currentFrame = currentFrame;
    entity->isVisible = visible;
    
    // Debug logging to verify sync
    std::cout << "Synced Entity ID " << entity->GetId() << " (" << entity->entityType 
              << ") to position (" << x << ", " << y << ") dimensions (" << width << ", " << height << ")" << std::endl;
}

void GameClient::RegisterEntity(const std::string& entityType, std::function<Entity*()> constructor) {
    entityFactory[entityType] = constructor;
}