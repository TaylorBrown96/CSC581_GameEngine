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
GameClient::GameClient() : GameEngine(), isConnected(false), serverPublisherPort(0), serverPullPort(0), 
                           playerEntityId(-1) {
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

    if (!subscriberSocket) {
        subscriberSocket = std::make_unique<zmq::socket_t>(*zmqContext, ZMQ_SUB);
    }
    if (!pushSocket) {
        pushSocket = std::make_unique<zmq::socket_t>(*zmqContext, ZMQ_PUSH);
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
        subscriberSocket.reset();
    }
    if (pushSocket) {
        pushSocket->close();
        pushSocket.reset();
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

    if (replayRecorder && replayRecorder->IsPlaying()) {
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
    
    // If no actions are active, send IDLE
    if (activeActions.empty()) {
        actionMessage << "IDLE";
    } else {
        // Add all active actions
        for (size_t i = 0; i < activeActions.size(); ++i) {
            if (i > 0) actionMessage << ",";
            actionMessage << activeActions[i];
        }
    }
    
    SendMessageToServer(actionMessage.str());
}

std::string GameClient::GetLastGameState() {
    return lastReceivedGameState;
}

bool GameClient::StartReplayPlayback(const std::string& filePath) {
    if (!replayRecorder) {
        return false;
    }

    replayRecorder->StopRecording();
    DisconnectFromServer();

    playerEntityId = -1;
    auto entityMgr = GetEntityManager();
    if (entityMgr) {
        entityMgr->ClearAllEntities();
    }

    bool started = replayRecorder->StartPlayback(filePath);
    if (started) {
        std::cout << "GameClient entering replay playback from " << replayRecorder->GetActivePlaybackFile() << std::endl;
    }
    return started;
}

void GameClient::StopReplayPlayback() {
    if (!replayRecorder) {
        return;
    }

    bool shouldReconnect = !isConnected && !serverAddress.empty();
    std::string reconnectAddress = serverAddress;
    int reconnectPubPort = serverPublisherPort;
    int reconnectPullPort = serverPullPort;
    std::string reconnectClientId = clientId;

    replayRecorder->StopPlayback();
    SDL_Log("Replay playback stopped.");

    playerEntityId = -1;
    auto entityMgr = GetEntityManager();
    if (entityMgr) {
        entityMgr->ClearAllEntities();
    }

    if (shouldReconnect) {
        SDL_Log("Attempting to reconnect to server after replay playback.");
        ConnectToServer(reconnectAddress, reconnectPubPort, reconnectPullPort, reconnectClientId);
    }
}

bool GameClient::IsReplayPlaying() const {
    return replayRecorder && replayRecorder->IsPlaying();
}

void GameClient::ProcessServerMessages() {
    if (replayRecorder && replayRecorder->IsPlaying()) {
        std::string replayMessage;
        while (replayRecorder->TryGetNextPlaybackMessage(replayMessage)) {
            HandleIncomingMessage(replayMessage);
        }
        return;
    }

    if (!isConnected || !subscriberSocket) {
        return;
    }

    zmq::message_t message;
    auto result = subscriberSocket->recv(message, zmq::recv_flags::dontwait);
    if (!result) {
        return;
    }

    std::string messageStr(static_cast<char*>(message.data()), message.size());
    if (replayRecorder && replayRecorder->IsRecording()) {
        replayRecorder->RecordMessage(messageStr);
    }

    HandleIncomingMessage(messageStr);
}

void GameClient::HandleIncomingMessage(const std::string& messageStr) {
    if (messageStr.empty()) {
        return;
    }

    lastReceivedGameState = messageStr;

    if (messageStr.rfind("PLAYER_ENTITY:", 0) == 0) {
        if (playerEntityId != -1) {
            return;
        }

        size_t firstColon = messageStr.find(':');
        size_t secondColon = messageStr.find(':', firstColon + 1);

        if (firstColon == std::string::npos || secondColon == std::string::npos) {
            return;
        }

        std::string targetClientId = messageStr.substr(firstColon + 1, secondColon - firstColon - 1);
        if (targetClientId != clientId) {
            return;
        }

        std::string entityIdStr = messageStr.substr(secondColon + 1);
        try {
            int entityId = std::stoi(entityIdStr);
            SetPlayerEntityId(entityId);
            std::cout << "Client " << clientId << " assigned player entity ID: " << entityId << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "Failed to parse player entity assignment: " << ex.what() << std::endl;
        }
        return;
    }

    ProcessStringEntityData(messageStr);
}

bool GameClient::Initialize(const char* title, int resx, int resy, float timeScale) {
    // Call base class initialization
    if (!GameEngine::Initialize(title, resx, resy, timeScale)) {
        return false;
    }
    ReplayHandler* rplHandler = new ReplayHandler(replayRecorder.get(), this);
    eventManager->RegisterEventHandler(EventType::EVENT_TYPE_REPLAY_START, rplHandler);
    eventManager->RegisterEventHandler(EventType::EVENT_TYPE_REPLAY_STOP, rplHandler);
    eventManager->RegisterEventHandler(ReplayEventType::PLAY_START, rplHandler);
    eventManager->RegisterEventHandler(ReplayEventType::PLAY_STOP, rplHandler);

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

        bool replayWasPlaying = replayRecorder && replayRecorder->IsPlaying();
        if (replayWasPlaying) {
            replayRecorder->Play();
            if (replayWasPlaying && replayRecorder && !replayRecorder->IsPlaying()) {
                StopReplayPlayback();
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

            if (inputManager->IsKeyPressed(SDL_SCANCODE_H)) {
                if (replayRecorder && !replayRecorder->IsRecording() && !replayRecorder->IsPlaying()) {
                    eventManager->Raise(ReplayEvent::Start());
                    SDL_Log("Replay recording requested via F5.");
                }
            }

            if (inputManager->IsKeyPressed(SDL_SCANCODE_J)) {
                if (replayRecorder && replayRecorder->IsRecording()) {
                    eventManager->Raise(ReplayEvent::Stop());
                    SDL_Log("Replay recording stop requested via F6.");
                }
            }

            if (inputManager->IsKeyPressed(SDL_SCANCODE_K)) {
                if (replayRecorder && !replayRecorder->IsPlaying()) {
                    // std::string lastRecording = replayRecorder->GetLastCompletedRecordingFile();
                    // if (lastRecording.empty()) {
                    //     SDL_Log("No completed replay file found to play.");
                    // } else if (StartReplayPlayback(lastRecording)) {
                    //     SDL_Log("Replay playback started from %s", lastRecording.c_str());
                    // } else {
                    //     SDL_Log("Failed to start replay playback from %s", lastRecording.c_str());
                    // }
                    eventManager->Raise(ReplayEvent::PlayStart());
                }
            }

            if (inputManager->IsKeyPressed(SDL_SCANCODE_L)) {
                if (replayRecorder && replayRecorder->IsPlaying()) {
                    eventManager->Raise(ReplayEvent::PlayStop());

                }
            }
        }
        
        // Send input to server periodically (every 50ms)
        if (isConnected && (currentTime - lastInputSend) > 50) {
            SendInputToServer();
            lastInputSend = currentTime;
        }
        eventManager->HandleCurrentEvents();
        replayRecorder->Record();

        Render(entities);
        float delay = std::max(0.0, 1000.0 / 60.0 - deltaTime);
        SDL_Delay(delay);
    }
}

void GameClient::Shutdown() {
    DisconnectFromServer();
    // Call base class shutdown
    eventManager->Raise(ReplayEvent::Stop());

    GameEngine::Shutdown();
}

void GameClient::ProcessStringEntityData(const std::string& entityData) {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    if (entityData.empty()) {
        return;
    }
        
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
        
        if (parts.size() < 11) {
            std::cerr << "Invalid entity data: " << entityLine << std::endl;
            continue;
        }
        
        // Parse values
        int id = std::stoi(parts[0]);
        std::string entityType = parts[1];
        float x = std::stof(parts[2]);
        float y = std::stof(parts[3]);
        if(id == playerEntityId && playerEntityId != -1) {
            offsetX = std::stof(parts[4]);
            offsetY = std::stof(parts[5]);
        }
        float width = std::stof(parts[6]);
        float height = std::stof(parts[7]);
        float velX = std::stof(parts[8]);
        float velY = std::stof(parts[9]);
        int textureState = std::stoi(parts[10]);
        int currentFrame = std::stoi(parts[11]);
        bool visible = (std::stoi(parts[12]) == 1);
        
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
            SyncEntityWithStringData(localEntity, x, y, offsetX, offsetY, width, height, velX, velY, textureState, currentFrame, visible);
        } else {
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
            SyncEntityWithStringData(localEntity, x, y, offsetX, offsetY, width, height, velX, velY, textureState, currentFrame, visible);
        }
    }
    
    // Remove any local entities that are no longer on the server
    auto& entities = entityMgr->getEntityVectorRef();
    for (auto it = entities.begin(); it != entities.end();) {
        if(playerEntityId != -1) {
            (*it)->rendering.offSetX = offsetX;
            (*it)->rendering.offSetY = offsetY;
        } else {
            (*it)->rendering.offSetX = 0.0f;
            (*it)->rendering.offSetY = 0.0f;
        }
        if (serverEntityIds.find((*it)->GetId()) == serverEntityIds.end()) {
            delete *it;
            it = entities.erase(it);
        } else {
            ++it;
        }
    }
}

