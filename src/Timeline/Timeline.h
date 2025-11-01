#pragma once
#include <vector>

class Timeline {
public:
    enum class State { RUN, PAUSE };


    Timeline(float s = 1.0f, Timeline* p = nullptr) {
        this->scale = s;
        this->parent = p;
        this->state = p ? p->getState() : State::RUN;
        this->elapsedTime = 0.0f;
        this->absoluteScale = p ? p->getAbsoluteScale() * s : s;

        if(p) {
            p->addChild(this);
        }
    }


    void Update(float rawDeltaTime) {
        if(state == State::RUN) {
            // Store the scaled delta time for this timeline
            deltaTime = rawDeltaTime * absoluteScale;
            elapsedTime += deltaTime;
        } else {
            // When paused, delta time should be 0
            deltaTime = 0.0f;
        }
        
        if(children.size() > 0) {
            for(auto child : children) {
                child->Update(rawDeltaTime);
            }
        }
    }

    
    float getDeltaTime() {
        return deltaTime;
    }


    void setScale(float scale) {
        if(scale >= 0.5f && scale <= 2.0f) {
            this->scale = scale;
            this->absoluteScale = parent ? parent->getAbsoluteScale() * scale : scale;

            if(children.size() > 0) {
                for(auto child : children) {
                    child->updateAbsoluteScale();
                }
            }
        }
    }


    void updateAbsoluteScale() {
        this->absoluteScale = parent ? parent->getAbsoluteScale() * scale : scale;
    }


    float getScale() {
        return scale;
    }


    float getAbsoluteScale() {
        return absoluteScale;
    }


    void setState(State state) {
        this->state = state;
        if(children.size() > 0) {
            for(auto child : children) {
                child->setState(state);
            }
        }
    }


    State getState() {
        return state;
    }


    void addChild(Timeline *child) {
        children.push_back(child);
    }

    float getElapsedTime() {
        return elapsedTime;
    }


private:
    float scale;
    float absoluteScale;
    float deltaTime;
    Timeline* parent;
    std::vector<Timeline*> children;
    State state;
    float elapsedTime;
};