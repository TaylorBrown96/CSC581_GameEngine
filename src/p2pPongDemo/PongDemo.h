#pragma once
#include "GameEngine.h"
#include "../p2p/P2PNode.h"
#include <SDL3/SDL.h>
#include <mutex>
#include <string>

// Minimal Pong demo that uses your GameEngine for window/rendering and the P2P template for comms.
class PongDemo : public GameEngine {
public:
    PongDemo() : GameEngine(false) {}

    bool Boot(const char* title = "P2P Pong Demo",
              int w = 1000, int h = 1000, float timeScale = 1.0f,
              const std::string& trackerRep = "tcp://127.0.0.1:6000",
              const std::string& trackerPub = "tcp://127.0.0.1:6001");

    void RunLoop();  // simple loop to present a working demo

private:
    void onPeerMessage(const std::string& payload);
    void drawRect(SDL_Renderer* r, int x, int y, int w, int h);

private:
    P2PNode node_;
    // Shared pong state
    std::mutex mtx_;
    int  myId_ = -1;
    int  otherId_ = -1;
    bool authority_ = false;

    // geometry
    const int W_ = 640, H_ = 480, PW_ = 12, PH_ = 80, BW_ = 10;

    // state
    float paddleL_ = 240.0f;
    float paddleR_ = 240.0f;
    float ballX_   = 320.0f;
    float ballY_   = 240.0f;
    float ballVX_  = 180.0f;
    float ballVY_  = 120.0f;
    int   scoreL_  = 0;
    int   scoreR_  = 0;

    // local paddle
    float myY_ = 240.0f;
};
