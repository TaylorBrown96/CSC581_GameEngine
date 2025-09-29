
#include "P2P_main.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <cmath>

using clock_type = std::chrono::steady_clock;

static uint64_t nowMs() {
    return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        clock_type::now().time_since_epoch()).count();
}

bool P2PSkellyGame::Boot(const char* title, int w, int h, float timeScale,
                         const std::string& trackerRep, const std::string& trackerPub)
{
    if (!Initialize(title, w, h, timeScale)) {
        std::cerr << "[P2PMain] Initialize failed\n";
        return false;
    }
    GetRenderSystem()->SetScalingMode(ScalingMode::PROPORTIONAL);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    // Factories used by clients to reconstruct entities from STATE
    auto makePlayer = [this]() -> Entity* {
        auto* e = new TestEntity(100, 100, GetRootTimeline(), GetRenderer());
        e->hasPhysics = true;
        SDL_Texture* t = LoadTexture(GetRenderer(),
            "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
        if (t) { Texture tex = { t, 8, 0, 512, 512, true }; e->SetTexture(0, &tex); }
        return e;
    };
    factory_["TestEntity"] = makePlayer;
    factory_["Player"]     = makePlayer;
    factory_["Skelly"]     = makePlayer;
    factory_["Skeleton"]   = makePlayer;

    factory_["Platform"] = [this]() -> Entity* {
        Platform* p = new Platform(0,0,200,20,false, GetRootTimeline(), GetRenderer());
        p->hasPhysics = false; p->affectedByGravity = false; p->isStatic = true;
        SDL_Texture* t = LoadTexture(GetRenderer(), "media/cartooncrypteque_platform_basicground_idle.bmp");
        if (t) { Texture tex = { t, 1, 1, 200, 20, true }; p->SetTexture(0, &tex); }
        return p;
    };

    // Networking
    node_.setOnPeerMessage([this](const std::string& s){ onPeerMessage(s); });
    P2PNode::Config cfg; cfg.trackerRep = trackerRep; cfg.trackerPub = trackerPub;
    if (!node_.start(cfg)) {
        std::cerr << "[P2PMain] P2PNode start failed\n"; return false;
    }
    myId_    = node_.peerId();
    otherId_ = node_.otherId();
    authority_ = (otherId_ == -1) || (myId_ < otherId_);
    std::cout << "[P2PMain] myId=" << myId_ << " otherId=" << otherId_
              << " authority? " << (authority_ ? "yes" : "no") << "\n";

    if (authority_) {
        spawnStaticWorld();
        spawnPlayerFor(myId_, 100, 100);  // authority's own player
        worldSpawned_ = true;
        std::lock_guard<std::mutex> lk(peersMtx_);
        connectedPeers_.push_back(myId_);
        publishStateNow(); // push immediate state so clients see us right away
    }

    // Bind keys
    GetInput()->AddAction("MOVE_LEFT",  SDL_SCANCODE_A);
    GetInput()->AddAction("MOVE_RIGHT", SDL_SCANCODE_D);
    GetInput()->AddAction("JUMP",       SDL_SCANCODE_SPACE);

    // Announce presence (authority will spawn us on CONNECT)
    sendConnect();
    return true;
}

void P2PSkellyGame::stop() {
    sendDisconnect();
    node_.stop();
    Shutdown();
}

void P2PSkellyGame::spawnStaticWorld() {
    Platform* platform1 = new Platform(300, 800, 300, 75, false, GetRootTimeline(), GetRenderer());
    platform1->hasPhysics = false; platform1->affectedByGravity = false; platform1->isStatic = true;
    GetEntityManager()->AddEntity(platform1);

    Platform* platform2 = new Platform(800, 650, 300, 75, true, GetRootTimeline(), GetRenderer());
    platform2->hasPhysics = true;  platform2->affectedByGravity = false; platform2->isStatic = true;
    GetEntityManager()->AddEntity(platform2);

    SDL_Texture* platformTexture = LoadTexture(GetRenderer(),
        "media/cartooncrypteque_platform_basicground_idle.bmp");
    if (platformTexture) {
        Texture tex = { platformTexture, 1, 1, 200, 20, true };
        platform1->SetTexture(0, &tex);
        platform2->SetTexture(0, &tex);
    }
    std::cout << "[P2PMain] World spawned (2 platforms)\n";
}

Entity* P2PSkellyGame::spawnPlayerFor(int peerId, float x, float y) {
    Timeline* tl = (peerId == myId_ && authority_)
                 ? new Timeline(authorityAnimMul_, GetRootTimeline())
                 : GetRootTimeline();

    TestEntity* e = new TestEntity(x, y, tl, GetRenderer());
    e->hasPhysics = true;

    SDL_Texture* entityTexture = LoadTexture(GetRenderer(),
        "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
    if (entityTexture) {
        Texture tex = { entityTexture, 8, 0, 512, 512, true };
        e->SetTexture(0, &tex);
    }

    GetEntityManager()->AddEntity(e);
    peerToEntity_[peerId] = e;
    std::cout << "[P2PMain] Spawned player for peer " << peerId << " at (" << x << "," << y << ")\n";
    return e;
}

void P2PSkellyGame::despawnPlayerFor(int peerId) {
    auto it = peerToEntity_.find(peerId);
    if (it == peerToEntity_.end()) return;
    Entity* e = it->second;
    GetEntityManager()->RemoveEntity(e);
    delete e;
    peerToEntity_.erase(it);
    std::cout << "[P2PMain] Despawned player for peer " << peerId << "\n";
    publishStateNow(); // propagate removal immediately
}

void P2PSkellyGame::applyActions(Entity* e, const RemoteInput& in) {
    if (!e) return;
    if (in.moveLeft && in.moveRight)      e->OnActivity("IDLE");
    else if (in.moveLeft)                 e->OnActivity("MOVE_LEFT");
    else if (in.moveRight)                e->OnActivity("MOVE_RIGHT");
    else                                  e->OnActivity("IDLE");
    if (in.jump)                          e->OnActivity("JUMP");
}

std::string P2PSkellyGame::serializeEntities(const std::vector<Entity*>& entities) {
    std::ostringstream ss;
    for (size_t i=0;i<entities.size();++i) {
        const Entity* e = entities[i];
        ss << e->GetId() << ","
           << e->entityType << ","
           << e->position.x << ","
           << e->position.y << ","
           << e->dimensions.x << ","
           << e->dimensions.y << ","
           << e->velocity.x  << ","
           << e->velocity.y  << ","
           << e->currentTextureState << ","
           << e->currentFrame << ","
           << (e->isVisible ? 1 : 0);
        if (i+1<entities.size()) ss << "\n";
    }
    return ss.str();
}

void P2PSkellyGame::publishStateNow() {
    auto& entities = GetEntityManager()->getEntityVectorRef();
    node_.publish(std::string("STATE\n") + serializeEntities(entities));
}

Entity* P2PSkellyGame::ensureEntityFor(const std::string& type, int remoteId) {
    auto it = idToEntity_.find(remoteId);
    if (it != idToEntity_.end()) return it->second;

    auto fit = factory_.find(type);
    if (fit == factory_.end()) {
        // Fallback to a player entity if type is unknown
        std::cerr << "[P2PMain] No factory for type: " << type << " -> using default player\n";
        fit = factory_.find("TestEntity");
    }
    if (fit == factory_.end()) return nullptr;

    Entity* e = fit->second();
    GetEntityManager()->AddEntity(e);
    idToEntity_[remoteId] = e;
    return e;
}

void P2PSkellyGame::processState(const std::string& payload) {
    std::istringstream lines(payload);
    std::string line;
    std::set<int> seen;
    while (std::getline(lines, line)) {
        if (line.empty()) continue;
        std::istringstream ls(line);
        int rid; std::string type;
        float x,y,w,h,vx,vy;
        int texState, frame, vis;

        char comma;
        if (!(ls >> rid)) continue;
        if (ls.peek()==',') ls.get();
        std::getline(ls, type, ',');
        ls >> x >> comma >> y >> comma >> w >> comma >> h >> comma
           >> vx >> comma >> vy >> comma >> texState >> comma >> frame >> comma >> vis;

        Entity* e = ensureEntityFor(type, rid);
        if (!e) continue;

        // Non-visual attributes
        e->dimensions = {w,h};
        e->isVisible  = (vis!=0);
        e->currentTextureState = texState;

        // Apply server animation frame to avoid anomalies
        e->currentFrame = frame;

        // Smooth target
        Smooth s; s.tx = x; s.ty = y; s.tvx = vx; s.tvy = vy; s.stamp = nowMs();
        smooth_[rid] = s;

        seen.insert(rid);
    }
    // Remove absent
    std::vector<int> toErase;
    for (auto& kv : idToEntity_) {
        if (!seen.count(kv.first)) {
            kv.second->isVisible = false;
            toErase.push_back(kv.first);
            smooth_.erase(kv.first);
        }
    }
    for (int rid : toErase) idToEntity_.erase(rid);
}

void P2PSkellyGame::sendConnect()    { std::ostringstream s; s << "CONNECT:"    << myId_; node_.publish(s.str()); }
void P2PSkellyGame::sendDisconnect() { std::ostringstream s; s << "DISCONNECT:" << myId_; node_.publish(s.str()); }

void P2PSkellyGame::sendActions(bool left, bool right, bool jump) {
    std::ostringstream s;
    s << "ACTIONS:" << myId_ << ":";
    bool first=true;
    if (left)  { if(!first) s<<","; s<<"MOVE_LEFT";  first=false; }
    if (right) { if(!first) s<<","; s<<"MOVE_RIGHT"; first=false; }
    if (jump)  { if(!first) s<<","; s<<"JUMP"; }
    node_.publish(s.str());
}

void P2PSkellyGame::onPeerMessage(const std::string& s) {
    // Authority handles CONNECT/DISCONNECT/ACTIONS
    if (authority_) {
        if (s.rfind("CONNECT:",0)==0) {
            int pid = std::stoi(s.substr(8));
            {
                std::lock_guard<std::mutex> lk(peersMtx_);
                if (std::find(connectedPeers_.begin(), connectedPeers_.end(), pid) == connectedPeers_.end())
                    connectedPeers_.push_back(pid);
            }
            if (peerToEntity_.find(pid) == peerToEntity_.end()) {
                spawnPlayerFor(pid, 1400, 100);
                publishStateNow(); // push spawn immediately
            }
            return;
        }
        if (s.rfind("DISCONNECT:",0)==0) {
            int pid = std::stoi(s.substr(11));
            {
                std::lock_guard<std::mutex> lk(peersMtx_);
                connectedPeers_.erase(std::remove(connectedPeers_.begin(), connectedPeers_.end(), pid),
                                      connectedPeers_.end());
            }
            inputs_.erase(pid);
            despawnPlayerFor(pid);
            return;
        }
        if (s.rfind("ACTIONS:",0)==0) {
            // ACTIONS:<peerId>:MOVE_LEFT,MOVE_RIGHT,...
            size_t a = s.find(':'), b = s.find(':', a+1);
            if (a!=std::string::npos && b!=std::string::npos) {
                int pid = std::stoi(s.substr(a+1, b-(a+1)));
                std::string list = s.substr(b+1);
                RemoteInput in{};
                std::istringstream ls(list);
                std::string tok;
                while (std::getline(ls, tok, ',')) {
                    if (tok=="MOVE_LEFT")      in.moveLeft  = true;
                    else if (tok=="MOVE_RIGHT") in.moveRight = true;
                    else if (tok=="JUMP")       in.jump      = true;
                }
                inputs_[pid] = in;

                // Backstop: if we somehow missed CONNECT, spawn on first ACTIONS
                if (peerToEntity_.find(pid) == peerToEntity_.end()) {
                    spawnPlayerFor(pid, 1400, 100);
                    publishStateNow();
                }
            }
            return;
        }
    }

    // Client: stash STATE for main thread
    if (!authority_) {
        if (s.rfind("STATE\n",0)==0) {
            std::lock_guard<std::mutex> lk(stateMtx_);
            pendingState_.assign(s.data()+6, s.size()-6);
            hasPendingState_ = true; // overwrite older snapshot
        }
    }
}

void P2PSkellyGame::RunLoop() {
    bool running = true;
    auto lastTick = clock_type::now();

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT || e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            }
        }

        auto now = clock_type::now();
        float dt = std::chrono::duration<float>(now - lastTick).count();
        lastTick = now;

        GetInput()->Update();

        // Recompute authority (lowest known ID)
        otherId_ = node_.otherId();
        bool nowAuthority = (otherId_ == -1) || (myId_ < otherId_);
        if (nowAuthority != authority_) {
            authority_ = nowAuthority;
            std::cout << "[P2PMain] authority switched -> " << (authority_ ? "authority" : "client") << "\n";
            // Lazy init if we just became authority
            if (authority_ && !worldSpawned_) {
                spawnStaticWorld();
                spawnPlayerFor(myId_, 100, 100);
                if (otherId_ != -1 && peerToEntity_.find(otherId_)==peerToEntity_.end())
                    spawnPlayerFor(otherId_, 1400, 100);
                worldSpawned_ = true;
                publishStateNow();
                std::cout << "[P2PMain] World initialized after authority switch\n";
            }
        }

        // Read keys
        const bool* keys = SDL_GetKeyboardState(nullptr);
        bool leftHeld  = keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
        bool rightHeld = keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];
        bool jumpHeld  = keys[SDL_SCANCODE_SPACE];

        if (authority_) {
            // Apply inputs to players (including ours)
            inputs_[myId_] = RemoteInput{leftHeld, rightHeld, jumpHeld};

            for (auto& kv : inputs_) {
                int pid = kv.first;
                RemoteInput in = kv.second;
                Entity* ePlayer = nullptr;
                auto it = peerToEntity_.find(pid);
                if (it != peerToEntity_.end()) ePlayer = it->second;
                applyActions(ePlayer, in);
            }

            // Advance game on authority
            auto& entities = GetEntityManager()->getEntityVectorRef();
            for (auto* ent : entities) {
                ent->Update(dt, GetInput(), GetEntityManager());
                if (ent->hasPhysics) GetPhysics()->ApplyPhysics(ent, dt);
            }
            GetCollision()->ProcessCollisions(entities);

            // Broadcast STATE ~20Hz
            uint64_t ms = nowMs();
            if (ms - lastBroadcastMs_ >= 50) {
                lastBroadcastMs_ = ms;
                node_.publish(std::string("STATE\n") + serializeEntities(entities));
            }

        } else {
            // Client: read newest STATE and smooth towards it
            std::string toApply;
            {
                std::lock_guard<std::mutex> lk(stateMtx_);
                if (hasPendingState_) { toApply.swap(pendingState_); hasPendingState_ = false; }
            }
            if (!toApply.empty()) processState(toApply);

            // Smooth positions for all remote entities
            float alpha = std::min(dt * smoothingHz_, 1.0f);
            for (auto& kv : idToEntity_) {
                int rid = kv.first;
                Entity* e = kv.second;
                auto it = smooth_.find(rid);
                if (it != smooth_.end()) {
                    const Smooth& s = it->second;
                    float nx = e->position.x + (s.tx - e->position.x) * alpha;
                    float ny = e->position.y + (s.ty - e->position.y) * alpha;
                    e->SetPosition(nx, ny);
                    e->velocity = { s.tvx, s.tvy };
                }
            }

            // Only update non-player types locally to avoid fighting server anim state
            auto& entities = GetEntityManager()->getEntityVectorRef();
            for (auto* ent : entities) {
                if (ent->entityType == "Platform") {
                    ent->Update(dt, GetInput(), GetEntityManager());
                }
            }

            // Send my input ~20Hz
            uint64_t ms = nowMs();
            if (ms - lastInputSendMs_ >= 50) {
                lastInputSendMs_ = ms;
                sendActions(leftHeld, rightHeld, jumpHeld);
            }
        }

        // Render
        auto& ents = GetEntityManager()->getEntityVectorRef();
        Render(ents);

        SDL_Delay(1);
    }

    // Inform authority we are leaving (if we are a client)
    if (!authority_) sendDisconnect();

    stop();
}

#ifdef BUILD_P2P_MAIN_STANDALONE
int main() {
    P2PSkellyGame game;
    game.SetAuthorityAnimMultiplier(2.5f); // boost client 1 anim a bit more
    if (!game.Boot("P2P Skelly (Authority)", 1800, 1000, 1.0f)) return 1;
    game.RunLoop();
    return 0;
}
#endif
