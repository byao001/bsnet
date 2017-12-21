#include "poller_epoll.hpp"
#include "neterr.hpp"
#include <cassert>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace std;

namespace bsnet {

ReadinessQueue::ReadinessQueue(size_t size, int notify)
    : _rq(size), _notify(notify) {}

void ReadinessQueue::put(const Event &evt) {
  static int64_t buf = 1;
  _rq.put(evt);
  ::write(_notify, &buf, sizeof(buf));
}

std::size_t ReadinessQueue::get_all(std::vector<Event> &res) {
  return _rq.get_all(res);
}

Guard<Poller> Poller::new_instance() {
  Guard<Poller> poller(new Poller());
  return std::move(poller);
}

void Poller::swap(Poller &other) noexcept {
  using std::swap;
  swap(_epfd, other._epfd);
  swap(_rq_notify, other._rq_notify);
  swap(_rq, other._rq);
}

Poller::Poller() {
  _epfd = ::epoll_create1(EPOLL_CLOEXEC);
  if (_epfd == -1)
    throw create_epoll_failed();

  _rq_notify = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (_rq_notify == -1)
    throw create_epoll_failed();

  _rq = new ReadinessQueue(1024, _rq_notify);

  struct epoll_event evt;
  evt.events = EPOLLIN | EPOLLET;
  evt.data.u64 = 0;
  if (-1 == ::epoll_ctl(_epfd, EPOLL_CTL_ADD, _rq_notify, &evt))
    throw create_epoll_failed();
}

Poller::Poller(Poller &&other) noexcept
    : _epfd(-1), _rq_notify(-1), _rq(nullptr) {
  this->swap(other);
}

Poller::~Poller() {
  if (_epfd > 0) {
    ::close(_epfd);
  }
  if (_rq_notify > 0) {
    ::close(_rq_notify);
  }
  delete _rq;
}

void Poller::register_evt(Evented &ev, Token tok, Ready interest,
                          PollOpt opts) {
  ev.register_on(*this, tok, interest, opts);
}

void Poller::reregister_evt(Evented &ev, Token tok, Ready interest,
                            PollOpt opts) {
  ev.reregister_on(*this, tok, interest, opts);
}

void Poller::deregister_evt(Evented &ev) { ev.deregister_on(*this); }

int Poller::poll(vector<Event> &events, const Duration *timeout) {
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

int Poller::user_poll(vector<Event> &events) {
  assert(_rq->size() > 0);
  _rq->get_all(events);
  int64_t v;
  ::read(_rq_notify, &v, sizeof(v));
}
}
