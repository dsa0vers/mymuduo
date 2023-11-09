#include <mymuduo/Logger.h>
#include <mymuduo/TcpServer.h>

#include <functional>
#include <string>
using namespace mymuduo;

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& addr, const std::string& name)
      : server_(loop, addr, name), loop_(loop) {
    //注册回调函数
    server_.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));

    server_.setThreadNum(3);
  }

  void start() { server_.start(); }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
      LOG_INFO("Connection UP : %s", conn->peerAddr().toIpPort().c_str());
    } else {
      LOG_INFO("Connection DOWN : %s", conn->peerAddr().toIpPort().c_str());
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    std::string msg = buf->retrieveAllAsString();
    conn->send(msg);
    conn->shutdown();
  }
  TcpServer server_;
  EventLoop* loop_;
};

int main() {
  EventLoop loop;
  InetAddress addr(8000);
  EchoServer server(&loop, addr, "EchoServer-01");
  server.start();
  loop.loop();

  return 0;
}