#include "poller_epoll.hpp"
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

using namespace std;

namespace bsnet {

Epoller Epoller::new_instance() {
  Epoller epoller;
  return epoller;
}

Epoller::Epoller() {
  _epfd = ::epoll_create1(EPOLL_CLOEXEC);
  if (_epfd == -1)
    throw create_epoll_failed();
}

Epoller::Epoller(Epoller &&other) noexcept : _epfd(other._epfd) {
  other._epfd = -1;
}

Epoller::~Epoller() {
  if (_epfd > 0) {
    ::close(_epfd);
  }
}

int Epoller::register_evt(Evented &ev, Token tok, Ready interest,
                          PollOpt opts) {
  return ev.register_on(*this, tok, interest, opts);
}

int Epoller::reregister_evt(Evented &ev, Token tok, Ready interest,
                            PollOpt opts) {
  return ev.reregister_on(*this, tok, interest, opts);
}

int Epoller::deregister_evt(Evented &ev) { return ev.deregister_on(*this); }

int Epoller::poll(vector<Event> &events, Duration *timeout) {
  static_assert(sizeof(Event) == sizeof(epoll_event),
                "Event and epoll_event not match");
  int tm = timeout ? static_cast<int>(timeout->count()) : -1;
  while (true) {
    int num_evt = ::epoll_wait(
        _epfd, reinterpret_cast<struct epoll_event *>(events.data()),
        events.size(), tm);

    if (num_evt == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        throw epoll_wait_failed();
      }
    } else
      return num_evt;
  }
}
}
