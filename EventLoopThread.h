#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>

#include "Thread.h"
#include "noncopyable.h"
namespace mymuduo {

class EventLoop;

//将Event和Thread进行绑定
class EventLoopThread : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = std::string());
  ~EventLoopThread();

  EventLoop* startLoop();

 private:
  void threadFunc();

  EventLoop* loop_;
  bool exiting_;
  std::mutex mutex_;
  Thread thread_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};

}  // namespace mymuduo
