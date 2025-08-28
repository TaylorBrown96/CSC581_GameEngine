#include <entity.hpp>

void moveEntity(Entity* ent, float posx, float posy) {
    ent->posx = posx;
    ent->posy = posy;
    if (ent->sprite)
        moveSpriteTo(ent->sprite, posx, posy);
}

void scaleEntity(Entity* ent, float scalex, float scaley) {
    ent->scalex = scalex;
    ent->scaley = scaley;
    if (ent->sprite)
        scaleSprite(ent->sprite, scalex, scaley);
}