#include <algorithm>   // std::clamp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>
#include <cmath>
#include <ctime>       // timestamps
#include <cstdarg>     // logger

#if defined(_WIN32)
  #define NOMINMAX
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  using socklen_t = int;
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  using SOCKET = int;
#endif

// ---------------- tiny logger ----------------
static std::string ts_now() {
  using namespace std::chrono;
  auto now = system_clock::now();
  std::time_t tt = system_clock::to_time_t(now);
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &tt);
#else
  localtime_r(&tt, &tm);
#endif
  char timebuf[32];
  std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", &tm);
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
  char out[48];
  std::snprintf(out, sizeof(out), "%s.%03lld", timebuf, (long long)ms.count());
  return out;
}
static void logf(const char* fmt, ...) {
  std::fprintf(stdout, "[server %s] ", ts_now().c_str());
  va_list ap; va_start(ap, fmt);
  std::vfprintf(stdout, fmt, ap);
  va_end(ap);
  std::fprintf(stdout, "\n");
  std::fflush(stdout);
}
// ------------------------------------------------

struct Entity { std::string id; int x=0; int y=0; unsigned tint=0x66ccff; };
static std::unordered_map<std::string, Entity> g_entities;
static std::mutex g_mtx;
static std::atomic<int> g_nextId{1};
static std::atomic<bool> g_running{true};

struct Client { SOCKET s; std::string id; };
static std::vector<Client> g_clients;
static std::mutex g_clients_mtx;

static void closesock(SOCKET s) {
#if defined(_WIN32)
  closesocket(s);
#else
  close(s);
#endif
}

static void broadcast(const std::string& line) {
  std::lock_guard<std::mutex> lk(g_clients_mtx);
  for (auto it = g_clients.begin(); it != g_clients.end();) {
    const int sent = send(it->s, line.c_str(), (int)line.size(), 0);
    if (sent <= 0) { closesock(it->s); it = g_clients.erase(it); }
    else { ++it; }
  }
}

static void add_env_entity() {
  std::lock_guard<std::mutex> lk(g_mtx);
  if (g_entities.find("ENV") == g_entities.end()) {
    g_entities["ENV"] = Entity{"ENV", 250, 250, 0xFFFFFF}; // client outlines it
  }
}

static void env_tick_loop() {
  using namespace std::chrono;
  auto t0 = steady_clock::now();
  int counter = 0;
  while (g_running.load()) {
    const double t = duration<double>(steady_clock::now() - t0).count();
    const int cx = 250, cy = 250, R = 120;     // 500x500 world
    const int x = cx + int(R * std::cos(t));
    const int y = cy + int(R * std::sin(t));
    {
      std::lock_guard<std::mutex> lk(g_mtx);
      auto it = g_entities.find("ENV");
      if (it != g_entities.end()) { it->second.x = x; it->second.y = y; }
    }
    char buf[128];
    std::snprintf(buf, sizeof(buf), "UPDATE ENV %d %d\n", x, y);
    broadcast(buf);

    if (++counter % 60 == 0) { // ~2s at 30Hz
      logf("ENV at (%d,%d)", x, y);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(33)); // ~30 Hz
  }
}

static void apply_client_move(const std::string& id, int dx, int dy) {
  Entity snap;
  {
    std::lock_guard<std::mutex> lk(g_mtx);
    auto it = g_entities.find(id);
    if (it == g_entities.end()) return;
    it->second.x = std::clamp(it->second.x + dx, 0, 480);
    it->second.y = std::clamp(it->second.y + dy, 0, 480);
    snap = it->second;
  }
  logf("UPDATE %s -> (%d,%d)", snap.id.c_str(), snap.x, snap.y);
  char line[128];
  std::snprintf(line, sizeof(line), "UPDATE %s %d %d\n", snap.id.c_str(), snap.x, snap.y);
  broadcast(line);
}

