#include "Channel.h"

#include <sys/epoll.h>

#include "EventLoop.h"
#include "Logger.h"

using namespace mymuduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {}

Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

/**
 *  向Channel所属的EventLoop更改Poller中对应事件epoll_ctl
 */
void Channel::update() { loop_->updateChannel(this); }

void Channel::remove() { loop_->removeChannel(this); }

void Channel::handleEvent(Timestamp receiveYTime) {
  if (tied_) {
    std::shared_ptr<void> guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveYTime);
    }
  } else {
    handleEventWithGuard(receiveYTime);
  }
}

//根据Poller通知的Cahnnel具体事件，由Channel进行相应的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime) {
  LOG_INFO("Channel handleEvent revents: %d\n", revents_);
  if (events_ & EPOLLHUP || !(events_ & EPOLLIN)) {
    if (closeCallback_) {
      closeCallback_();
    }
  }
  if (events_ & EPOLLERR) {
    if (erroCallback_) {
      erroCallback_();
    }
  }
  if (events_ & (EPOLLIN | EPOLLPRI)) {
    if (readCallback_) {
      readCallback_(receiveTime);
    }
  }
  if (events_ & EPOLLOUT) {
    if (writeCallback_) {
      writeCallback_();
    }
  }
}