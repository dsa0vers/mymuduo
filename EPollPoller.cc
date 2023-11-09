#include "EPollPoller.h"

#include <errno.h>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "Channel.h"
#include "Logger.h"

using namespace mymuduo;

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}  // namespace

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_FATAL("epoll_create err: %d \n", errno);
  }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

//通过epoll_wait监听channel发生的事件，并返回
Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
  //此处使用LOG_DEBUG更为合理，目前进行测试打点
  LOG_INFO("func:%s => fd total count:%ld \n", __func__, channels_.size());

  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                               static_cast<int>(events_.size()), timeoutMs);

  int saveErrono = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0) {
    LOG_INFO("func:%s => events happened:%d \n", __func__, numEvents);
    fillActiveChannels(numEvents, activeChannels);
    if (numEvents == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (numEvents == 0) {
    LOG_DEBUG("nothing happened \n");
  } else {
    if (saveErrono != EINTR) {
      errno = saveErrono;
      LOG_ERROR("EPollPoller::poll() erro! \n");
    }
  }
  return now;
}

// channel::update => EventLoop::uodateChannel => poller::updateChannel
void EPollPoller::updateChannel(Channel* channel) {
  const int index = channel->index();
  LOG_INFO("func:%s => fd:%d events:%d index:%d \n", __FUNCTION__,
           channel->fd(), channel->events(), index);
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      int fd = channel->fd();
      channels_[fd] = channel;
    }

    channel->setIndex(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else {
    int fd = channel->fd();
    if (channel->isNoneEvent()) {
      update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else
      update(EPOLL_CTL_MOD, channel);
  }
}

//从poller中移除channel
void EPollPoller::removeChannel(Channel* channel) {
  int fd = channel->fd();
  channels_.erase(fd);

  LOG_INFO("func:%s => fd:%d  \n", __func__, channel->fd());

  int index = channel->index();
  if (index == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

//更新channel管道 epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel* channel) {
  epoll_event event;
  bzero(&event, sizeof event);
  int fd = channel->fd();
  event.events = channel->events();
  event.data.fd = fd;
  event.data.ptr = channel;

  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_ERROR("epoll_ctl delete fd:%d \n", fd);
    } else {
      LOG_FATAL("epoll_ctl add/mod fd:%d \n", fd);
    }
  }
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const {
  for (int i = 0; i < numEvents; ++i) {
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}