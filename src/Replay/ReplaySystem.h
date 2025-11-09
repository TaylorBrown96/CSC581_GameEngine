#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

/**
 * ReplaySystem
 *
 * A small, modular system that records the last N seconds of arbitrary
 * game state and can play that state back.
 *
 * It does not know anything about Entities, Physics, or the Event system.
 * Instead, devs register "tracks" with capture/apply callbacks.
 *
 * Later, our EventManager can treat replay as just another track whose
 * state is "list of events raised at this time-step".
 */
class ReplaySystem {
public:
    enum class Mode {
        Idle,
        Recording,
        Playing
    };

    // bufferSeconds = how many seconds of history you want (e.g. 5.0f)
    explicit ReplaySystem(float bufferSeconds = 5.0f);

    // TState must be copyable.
    // Returns a track id you can keep if you need it later.
    template <typename TState>
    int RegisterTrack(
        const std::string& name,
        std::function<TState()> captureFunc,
        std::function<void(const TState&)> applyFunc);

    // Control API
    void StartRecording();
    void StopRecording();

    // Begin playback of the last bufferSeconds that were recorded.
    // If less than bufferSeconds were recorded, plays whatever is available.
    void StartPlayback();
    void StopPlayback();

    // Per-frame update. Call once per tick with your simulation delta time.
    void Update(float deltaSeconds);

    Mode GetMode() const { return mode; }

    // Optional hooks future EventManager (or any other system)
    // can subscribe to. For Milestone 4 they are dormant until we plug
    // them into the event system.
    void SetOnRecordingStarted(std::function<void()> cb) { onRecordingStarted = std::move(cb); }
    void SetOnRecordingStopped (std::function<void()> cb) { onRecordingStopped  = std::move(cb); }
    void SetOnPlaybackStarted  (std::function<void()> cb) { onPlaybackStarted   = std::move(cb); }
    void SetOnPlaybackStopped  (std::function<void()> cb) { onPlaybackStopped   = std::move(cb); }

    // How much usable data is currently stored.
    float GetRecordedDuration() const { return recordedDuration; }

private:
    struct ITrack {
        virtual ~ITrack() = default;
        virtual void CaptureSample(float t) = 0;
        virtual void ApplyAtTime(float t) = 0;
        virtual float GetOldestTime() const = 0;
        virtual float GetNewestTime() const = 0;
        virtual void TrimOlderThan(float minTime) = 0;
        virtual bool Empty() const = 0;
    };

    template <typename TState>
    struct Track : ITrack {
        struct Sample {
            float t;
            TState state;
        };

        std::function<TState()> capture;
        std::function<void(const TState&)> apply;
        std::vector<Sample> samples;

        explicit Track(std::function<TState()> c,
                       std::function<void(const TState&)> a)
            : capture(std::move(c)), apply(std::move(a)) {}

        void CaptureSample(float t) override {
            if (!capture) return;
            samples.push_back(Sample{t, capture()});
        }

        void ApplyAtTime(float t) override {
            if (!apply || samples.empty()) return;

            const Sample* best = &samples.front();
            // Simple linear search is fine for 5 seconds of data;
            // Can optimize to binary search later if needed.
            for (const auto& s : samples) {
                if (s.t <= t) best = &s;
                else break;
            }
            apply(best->state);
        }

        float GetOldestTime() const override {
            return samples.empty() ? 0.0f : samples.front().t;
        }

        float GetNewestTime() const override {
            return samples.empty() ? 0.0f : samples.back().t;
        }

        void TrimOlderThan(float minTime) override {
            while (!samples.empty() && samples.front().t < minTime) {
                samples.erase(samples.begin());
            }
        }

        bool Empty() const override {
            return samples.empty();
        }
    };

    void CaptureFrame();
    void ApplyFrame();

    float bufferLength;      // seconds to keep in memory (e.g., 5.0f)
    float currentTime;       // time during recording
    float playbackTime;      // time inside playback window
    float playbackStartTime; // earliest timestamp we will play
    float playbackEndTime;   // latest timestamp we will play
    float recordedDuration;  // duration between oldest and newest sample

    Mode mode;

    std::vector<std::unique_ptr<ITrack>> tracks;

    // Event hooks (initially unused, for future event system integration)
    std::function<void()> onRecordingStarted;
    std::function<void()> onRecordingStopped;
    std::function<void()> onPlaybackStarted;
    std::function<void()> onPlaybackStopped;
};


// Template implementation
template <typename TState>
int ReplaySystem::RegisterTrack(
    const std::string& name,
    std::function<TState()> captureFunc,
    std::function<void(const TState&)> applyFunc)
{
    (void)name; // For now just kept for debugging / future tooling.
    auto track = std::make_unique<Track<TState>>(std::move(captureFunc),
                                                 std::move(applyFunc));
    tracks.push_back(std::move(track));
    return static_cast<int>(tracks.size() - 1);
}
