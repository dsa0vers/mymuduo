#pragma once

#include "noncopyable.h"

namespace mymuduo {
class InetAddress;
class Socket : noncopyable {
 public:
  explicit Socket(int socketfd) : socketfd_(socketfd) {}
  ~Socket();

  int fd() const { return socketfd_; }
  void bindAddress(const InetAddress& localaddr);
  void listen();
  int accept(InetAddress* peeraddr);

  void shutdownWrite();

  void setTcpDelay(bool on);
  void setReuseAddr(bool on);
  void setReusePort(bool on);
  void setKeepAlive(bool on);

 private:
  const int socketfd_;
};
}  // namespace mymuduo