void GameClient::SyncEntityWithStringData(Entity* entity, float x, float y, float offSetX, float offSetY, float width, float height, 
                                         float velX, float velY, int textureState, int currentFrame, bool visible) {
    // Update entity properties from string data
    entity->SetPosition(x, y);
    entity->rendering.offSetX = offSetX;
    entity->rendering.offSetY = offSetY;
    entity->dimensions = {.x = width, .y = height};
    // Update physics component if it exists
    if (entity->physicsEnabled && entity->hasComponent("physics")) {
        auto& physics = entity->getComponent<PhysicsComponent>("physics");
        physics.velocity = {.x = velX, .y = velY};
    }
    
    // Update rendering component
    entity->rendering.currentTextureState = textureState;
    entity->rendering.currentFrame = currentFrame;
    entity->rendering.isVisible = visible;
}

void GameClient::RegisterEntity(const std::string& entityType, std::function<Entity*()> constructor) {
    entityFactory[entityType] = constructor;
}

void ReplayHandler::OnEvent(Event *E){
    SDL_Log("Recording Event Launched.\n");

    if (!rplRecordRef)
      return;

    if (E->type == ReplayEventType::START) {
      if (!rplRecordRef->IsRecording()) {
        if (!rplRecordRef->StartRecording()) {
          SDL_Log("Failed to start replay recording.");
        }
      }
    } else if (E->type == ReplayEventType::STOP) {
      if (rplRecordRef->IsRecording()) {
        rplRecordRef->StopRecording();
      }
      if (rplRecordRef->IsPlaying()) {
        rplRecordRef->StopPlayback();
      }
    }
    else if (E->type == ReplayEventType::PLAY_START) {
      std::string lastRecording = rplRecordRef->GetLastCompletedRecordingFile();
      if (lastRecording.empty()) {
          SDL_Log("No completed replay file found to play.");
      } else if (cl->StartReplayPlayback(lastRecording)) {
          SDL_Log("Replay playback started from %s", lastRecording.c_str());
      } else {
          SDL_Log("Failed to start replay playback from %s", lastRecording.c_str());
      }

    }
    else if (E->type == ReplayEventType::PLAY_STOP) {
      cl->StopReplayPlayback();
      SDL_Log("Replay playback stopped.");
    }
  }