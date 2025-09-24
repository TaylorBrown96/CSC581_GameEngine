#include "NetClient.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <chrono>

static void closesock(socket_t s) {
#if defined(_WIN32)
  closesocket(s);
#else
  close(s);
#endif
}

bool SimpleTcpClient::connect(const std::string& host, int port) {
  _host = host; _port = port;
#if defined(_WIN32)
  WSADATA w; WSAStartup(MAKEWORD(2,2), &w);
#endif
  _stop.store(false);
  _th = std::thread(&SimpleTcpClient::readerThread, this);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  return true;
}

void SimpleTcpClient::close() {
  _stop.store(true);
  if (_th.joinable()) _th.join();
  if (_sock != INVALID_SOCKET) { closesock(_sock); _sock = INVALID_SOCKET; }
#if defined(_WIN32)
  WSACleanup();
#endif
}

std::unordered_map<std::string, NetEntity> SimpleTcpClient::snapshot() {
  std::lock_guard<std::mutex> lk(_mtx);
  return _ents;
}

void SimpleTcpClient::move(int dx, int dy) {
  if (!_connected.load() || _sock == INVALID_SOCKET) return;
  char line[64];
  std::snprintf(line, sizeof(line), "MOVE %d %d\n", dx, dy);
  std::lock_guard<std::mutex> lk(_send_mtx);
  send(_sock, line, (int)std::strlen(line), 0);
}

void SimpleTcpClient::readerThread() {
  _sock = socket(AF_INET, SOCK_STREAM, 0);
  if (_sock == INVALID_SOCKET) return;

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_port);
  inet_pton(AF_INET, _host.c_str(), &addr.sin_addr);

  if (::connect(_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
    closesock(_sock); _sock = INVALID_SOCKET; return;
  }
  _connected.store(true);

  std::string line;
  char buf[512];
  while (!_stop.load()) {
    int n = recv(_sock, buf, sizeof(buf)-1, 0);
    if (n <= 0) break;
    buf[n] = 0;
    for (int i=0; i<n; ++i) {
      if (buf[i] == '\n') {
        applyLine(line);
        line.clear();
      } else if (buf[i] != '\r') {
        line.push_back(buf[i]);
      }
    }
  }
  _connected.store(false);
  closesock(_sock); _sock = INVALID_SOCKET;
}

void SimpleTcpClient::applyLine(const std::string& line) {
  std::istringstream is(line);
  std::string cmd; is >> cmd;
  if (cmd == "WELCOME") {
    is >> _myId;
  } else if (cmd == "SPAWN") {
    NetEntity e; is >> e.id >> e.x >> e.y >> e.tint;
    std::lock_guard<std::mutex> lk(_mtx);
    _ents[e.id] = e;
  } else if (cmd == "UPDATE") {
    std::string id; int x,y; is >> id >> x >> y;
    std::lock_guard<std::mutex> lk(_mtx);
    auto it = _ents.find(id);
    if (it != _ents.end()) { it->second.x = x; it->second.y = y; }
  } else if (cmd == "DESPAWN") {
    std::string id; is >> id;
    std::lock_guard<std::mutex> lk(_mtx);
    _ents.erase(id);
  }
}
