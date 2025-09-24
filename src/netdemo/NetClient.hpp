#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>

#if defined(_WIN32)
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  using socket_t = SOCKET;
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  using socket_t = int;
#endif

struct NetEntity { std::string id; int x=0; int y=0; unsigned tint=0x66ccff; };

class SimpleTcpClient {
public:
  bool connect(const std::string& host, int port);
  void close();
  bool isConnected() const { return _connected.load(); }
  std::unordered_map<std::string, NetEntity> snapshot();

  // movement (client -> server)
  void move(int dx, int dy);

private:
  void readerThread();
  void applyLine(const std::string& line);

  int _port = 0;
  std::string _host;
  std::thread _th;
  std::atomic<bool> _connected{false};
  std::atomic<bool> _stop{false};

  std::mutex _mtx;
  std::unordered_map<std::string, NetEntity> _ents;
  std::string _myId;

  socket_t _sock = INVALID_SOCKET;
  std::mutex _send_mtx;
};
