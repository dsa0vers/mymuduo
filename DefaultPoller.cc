#include <cstdlib>
#include "EPollPoller.h"
#include "Poller.h"
using namespace mymuduo;
Poller* Poller::NewDefaultPoller(EventLoop* eventloop) {
  if (::getenv("MUDUO_USE_POLL")) {
    return nullptr;
  } else {
    return new EPollPoller(eventloop);;
  }
}