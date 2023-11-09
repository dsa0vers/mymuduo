#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <string>

#include "noncopyable.h"

namespace mymuduo {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
  ~EventLoopThreadPool();

  void setThreadNum(int numThread) { numThread_ = numThread; }

  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  //在多线程中，主线程（baseLoop）以轮询的方式将channel分配给subReactor(subLoop)
  EventLoop* getNextLoop();
  std::vector<EventLoop*> getAllLoops();

  bool staeted() const { return started_; }
  const std::string& name() const { return name_; }

 private:
  EventLoop* baseLoop_;
  std::string name_;
  bool started_;
  int numThread_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> thread_;
  std::vector<EventLoop*> loops_;
};
}  // namespace mymuduo