static void client_thread(SOCKET s) {
  // Assign id + random tint/pos (fit 500x500)
  int idn = g_nextId.fetch_add(1);
  std::string id = "P" + std::to_string(idn);
  std::mt19937 rng((unsigned)std::random_device{}());
  std::uniform_int_distribution<int> dx(20, 460), dy(20, 460);
  std::uniform_int_distribution<int> col(0x202020, 0xDFDFDF);
  Entity me{ id, dx(rng), dy(rng), (unsigned)col(rng) };
  logf("assigned id=%s tint=0x%06X spawn=(%d,%d)", id.c_str(), me.tint, me.x, me.y);

  {
    std::lock_guard<std::mutex> lk(g_mtx);
    g_entities[id] = me;
  }
  {
    std::lock_guard<std::mutex> lk(g_clients_mtx);
    g_clients.push_back({s, id});
  }

  // Welcome + snapshot
  {
    char w[128]; std::snprintf(w, sizeof(w), "WELCOME %s\n", id.c_str());
    send(s, w, (int)std::strlen(w), 0);
    add_env_entity();
    size_t count = 0;
    {
      std::lock_guard<std::mutex> lk(g_mtx);
      for (auto& kv : g_entities) {
        const auto& e = kv.second;
        char line[256];
        std::snprintf(line, sizeof(line), "SPAWN %s %d %d %u\n", e.id.c_str(), e.x, e.y, e.tint);
        send(s, line, (int)std::strlen(line), 0);
        ++count;
      }
    }
    logf("snapshot sent to %s (entities=%zu)", id.c_str(), count);
  }
  // Announce this new player to others
  {
    char line[256];
    std::snprintf(line, sizeof(line), "SPAWN %s %d %d %u\n", me.id.c_str(), me.x, me.y, me.tint);
    broadcast(line);
    logf("broadcast SPAWN %s at (%d,%d)", me.id.c_str(), me.x, me.y);
  }

  // Read loop (handle MOVE dx dy)
  std::string linebuf;
  char buf[512];
  while (g_running.load()) {
    int n = recv(s, buf, sizeof(buf)-1, 0);
    if (n <= 0) break;
    buf[n] = 0;
    for (int i=0; i<n; ++i) {
      const char c = buf[i];
      if (c == '\n') {
        if (!linebuf.empty()) {
          if (linebuf.rfind("MOVE ", 0) == 0) {
            int mdx = 0, mdy = 0;
            std::sscanf(linebuf.c_str() + 5, "%d %d", &mdx, &mdy);
            logf("%s sent MOVE %d %d", id.c_str(), mdx, mdy);
            apply_client_move(id, mdx, mdy);
          }
        }
        linebuf.clear();
      } else if (c != '\r') {
        linebuf.push_back(c);
      }
    }
  }

  // Cleanup
  {
    std::lock_guard<std::mutex> lk(g_mtx);
    g_entities.erase(id);
  }
  {
    std::lock_guard<std::mutex> lk(g_clients_mtx);
    for (auto it = g_clients.begin(); it != g_clients.end(); ++it) {
      if (it->s == s) { g_clients.erase(it); break; }
    }
  }
  char dline[128]; std::snprintf(dline, sizeof(dline), "DESPAWN %s\n", id.c_str());
  broadcast(dline);
  logf("client %s disconnected", id.c_str());
  closesock(s);
}

int main(int argc, char** argv) {
  int port = 7777;
  if (argc > 1) port = std::atoi(argv[1]);

#if defined(_WIN32)
  WSADATA w; WSAStartup(MAKEWORD(2,2), &w);
#endif

  SOCKET listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd == INVALID_SOCKET) { std::printf("socket() failed\n"); return 1; }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  int yes = 1;
#if defined(_WIN32)
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
#else
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

  if (bind(listenfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) { std::printf("bind() failed\n"); return 1; }
  if (listen(listenfd, 16) == SOCKET_ERROR) { std::printf("listen() failed\n"); return 1; }

  logf("listening on port %d", port);
  std::thread env(env_tick_loop);

  while (g_running.load()) {
    sockaddr_in cli{}; socklen_t clen = sizeof(cli);
    SOCKET s = accept(listenfd, (sockaddr*)&cli, &clen);
    if (s == INVALID_SOCKET) continue;
    char ipbuf[64]; inet_ntop(AF_INET, &cli.sin_addr, ipbuf, sizeof(ipbuf));
    logf("client connected from %s:%d", ipbuf, ntohs(cli.sin_port));
    std::thread(client_thread, s).detach();
  }

  env.join();
  closesock(listenfd);
#if defined(_WIN32)
  WSACleanup();
#endif
  return 0;
}
