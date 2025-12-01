#pragma once
#include "Events/EventSystem.h"
#include "Events/EventTypes.h"
#include "Memory/MemoryPool.h"
#include <string>

struct InputEvent : public Event {
    inline static MemoryPool* InputEventPool;
    std::string actionName;
    Entity* playerEntity;
    
    void* operator new(size_t size) {
        int sl_id = InputEvent::InputEventPool->alloc();
        if (sl_id == -1)
            return nullptr;
        return InputEvent::InputEventPool->getPtr(sl_id);
    }
    
    void operator delete(void* ptr) {
        InputEvent::InputEventPool->freeSlot(InputEvent::InputEventPool->getSlot(ptr));
    }

    InputEvent(const std::string& action, Entity* entity) 
        : actionName(action), playerEntity(entity) {
        type = EventType::EVENT_TYPE_INPUT;
        timestamp = 0.0f; 
    }
};

class InputEventHandler : public EventHandler {
    public:
    InputEventHandler() {
        InputEvent::InputEventPool = new MemoryPool(sizeof(InputEvent), 1024);
    }
    void OnEvent(Event* E) override {
        if (E->type == EventType::EVENT_TYPE_INPUT) { 
            InputEvent* inputEvent = static_cast<InputEvent*>(E);
            inputEvent->playerEntity->OnActivity(inputEvent->actionName);
        }
    }
};