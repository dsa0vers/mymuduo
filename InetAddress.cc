#include "InetAddress.h"

#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
namespace mymuduo {

InetAddress::InetAddress(u_int16_t port, std::string ip) {
  bzero(&addr_, sizeof addr_);
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);  //将本地字节序转换成网络(大端)字节序
  addr_.sin_addr.s_addr = inet_addr(
      ip.c_str());  // inet_addr()用来将参数cp
                    // 所指的网络地址字符串转换成网络所使用的二进制数字.
                    // 网络地址字符串是以数字和点组成的字符串, 例如:"127.0.0.1".
                    // c_str()将const string* 类型转换成 const char* 类型
}
std::string InetAddress::toIp() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  return buf;
}
std::string InetAddress::toIpPort() const {
  char buf[64] = {0};
  ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
  size_t end = strlen(buf);
  u_int16_t port = ntohs(addr_.sin_port);
  sprintf(buf + end, ":%u", port);
  return buf;
}
uint16_t InetAddress::Port() const { return ntohs(addr_.sin_port); }


}  // namespace mymuduo

// #include <iostream>
// int main() {
//   mymuduo::sockets::InetAddress addr(8080);
//   std::cout << addr.toIpPort() << std::endl;
//   return 0;
// }