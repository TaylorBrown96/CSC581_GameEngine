#pragma once
#include "GameEngine.h"
#include "../p2p_v2/P2PNodeV2.h"
#include <SDL3/SDL.h>
#include <mutex>
#include <cstdint>
#include <algorithm>

class PongDemoV2 : public GameEngine {
public:
    PongDemoV2() : GameEngine(false) {}

    bool Boot(const char* title = "P2P Pong Demo (Decentralized)",
              int w = 1000, int h = 1000, float timeScale = 1.0f,
              const std::string& trackerRep = "tcp://127.0.0.1:6000",
              const std::string& trackerPub = "tcp://127.0.0.1:6001");

    void RunLoop();

private:
    void onPeerMessage(const std::string& payload);
    static void drawRect(SDL_Renderer* r, int x, int y, int w, int h);

    P2PNodeV2 node_;
    std::mutex mtx_;

    int myId_ = -1;
    int otherId_ = -1;

    const int W_ = 640, H_ = 480, PW_ = 12, PH_ = 80, BW_ = 10;

    float paddleL_ = 240.f, paddleR_ = 240.f;
    float myY_ = 240.f;

    // current local ball
    float ballX_ = 320.f, ballY_ = 240.f;
    // was 180/120 — bump up
    float ballVX_ = 210.f, ballVY_ = 130.f;

    // target ball from last winning update
    float ballTX_ = 320.f, ballTY_ = 240.f;
    float ballTVX_ = 210.f, ballTVY_ = 130.f;


    // Lamport clock consensus
    std::uint64_t lclock_ = 0;
    std::uint64_t lastBallClock_ = 0;

    // G-counter scores
    int scoreL_local_ = 0, scoreR_local_ = 0;
    int scoreL_ = 0,       scoreR_       = 0;
};
