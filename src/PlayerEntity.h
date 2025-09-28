#pragma once

#include <GameEngine.h>
#include <iostream>
class PlayerEntity : public Entity {
    InputManager* virtualInputManager = nullptr;
        
public:

    bool OnServer;

    InputManager* getVirtInputManager() {
        return virtualInputManager;
    }

    // PlayerEntity(PlayerEntity* other) = default;

    PlayerEntity(float x, float y, float w, float h) : Entity(x, y, w, h) {
        
        // virtualInputManager = new InputManager(keys);
    }

    // Graphical Updates
    virtual void GraphicalClientUpdate(float dt, InputManager* im, EntityManager* em) {}
    
    // Server side updates
    virtual void ServerUpdate(float dt, InputManager* in, EntityManager* en) {
    }

    virtual void ServerOnCollision(Entity* other, CollisionData* collData) {}

    virtual void OnCollision(Entity* other, CollisionData* collData) override {
        if (OnServer)
            ServerOnCollision(other, collData);
    }

    virtual void Update(float dt, InputManager* in, EntityManager* en) override {
        if (OnServer)
            ServerUpdate(dt, virtualInputManager, en);
        else {
            GraphicalClientUpdate(dt, in, en);
        }
    }
    
    void SetNumKeys(int numkeys) {
        if (virtualInputManager) {
            delete(virtualInputManager);
            virtualInputManager = nullptr;
        }
        
        virtualInputManager = new InputManager(numkeys);
        
    }
};
