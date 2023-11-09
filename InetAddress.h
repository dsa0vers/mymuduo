#pragma once

#include <netinet/in.h>

#include <string>

namespace mymuduo {

class InetAddress {
 private:
  sockaddr_in addr_;

 public:
  explicit InetAddress(u_int16_t port = 0, std::string ip = "127.0.0.1");
  explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}
  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t Port() const;
  const sockaddr_in* getSocketAddr() const { return &addr_; }
  void setSocketAddr(const sockaddr_in& addr) { addr_ = addr; }
};

}  // namespace mymuduo