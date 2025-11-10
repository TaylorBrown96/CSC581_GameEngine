#pragma once

enum EventType {
    EVENT_TYPE_INPUT = __INT_MAX__ - 1,
    EVENT_TYPE_COLLISION = __INT_MAX__ - 2,
    EVENT_TYPE_SPAWN = __INT_MAX__ - 3,
    EVENT_TYPE_REPLAY_START = __INT_MAX__ - 4,
    EVENT_TYPE_REPLAY_STOP = __INT_MAX__ - 5
};