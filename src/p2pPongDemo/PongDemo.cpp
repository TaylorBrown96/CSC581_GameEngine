#include "PongDemo.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

bool PongDemo::Boot(const char* title, int w, int h, float timeScale,
                    const std::string& trackerRep, const std::string& trackerPub)
{
    if (!Initialize(title, w, h, timeScale)) {
        std::cerr << "[PongDemo] Initialize failed\n";
        return false;
    }
    node_.setOnPeerMessage([this](const std::string& s){ onPeerMessage(s); });
    P2PNode::Config cfg; cfg.trackerRep = trackerRep; cfg.trackerPub = trackerPub;
    if (!node_.start(cfg)) {
        std::cerr << "[PongDemo] P2PNode start failed\n";
        return false;
    }
    myId_ = node_.peerId();
    myY_  = H_/2.0f - PH_/2.0f;
    return true;
}

void PongDemo::onPeerMessage(const std::string& payload) {
    std::istringstream ps(payload);
    std::string tag; ps >> tag;
    if (tag == "PADDLE") {
        int pid; float y; ps >> pid >> y;
        std::lock_guard<std::mutex> lk(mtx_);
        if (otherId_ == -1 && pid != myId_) {
            otherId_ = pid;
            authority_ = node_.isAuthority();
        }
        bool amLeft = (myId_ < otherId_);
        if (pid == otherId_) {
            if (amLeft) paddleR_ = y; else paddleL_ = y;
        }
    } else if (tag == "BALL") {
        float x,y,vx,vy; int l,r; ps >> x >> y >> vx >> vy >> l >> r;
        std::lock_guard<std::mutex> lk(mtx_);
        if (!authority_) {
            ballX_ = x; ballY_ = y; ballVX_ = vx; ballVY_ = vy;
            scoreL_ = l; scoreR_ = r;
        }
    } else if (tag == "JOINED") {
        // optional
    }
}

void PongDemo::drawRect(SDL_Renderer* r, int x, int y, int w, int h) {
    SDL_FRect rect{(float)x,(float)y,(float)w,(float)h};
    SDL_RenderFillRect(r, &rect);
}

void PongDemo::RunLoop() {
    using clock = std::chrono::steady_clock;
    auto last = clock::now();
    bool running = true;

    // Try to use an engine-created renderer; if none, create our own.
    SDL_Window*    win = SDL_GetWindowFromID(1);
    SDL_Renderer*  ren = win ? SDL_GetRenderer(win) : nullptr;
    if (!ren) {
        if (!win) win = SDL_CreateWindow("P2P Pong Demo", W_, H_, SDL_WINDOW_RESIZABLE);
        ren = SDL_CreateRenderer(win, nullptr);
        if (!ren) {
            std::cerr << "[PongDemo] No renderer available: " << SDL_GetError() << "\n";
            return;
        }
    }

    // Learn authority/peer immediately (don’t wait for first message)
    myId_      = node_.peerId();
    authority_ = node_.isAuthority();
    otherId_   = node_.otherId();  // may be -1 until the second client joins

    while (running) {
        // Events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            }
        }

        // Input (SDL3: const bool*)
        const bool* keys = SDL_GetKeyboardState(nullptr);
        float step = 300.0f * (1.0f/60.0f);
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])   myY_ -= step;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) myY_ += step;
        if (myY_ < 0) myY_ = 0;
        if (myY_ > H_ - PH_) myY_ = float(H_ - PH_);

        // Keep authority/peer in sync in case a second client connects later
        authority_ = node_.isAuthority();
        otherId_   = node_.otherId();

        // Always publish our paddle
        {
            std::ostringstream os; os << "PADDLE " << myId_ << " " << myY_;
            node_.publish(os.str());
        }

        // If authority, integrate ball and broadcast
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (authority_) {
                auto now = clock::now();
                float dt = std::chrono::duration<float>(now - last).count();
                last = now;

                ballX_ += ballVX_ * dt;
                ballY_ += ballVY_ * dt;

                // walls
                if (ballY_ < 0)            { ballY_ = 0;           ballVY_ = std::fabs(ballVY_); }
                if (ballY_ > H_ - BW_)     { ballY_ = H_ - BW_;    ballVY_ = -std::fabs(ballVY_); }

                // collisions (only when we have a peer)
                if (otherId_ != -1) {
                    if (ballX_ <= 20 + PW_ && ballX_ >= 20 &&
                        ballY_ + BW_ >= paddleL_ && ballY_ <= paddleL_ + PH_ && ballVX_ < 0) {
                        ballVX_ = std::fabs(ballVX_) * 1.02f;
                    }
                    if (ballX_ + BW_ >= W_ - 20 - PW_ && ballX_ <= W_ - 20 &&
                        ballY_ + BW_ >= paddleR_ && ballY_ <= paddleR_ + PH_ && ballVX_ > 0) {
                        ballVX_ = -std::fabs(ballVX_) * 1.02f;
                    }
                }

                // scoring
                if (ballX_ < 0) {
                    scoreR_++; ballX_ = W_/2; ballY_ = H_/2; ballVX_ = 180.0f; ballVY_ = 120.0f;
                }
                if (ballX_ > W_) {
                    scoreL_++; ballX_ = W_/2; ballY_ = H_/2; ballVX_ = -180.0f; ballVY_ = -120.0f;
                }

                std::ostringstream bs;
                bs << "BALL " << ballX_ << " " << ballY_ << " " << ballVX_ << " " << ballVY_
                   << " " << scoreL_ << " " << scoreR_;
                node_.publish(bs.str());
            }
        }

        // Mirror our paddle locally for smoothness (left/right decided by ids)
        {
            std::lock_guard<std::mutex> lk(mtx_);
            bool amLeft = (otherId_ == -1) || (myId_ < otherId_);
            if (amLeft) paddleL_ = myY_; else paddleR_ = myY_;
        }

        // Render
        SDL_SetRenderDrawColor(ren, 30,30,30,255);
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 230,230,230,255);

        for (int y=0; y<H_; y+=20) {
            SDL_FRect d{ (float)(W_/2-1), (float)y, 2.0f, 10.0f };
            SDL_RenderFillRect(ren, &d);
        }
        // snapshot under lock to avoid tearing
        float lY, rY, bX, bY;
        { std::lock_guard<std::mutex> lk(mtx_); lY = paddleL_; rY = paddleR_; bX = ballX_; bY = ballY_; }
        SDL_FRect pL{ 20.f,          lY, (float)PW_, (float)PH_ };
        SDL_FRect pR{ (float)(W_-20-PW_), rY, (float)PW_, (float)PH_ };
        SDL_FRect ball{ bX, bY, (float)BW_, (float)BW_ };
        SDL_RenderFillRect(ren, &pL);
        SDL_RenderFillRect(ren, &pR);
        SDL_RenderFillRect(ren, &ball);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    node_.stop();
    Shutdown();
}
