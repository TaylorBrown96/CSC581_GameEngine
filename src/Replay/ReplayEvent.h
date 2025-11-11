#pragma once
#include "Events/EventSystem.h"
#include "Events/EventTypes.h"
#include "Networking/GameServer.h"

typedef struct ReplayStartEvent : public Event {
    public:
    ReplayStartEvent() {
        type = EventType::EVENT_TYPE_REPLAY_START;
    }
};

class ReplayHandler : public EventHandler {
ReplaySystem* replay = nullptr;
GameServer* servRef = nullptr;
public:
    ReplayHandler(ReplaySystem* pReplay, GameServer* pserver) : replay(pReplay),
        servRef(pserver) {}
    void OnEvent(Event* E) override {
        if (E->type == EventType::EVENT_TYPE_REPLAY_START)
        if (replay && replay->GetMode() == ReplaySystem::Mode::Recording) {
            // Switch server into playback mode
            replay->StopRecording();
            replay->StartPlayback();

            // Tell all clients to start their own replay playback
            servRef->BroadcastGameState("REPLAY_START");
            // std::cout << "Replay requested by client " << clientId
            //           << " -> starting replay on all clients" << std::endl;
        }
    }
};