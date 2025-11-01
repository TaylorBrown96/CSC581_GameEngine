#pragma once
#include "Timeline/Timeline.h"
#include "Entities/Entity.h"

#include <variant>
#include <map>
#include <queue>

typedef struct Event {
    int type;
} Event;

// overloaded in main
class EventHandler {
    std::map<std::string, Component> components;
    mutable std::mutex componentMutex;
public:
    EventHandler() {}
    
    // user override
    virtual void OnEvent(Event E) = 0;
    
    bool hasComponent(const std::string& key) const {
        std::lock_guard<std::mutex> lock(componentMutex);
        return components.find(key) != components.end();
    }
    
    void setComponent(const std::string& key, Component value) {
        std::lock_guard<std::mutex> lock(componentMutex);
        components[key] = value;
    } 
    
    template <typename T>
    T& getComponent(const std::string& key) {
        std::lock_guard<std::mutex> lock(componentMutex);
        return std::get<T>(components.at(key));
    }

};


class EventManager {
    std::map<int, std::vector<EventHandler*>> EventHandlers;
    std::vector<Event> events;
    Timeline* timeline;

public:
    EventManager(Timeline* tl) : timeline(tl) {}
   
    void RegisterEventHandler(int eventType, EventHandler* eventHandler) {
        EventHandlers[eventType].push_back(eventHandler);
    }

    void Raise(Event E) {
        events.push_back(E);
    }

    void HandleEvents() {
        for (int i = 0; i < events.size(); i++) {
            Event currentEvent = events[i];
            for (int j = 0; j < EventHandlers[currentEvent.type].size(); j++) {
                EventHandlers[currentEvent.type][j]->OnEvent(currentEvent);
            }
        }
        events.clear();
    }
};