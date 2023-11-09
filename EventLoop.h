#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <vector>
#include <memory>

#include "CurrentThread.h"
#include "Timestamp.h"
#include "noncopyable.h"
/**
 * 时间循环类，主要包括两大模块：Channel and Poller（epoll的抽象）
 */
namespace mymuduo {
class Channel;
class Poller;

class EventLoop : noncopyable {
 public:
  using Functor = std::function<void()>;
  EventLoop();
  ~EventLoop();

  void loop();  //开启循环

  void quit();  //退出循环

  Timestamp pollReturnTime(){return pollReturnTime_;}

  void runInLoop(Functor cb);
  void queueInLoop(Functor cb);

  void wakeup();  //唤醒线程
  void updateChannel(Channel* channel);
  void removeChannel(Channel* chanenl);
  bool hasChannel(Channel* channel);

  bool isInLoopThread() const { return threadId_ == tid(); }

 private:
  void handleRead();         //唤醒
  void doPengingFunctors();  //执行回调

  using ChannelList = std::vector<Channel*>;

  std::atomic_bool looping_;  //原子操作，通过cas实现的
  std::atomic_bool quit_;     //标识退出loop循环

  const pid_t threadId_;  //记录当前的loop所在线程id

  Timestamp pollReturnTime_;  // poller返回事件的时间点
  std::unique_ptr<Poller>
      poller_;  // evntloop中管理的poller，监听channel所有的事件发生

  int wakeupFd_;  //当mainLoop获取到一个新用户的channel，通过轮询方式选择一个subLoop，通过该标识将其唤醒
  std::unique_ptr<Channel> wakeupChannel_;

  ChannelList activeChannels_;  // evntloop中管理的channels组
  Channel* currentActiveChannel_;

  std::mutex mutex_;  //互斥锁，保护vector容器的线程安全
  std::atomic_bool callingPendingFunctors_;  //标识当前loop是否需要执行回调
  std::vector<Functor> pendingFunctors_;  //存储loop需要执行的回调操作
};

}  // namespace mymuduo