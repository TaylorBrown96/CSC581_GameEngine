#pragma once

#include "EventSystem.h"
#include "EventTypes.h"

typedef enum ReplayEventType {
    START = EventType::EVENT_TYPE_REPLAY_START,
    STOP = EventType::EVENT_TYPE_REPLAY_STOP,
    PLAY_START = EventType::EVENT_TYPE_REPLAY_START / 2,
    PLAY_STOP = EventType::EVENT_TYPE_REPLAY_STOP / 2 - 1
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
  static ReplayEvent *PlayStart() {
    auto *rpl = new ReplayEvent();
    rpl->timestamp = 0.0;
    rpl->type = ReplayEventType::PLAY_START;
    return rpl;
  }
   static ReplayEvent *PlayStop() {
    auto *rpl = new ReplayEvent();
    rpl->timestamp = 0.0;
    rpl->type = ReplayEventType::PLAY_STOP;
    return rpl;
  }
};