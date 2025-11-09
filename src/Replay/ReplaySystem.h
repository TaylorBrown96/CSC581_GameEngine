#pragma once

#include "Events/EventSystem.h"
#include "Events/EventTypes.h"


typedef enum ReplayEventType {
    START = EventType::EVENT_TYPE_REPLAY_START,
    STOP = EventType::EVENT_TYPE_REPLAY_STOP
} ReplayEventType;

class ReplayRecorder {
    EntityManager* entityManager = nullptr;
public:
    bool recording = false;
    bool playing = false;
    ReplayRecorder(EntityManager* entityManagerRef) : entityManager(entityManagerRef) 
    {}

    void Record() {
        if (!recording)
            return;

        // RECORDING LOGIC HERE
        SDL_Log("Replay is being recorded\n");
    }

    void Play() {
        if (!playing)
            return;
        // MAYBE PLAYING LOGIC HERE?
    }
};


typedef struct ReplayEvent : public Event {
private:
    ReplayEvent() = default;
public:
    static ReplayEvent* Start() {
        ReplayEvent* rpl = new ReplayEvent();
        rpl->timestamp = 0.0;
        rpl->type = ReplayEventType::START;
        return rpl;
    }
    static ReplayEvent* Stop() {
        ReplayEvent* rpl = new ReplayEvent();
        rpl->timestamp = 0.0;
        rpl->type = ReplayEventType::STOP;
        return rpl;
    }
};

class ReplayHandler : public EventHandler {
    ReplayRecorder* rplRecordRef = nullptr;
public:
    ReplayHandler(ReplayRecorder* pRplRecordRef) : rplRecordRef(pRplRecordRef) 
    {}

    void OnEvent(Event* E) override {
        SDL_Log("Recording Event Launched.\n");

        if (E->type == ReplayEventType::START || E->type == ReplayEventType::STOP) { 
            rplRecordRef->recording = (E->type == ReplayEventType::START);
        }
    }
    
    
};

