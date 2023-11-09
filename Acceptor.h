#pragma once
#include "noncopyable.h"
#include <functional>
#include "Socket.h"
#include "Channel.h"

namespace mymuduo{
class EventLoop; 
class InetAddress;
class Acceptor : noncopyable {
 public:
 using  NewConnectionCallback = std::function<void(int sockfd,const InetAddress&)>;
  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_  = cb;}

  void listen();

  bool listening() const { return listening_;}



 private:
    void handleRead();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    bool listening_;

    NewConnectionCallback newConnectionCallback_;
  
};
}