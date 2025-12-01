#pragma once
#include "Timeline/Timeline.h"
#include "Entities/Entity.h"
#include <SDL3/SDL.h>

#include <variant>
#include <map>
#include <queue>
#include <memory>

// TODO make this into variants? bit too unsafe right now imo, but variants will have less flexibility

typedef struct Event {
    int type;
    float timestamp;
    virtual ~Event() = default;
} Event;

// overloaded in main
class EventHandler {
    std::map<std::string, Component> components;
    mutable std::mutex componentMutex;
public:
    EventHandler() {}
    virtual ~EventHandler() = default;
    
    // user override - use pointer to avoid slicing
    virtual void OnEvent(Event* E) = 0;
    
    // similar api as Entity, just rewritten because inheriting from wntity would be too costly

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

    // event priority comparator for pointers

    class EventPriority {
        public: bool operator()(Event* A, Event* B) {
            if (A->timestamp > B->timestamp) {
                return true;
            }
            else return false;
        }
    };

    std::map<int, std::vector<EventHandler*>> EventHandlers;
    std::priority_queue<Event*, std::vector<Event*>, EventManager::EventPriority> events;
    std::mutex eventsMutex;
    Timeline* timeline; // global timeline or per-handler?
    
public:
    EventManager(Timeline* tl) : timeline(tl) {}
   
    void RegisterEventHandler(int eventType, EventHandler* eventHandler) {

        EventHandlers[eventType].push_back(eventHandler);
    }

    // Use pointer to avoid slicing when passing derived event types
    void Raise(Event* E) {
        std::lock_guard<std::mutex> lock(eventsMutex);
        E->timestamp = timeline->getElapsedTime();
        events.push(E);
    }

    void RaiseDelayed(Event* E, float delay) {
        std::lock_guard<std::mutex> lock(eventsMutex);
        E->timestamp = timeline->getElapsedTime() + delay;
        events.push(E);
    }

    void HandleCurrentEvents() {
        
        float currentTimestamp = timeline->getElapsedTime();
        

        while (!events.empty() && events.top()->timestamp <= currentTimestamp) {
            Event* e = nullptr;
        
            {
                std::lock_guard<std::mutex> lock(eventsMutex); 
                e = events.top();
                events.pop();
            } 
            
            if (e) {
                // handle the event
                std::vector<EventHandler*> evHandlers = EventHandlers[e->type];
                for (int i = 0; i < evHandlers.size(); i++) {
                    evHandlers[i]->OnEvent(e);
                }   

                // Clean up the event after handling
                delete e;
            }
        }
    }
};