#pragma ocne

#include <sys/epoll.h>

#include <vector>

#include "Poller.h"
/**
 * epoll的使用
 * epoll_create
 * epoll_ctl add/mod/del
 * epoll_wait
 */
namespace mymuduo {
class EPollPoller : public Poller {
 private:
  using EventList = std::vector<epoll_event>;
  int epollfd_;
  EventList events_;

  static const int kInitEventListSize = 16;
  static const char* operatorToString(int op);

  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  void update(int operation, Channel* channel);

 public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller() override;

  //重写基类的三种函数
  Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;
};

}  // namespace mymuduo
