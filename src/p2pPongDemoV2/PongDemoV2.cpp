#include "PongDemoV2.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

bool PongDemoV2::Boot(const char* title, int w, int h, float timeScale,
                      const std::string& trackerRep, const std::string& trackerPub)
{
    if (!Initialize(title, w, h, timeScale)) { std::cerr << "[PongDemoV2] Initialize failed\n"; return false; }
    node_.setOnPeerMessage([this](const std::string& s){ onPeerMessage(s); });
    P2PNodeV2::Config cfg; cfg.trackerRep = trackerRep; cfg.trackerPub = trackerPub;
    if (!node_.start(cfg)) { std::cerr << "[PongDemoV2] P2PNodeV2 start failed\n"; return false; }
    myId_ = node_.peerId(); myY_ = H_/2.0f - PH_/2.0f;
    return true;
}

void PongDemoV2::drawRect(SDL_Renderer* r, int x, int y, int w, int h) {
    SDL_FRect rect{(float)x,(float)y,(float)w,(float)h};
    SDL_RenderFillRect(r, &rect);
}

void PongDemoV2::onPeerMessage(const std::string& payload) {
    std::istringstream ps(payload);
    std::string tag; ps >> tag;

    if (tag == "PADDLE") {
        int pid; float y; ps >> pid >> y;
        std::lock_guard<std::mutex> lk(mtx_);
        if (otherId_ == -1 && pid != myId_) otherId_ = pid;
        bool amLeft = (otherId_ == -1) || (myId_ < otherId_);
        if (pid == otherId_) { if (amLeft) paddleR_ = y; else paddleL_ = y; }
        return;
    }

    if (tag == "BALL2") {
        int pid; std::uint64_t clk; float x,y,vx,vy; ps >> pid >> clk >> x >> y >> vx >> vy;
        std::lock_guard<std::mutex> lk(mtx_);
        if (clk > lastBallClock_ || (clk == lastBallClock_ && pid < myId_)) {
            lastBallClock_ = clk;
            // Smooth position in the main loop, but adopt the peer's velocity directly:
            ballTX_ = x;  ballTY_ = y;
            ballTVX_ = vx; ballTVY_ = vy;
            ballVX_  = vx; ballVY_  = vy;   // <- new
        }
        return;
    }

    if (tag == "SCORE2") {
        int pid, sl, sr; ps >> pid >> sl >> sr;
        std::lock_guard<std::mutex> lk(mtx_);
        scoreL_ = std::max(scoreL_, sl);
        scoreR_ = std::max(scoreR_, sr);
        return;
    }
}

