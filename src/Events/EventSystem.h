#pragma once
#include "Timeline/Timeline.h"
#include "Entities/Entity.h"

#include <variant>
#include <map>
#include <queue>

// TODO make this into variants? bit too unsafe right now imo, but variants will have less flexibility

typedef struct Event {
    int type;
    float timestamp;
    void* data;
    size_t size;
} Event;

// overloaded in main
class EventHandler {
    std::map<std::string, Component> components;
    mutable std::mutex componentMutex;
public:
    EventHandler() {}
    
    // user override
    virtual void OnEvent(Event E) = 0;
    
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

    // event priority comparator

    class EventPriority {
        public: bool operator()(Event A, Event B) {
            if (A.timestamp > B.timestamp) {
                return true;
            }
            else return false;
        }
    };

    std::map<int, std::vector<EventHandler*>> EventHandlers;
    // std::vector<Event> events;
    std::priority_queue<Event, std::vector<Event>, EventManager::EventPriority> events;
    Timeline* timeline; // global timeline or per-handler?
    
public:
    EventManager(Timeline* tl) : timeline(tl) {}
   
    void RegisterEventHandler(int eventType, EventHandler* eventHandler) {
        EventHandlers[eventType].push_back(eventHandler);
    }

    void Raise(Event E) {
        events.push(E);
    }

    void HandleCurrentEvents() {
        float currentTimestamp = timeline->getElapsedTime();

        std::vector<Event> futureEvents;
        while (!events.empty()) {

            Event e = events.top();

            // if this is a future event
            if (e.timestamp > currentTimestamp) {
                // remove event from prio queue and preserve it in futureEvents
                futureEvents.push_back(e);
                
            }
            else {
                // handle the event
                std::vector<EventHandler*> evHandlers = EventHandlers[e.type];
                for (int i = 0; i < evHandlers.size(); i++) {
                    evHandlers[i]->OnEvent(e);
                }   
            }    

            events.pop();
        }

        // push future events back into prio queue
        for (int i = 0; i < futureEvents.size(); i++) {
            events.push(futureEvents[i]);
        }
    }
};