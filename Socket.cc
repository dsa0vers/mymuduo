#include "Socket.h"

#include <netinet/tcp.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "InetAddress.h"
#include "Logger.h"

using namespace mymuduo;

Socket::~Socket() { close(socketfd_); }

void Socket::bindAddress(const InetAddress& localaddr) {
  if (0 != ::bind(socketfd_,
                  (sockaddr*) localaddr.getSocketAddr(),
                  sizeof(sockaddr_in))) {
    LOG_FATAL("bind sockfd:%d fail \n", socketfd_);
  }
}

void Socket::listen() {
  if (0 != ::listen(socketfd_, 1024)) {
    LOG_FATAL("listen sockfd:%d fail \n", socketfd_);
  }
}
int Socket::accept(InetAddress* peeraddr) {
  /**
   * 1. accept函数的参数不合法
   * 2. 对返回的connfd没有设置非阻塞
   * Reactor模型 one loop per thread
   * poller + non-blocking IO
   */
  sockaddr_in addr;
  socklen_t len = sizeof addr;
  bzero(&addr, sizeof addr);
  int connfd = ::accept4(socketfd_, (sockaddr*)&addr, &len,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0) {
    peeraddr->setSocketAddr(addr);
  }
  return connfd;
}

void Socket::shutdownWrite() {
  if (::shutdown(socketfd_, SHUT_WR) < 0) {
    LOG_ERROR("shutdownWrite error");
  }
}

void Socket::setTcpDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}
void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}
void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}
void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socketfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}