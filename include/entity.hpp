#ifndef ENTITY_H
#define ENTITY_H

#include <sprite.hpp>

typedef struct Entity {
    float posx;
    float posy;
    float scalex;
    float scaley;
    Sprite* sprite;
    /**
    References to objects in other managers will go here
    eg;
    Physics* physics;
    Audio* audio;
    etc..
    */
} Entity;

void moveEntity(Entity* ent, float posx, float posy);
void scaleEntity(Entity* ent, float scalex, float scaley);

#endif