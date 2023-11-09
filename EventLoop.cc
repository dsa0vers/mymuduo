#include "EventLoop.h"

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "Channel.h"
#include "Logger.h"
#include "Poller.h"

using namespace mymuduo;

namespace {
__thread EventLoop* t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

int createEventfd() {
  int evtfd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  if (evtfd < 0) {
    LOG_FATAL("Failed in eventfd %d \n", errno);
  }
  return evtfd;
}
}  // namespace

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(tid()),
      poller_(Poller::NewDefaultPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)) {
  LOG_INFO("EventLoop created %p in thread %d \n", this, threadId_);
  if (t_loopInThisThread) {
    LOG_FATAL("Another EventLoop %p exists in this thread %d \n",
              t_loopInThisThread, threadId_);
  } else {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  // we are always reading the wakeupfd
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

//开启事件循环
void EventLoop::loop() {
  looping_ = true;
  quit_ = false;

  LOG_INFO("EventLoop %p start looping \n", this);

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (Channel* channel : activeChannels_) {
      channel->handleEvent(pollReturnTime_);
    }
    // 执行当前EventLoop事件循环需要处理的回调操作
    /**
     * IO线程 mainLoop accept fd《=channel subloop
     * mainLoop 事先注册一个回调cb（需要subloop来执行）    wakeup
     * subloop后，执行下面的方法，执行之前mainloop注册的cb操作
     */
    doPengingFunctors();
  }
  LOG_INFO("EventLoop %p stop looping. \n", this);
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;

  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(cb);
  }
}

// 把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb) {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    pendingFunctors_.emplace_back(cb);
  }
  // 唤醒相应的，需要执行上面回调操作的loop的线程了
  // callingPendingFunctors_的意思是：当前loop正在执行回调，但是loop又有了新的回调
  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

//唤醒线程
void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
  }
}

// EventLoop的方法 =》 Poller的方法
void EventLoop::updateChannel(Channel* channel) {
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
  return poller_->hasChannel(channel);
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeupFd_, &one, sizeof one);

  if (n != sizeof one) {
    LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8 \n", n);
  }
}

//执行回调
void EventLoop::doPengingFunctors() {
  std::vector<Functor> funtors;
  callingPendingFunctors_ = true;

  {
    std::unique_lock<std::mutex> lock(mutex_);
    funtors.swap(pendingFunctors_);
  }

  for (const Functor& functor : funtors) {
    functor();
  }

  callingPendingFunctors_ = false;
}
