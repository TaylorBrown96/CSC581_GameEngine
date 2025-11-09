#include "ReplaySystem.h"
#include <limits>

ReplaySystem::ReplaySystem(float bufferSeconds)
    : bufferLength(bufferSeconds),
      currentTime(0.0f),
      playbackTime(0.0f),
      playbackStartTime(0.0f),
      playbackEndTime(0.0f),
      recordedDuration(0.0f),
      mode(Mode::Idle) {}

void ReplaySystem::StartRecording() {
    if (mode == Mode::Recording) return;

    // When starting a new recording session we reset time and buffers.
    currentTime = 0.0f;
    playbackTime = 0.0f;
    playbackStartTime = 0.0f;
    playbackEndTime = 0.0f;
    recordedDuration = 0.0f;

    for (auto& t : tracks) {
        // Trim everything; we are starting fresh.
        t->TrimOlderThan(std::numeric_limits<float>::max());
    }

    mode = Mode::Recording;
    if (onRecordingStarted) onRecordingStarted();
}

void ReplaySystem::StopRecording() {
    if (mode != Mode::Recording) return;

    mode = Mode::Idle;
    if (onRecordingStopped) onRecordingStopped();
}

void ReplaySystem::StartPlayback() {
    if (tracks.empty()) return;

    bool anySamples = false;
    float globalOldest = 0.0f;
    float globalNewest = 0.0f;

    for (auto& t : tracks) {
        if (t->Empty()) continue;
        float oldest = t->GetOldestTime();
        float newest = t->GetNewestTime();
        if (!anySamples) {
            anySamples = true;
            globalOldest = oldest;
            globalNewest = newest;
        } else {
            if (oldest < globalOldest) globalOldest = oldest;
            if (newest > globalNewest) globalNewest = newest;
        }
    }

    if (!anySamples) return;

    playbackStartTime = globalOldest;
    playbackEndTime   = globalNewest;
    playbackTime      = playbackStartTime;
    recordedDuration  = playbackEndTime - playbackStartTime;

    mode = Mode::Playing;
    if (onPlaybackStarted) onPlaybackStarted();

    // Immediately apply the first frame so there is no jump.
    ApplyFrame();
}

void ReplaySystem::StopPlayback() {
    if (mode != Mode::Playing) return;

    mode = Mode::Idle;
    if (onPlaybackStopped) onPlaybackStopped();
}

void ReplaySystem::Update(float deltaSeconds) {
    if (mode == Mode::Idle) return;

    if (mode == Mode::Recording) {
        currentTime += deltaSeconds;
        CaptureFrame();

        // Maintain sliding window of bufferLength seconds.
        float minTime = currentTime - bufferLength;
        if (minTime < 0.0f) minTime = 0.0f;

        bool anySamples = false;
        float oldest = 0.0f;
        float newest = 0.0f;

        for (auto& t : tracks) {
            t->TrimOlderThan(minTime);
            if (t->Empty()) continue;
            float tOld = t->GetOldestTime();
            float tNew = t->GetNewestTime();
            if (!anySamples) {
                anySamples = true;
                oldest = tOld;
                newest = tNew;
            } else {
                if (tOld < oldest) oldest = tOld;
                if (tNew > newest) newest = tNew;
            }
        }

        if (anySamples) {
            recordedDuration = newest - oldest;
        } else {
            recordedDuration = 0.0f;
        }
    } else if (mode == Mode::Playing) {
        playbackTime += deltaSeconds;
        if (playbackTime > playbackEndTime) {
            // End of replay window.
            StopPlayback();
            return;
        }
        ApplyFrame();
    }
}

void ReplaySystem::CaptureFrame() {
    for (auto& t : tracks) {
        t->CaptureSample(currentTime);
    }
}

void ReplaySystem::ApplyFrame() {
    for (auto& t : tracks) {
        t->ApplyAtTime(playbackTime);
    }
}
