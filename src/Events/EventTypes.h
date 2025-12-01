#pragma once
#include <climits>
enum EventType {
    EVENT_TYPE_INPUT = INT_MAX - 1,
    EVENT_TYPE_COLLISION = INT_MAX - 2,
    EVENT_TYPE_SPAWN = INT_MAX - 3,
    EVENT_TYPE_REPLAY_START = INT_MAX - 4,
    EVENT_TYPE_REPLAY_STOP = INT_MAX - 5
};
