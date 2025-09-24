#pragma once
#include <vector>

class Timeline {
public:
    enum class State { RUN, PAUSE};


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


    void switchState() {
        if(state == State::RUN) {
            state = State::PAUSE;
        } else {
            state = State::RUN;
        }
        if(children.size() > 0) {
            for(auto child : children) {
                child->switchState();
            }
        }
    }


    State getState() {
        return state;
    }


    void addChild(Timeline *child) {
        children.push_back(child);
    }


private:
    float scale;
    float absoluteScale;
    Timeline* parent;
    std::vector<Timeline*> children;
    State state;
    float elapsedTime;
};