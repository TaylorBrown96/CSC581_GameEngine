#pragma once

#include "EventSystem.h"
#include "EventTypes.h"

typedef enum ReplayEventType {
    START = EventType::EVENT_TYPE_REPLAY_START,
    STOP = EventType::EVENT_TYPE_REPLAY_STOP
  } ReplayEventType;

struct ReplayEvent : public Event {
private:
  ReplayEvent() = default;

public:
  static ReplayEvent *Start() {
    auto *rpl = new ReplayEvent();
    rpl->timestamp = 0.0;
    rpl->type = ReplayEventType::START;
    return rpl;
  }
  static ReplayEvent *Stop() {
    auto *rpl = new ReplayEvent();
    rpl->timestamp = 0.0;
    rpl->type = ReplayEventType::STOP;
    return rpl;
  }
};