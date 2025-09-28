#pragma once
#include <vec2.h>

#define P_CLIENT_HELLO 1
#define P_CLIENT_INPUT 2

#define P_ENTITY_SPAWN 3
#define P_ENTITY_UPDATE_PUT 4
#define P_ENTITY_UPDATE_FETCH 5
#define P_STREAM_DONE 6

#define P_DISCONNECT 9

#define K_KEYDOWN -7
#define K_KEYUP 7

#define P_MAP_TYPE_A 10
#define P_MAP_TYPE_B 11
#define P_MAP_TYPE_C 12

typedef struct packet_def {
    char packet_type;
    char entity_id;
    char client_id;
    vec2 position;
    vec2 velocity;
    vec2 force;
    vec2 dimensions;
} packet_def;

typedef struct rr_packet {
    char packet_type;
    char entity_id;
    char client_id;
    short keycode;
    char keystate;
    int numkeys;
};

typedef struct ps_packet {
    char packet_type;
    char entity_id;
    vec2 position;
    vec2 velocity;
    vec2 force;
    vec2 dimensions;
};