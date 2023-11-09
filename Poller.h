#pragma once

#include <unordered_map>
#include <vector>

#include "./Timestamp.h"
#include "./noncopyable.h"

//事件分发器的核心模块：IO多路复用
namespace mymuduo {

class Channel;
class EventLoop;

class Poller : noncopyable {
 private:
  EventLoop* ownerLoop_;  // 事件循环

 protected:
  // Map的key：socketfd
  using ChannelMap = std::unordered_map<int, Channel*>;
  ChannelMap channels_;

 public:
  using ChannelList = std::vector<Channel*>;
  Poller(EventLoop* loop);
  virtual ~Poller() = default;
  // IO复用的统一接口
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
  virtual void updateChannel(Channel* channel) = 0;
  virtual void removeChannel(Channel* channel) = 0;
  // 判断 channel是否存在在当前poller中
  bool hasChannel(Channel* channel) const;

  static Poller* NewDefaultPoller(EventLoop* eventloop);
};

}  // namespace mymuduo