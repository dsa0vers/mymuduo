#pragma once
#include <functional>
#include <memory>

#include "Timestamp.h"
#include "noncopyable.h"
/**
 * Channel:通道类，主要封装了Soketfd以及感兴趣的事件如：EPOLLIN、EPOLLOUT等，
 * 绑定了Poller返回的具体事件
 */
namespace mymuduo {

class EventLoop;
class Channel : noncopyable {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;
  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handleEvent(Timestamp receiveYTime);

  //设置回调函数
  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErroCallback(EventCallback cb) { erroCallback_ = std::move(cb); }

  //防止手动remove掉Channel之后，其回调函数还在执行
  void tie(const std::shared_ptr<void>&);

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool isNoneEvent() const { return events_ == kNoneEvent; }
  bool isWriting() const { return events_ == kWriteEvent; }
  bool isReading() const { return events_ == kReadEvent; }

  int index() { return index_; }
  void setIndex(int idx) { index_ = idx; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  void handleEventWithGuard(Timestamp receiveTime);
  void update();
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;  // 事件循环
  const int fd_;     // fd，Poller所监听的对象
  int events_;       //注册fd感兴趣的事件
  int revents_;      // Poller所返回的具体发生的事件
  int index_;        // channel相对于poller的状态码

  std::weak_ptr<void>
      tie_;  // weak_ptr，不控制所指向对象的生存周期的智能指针，指向shared_ptr所管理的对象
  bool tied_;

  // Channel能够获取监听fd所返回的具体事件revent，所以由它进行具体事件的回调操作
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback erroCallback_;
};

}  // namespace mymuduo
