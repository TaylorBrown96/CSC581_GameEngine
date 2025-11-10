#pragma once
#include "Events/EventSystem.h"
#include "Events/EventTypes.h"
#include <string>

struct InputEvent : public Event {
    std::string actionName;
    Entity* playerEntity;
    
    InputEvent(const std::string& action, Entity* entity) 
        : actionName(action), playerEntity(entity) {
        type = EventType::EVENT_TYPE_INPUT;
        timestamp = 0.0f; 
    }
};

class InputEventHandler : public EventHandler {
    public:
    void OnEvent(Event* E) override {
        if (E->type == EventType::EVENT_TYPE_INPUT) { 
            InputEvent* inputEvent = static_cast<InputEvent*>(E);
            inputEvent->playerEntity->OnActivity(inputEvent->actionName);
        }
    }
};