void PongDemoV2::RunLoop() {
    using clock = std::chrono::steady_clock;
    auto last = clock::now();
    bool running = true;
    auto lastBallSend = clock::now();

    SDL_Window*   win = SDL_GetWindowFromID(1);
    SDL_Renderer* ren = win ? SDL_GetRenderer(win) : nullptr;
    if (!ren) {
        if (!win) win = SDL_CreateWindow("P2P Pong Demo (Decentralized)", W_, H_, SDL_WINDOW_RESIZABLE);
        ren = SDL_CreateRenderer(win, nullptr);
        if (!ren) { std::cerr << "[PongDemoV2] No renderer: " << SDL_GetError() << "\n"; node_.stop(); Shutdown(); return; }
    }

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = false;
        }

        // Input
        const bool* keys = SDL_GetKeyboardState(nullptr);
        float step = 50.0f * (1.0f/60.0f);
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])   myY_ -= step;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) myY_ += step;
        if (myY_ < 0) myY_ = 0; if (myY_ > H_-PH_) myY_ = float(H_-PH_);

        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        {
            std::lock_guard<std::mutex> lk(mtx_);
            // Smooth toward last adopted target to converge
            const float k = 0.35f;
            ballX_  += (ballTX_  - ballX_)  * k;
            ballY_  += (ballTY_  - ballY_)  * k;
            //ballVX_ += (ballTVX_ - ballVX_) * k;
            //ballVY_ += (ballTVY_ - ballVY_) * k;

            // Integrate
            ballX_ += ballVX_ * dt;
            ballY_ += ballVY_ * dt;

            // Walls
            if (ballY_ < 0)           { ballY_ = 0;        ballVY_ = std::fabs(ballVY_); }
            if (ballY_ > H_-BW_)      { ballY_ = H_-BW_;   ballVY_ = -std::fabs(ballVY_); }

            // Decide sides when we know other
            bool amLeft = (otherId_ == -1) || (myId_ < otherId_);

            // Paddles
            if (ballX_ <= 20 + PW_ && ballX_ >= 20 &&
                ballY_ + BW_ >= paddleL_ && ballY_ <= paddleL_ + PH_ && ballVX_ < 0) {
                ballVX_ =  std::fabs(ballVX_) * 1.02f;
            }
            if (ballX_ + BW_ >= W_ - 20 - PW_ && ballX_ <= W_ - 20 &&
                ballY_ + BW_ >= paddleR_ && ballY_ <= paddleR_ + PH_ && ballVX_ > 0) {
                ballVX_ = -std::fabs(ballVX_) * 1.02f;
            }

            // Scoring (monotonic local counters)
            bool scored = false;
            if (ballX_ < 0) {
                scoreR_local_++; scored = true;
                ballX_ = W_/2; ballY_ = H_/2; ballVX_ = 210.f;  ballVY_ = 130.f;
            } else if (ballX_ > W_) {
                scoreL_local_++; scored = true;
                ballX_ = W_/2; ballY_ = H_/2; ballVX_ = -210.f; ballVY_ = -130.f;
            }

            // Set new target (what we publish)
            ballTX_ = ballX_; ballTY_ = ballY_; ballTVX_ = ballVX_; ballTVY_ = ballVY_;

            // Publish local paddle and ball (no authority)
            {
                // Always send my paddle every frame
                std::ostringstream p; p << "PADDLE " << myId_ << " " << myY_;
                node_.publish(p.str());
            }
            {
                // Send BALL2 at ~20 Hz
                auto nowSend = clock::now();
                if (std::chrono::duration<float, std::milli>(nowSend - lastBallSend).count() >= 50.0f) {
                    lastBallSend = nowSend;
                    ++lclock_;
                    std::ostringstream b; b << "BALL2 " << myId_ << " " << lclock_ << " "
                                            << ballX_ << " " << ballY_ << " "
                                            << ballVX_ << " " << ballVY_;
                    node_.publish(b.str());
                }
            }
            if (scored) {
                std::ostringstream s; s << "SCORE2 " << myId_ << " " << scoreL_local_ << " " << scoreR_local_;
                node_.publish(s.str());
            }

            scoreL_ = std::max(scoreL_, scoreL_local_);
            scoreR_ = std::max(scoreR_, scoreR_local_);
        }

        // Mirror my paddle locally
        {
            std::lock_guard<std::mutex> lk(mtx_);
            bool amLeft = (otherId_ == -1) || (myId_ < otherId_);
            if (amLeft) paddleL_ = myY_; else paddleR_ = myY_;
        }

        // Render
        SDL_SetRenderDrawColor(ren, 30,30,30,255);
        SDL_RenderClear(ren);
        SDL_SetRenderDrawColor(ren, 230,230,230,255);
        for (int y=0; y<H_; y+=20) { SDL_FRect d{ (float)(W_/2-1), (float)y, 2.f, 10.f }; SDL_RenderFillRect(ren,&d); }

        float lY,rY,bX,bY;
        { std::lock_guard<std::mutex> lk(mtx_); lY = paddleL_; rY = paddleR_; bX = ballX_; bY = ballY_; }
        SDL_FRect pL{ 20.f, (float)lY, (float)PW_, (float)PH_ };
        SDL_FRect pR{ (float)(W_-20-PW_), (float)rY, (float)PW_, (float)PH_ };
        SDL_FRect ball{ (float)bX, (float)bY, (float)BW_, (float)BW_ };
        SDL_RenderFillRect(ren, &pL); SDL_RenderFillRect(ren, &pR); SDL_RenderFillRect(ren, &ball);
        SDL_RenderPresent(ren);
        SDL_Delay(1); 
    }

    node_.stop();
    Shutdown();
}
