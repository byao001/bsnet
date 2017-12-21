#ifndef BSNET_POLLER_EPOLL_HPP
#define BSNET_POLLER_EPOLL_HPP

#include "blocking_queue.hpp"
#include "event.hpp"
#include "utility.hpp"
#include <cassert>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <vector>

namespace bsnet {

class ReadinessQueue {
public:
  ReadinessQueue(std::size_t size, int notify);

  template <typename... Args> void emplace(Args &&... args) {
    static int64_t buf = 1;
    _rq.emplace(std::forward<Args>(args)...);
    int n = ::write(_notify, &buf, sizeof(buf));
    assert(n == sizeof(buf));
  }

  void put(const Event &evt);
  std::size_t get_all(std::vector<Event> &res);
  std::size_t size() const { return _rq.size(); }

private:
  bounded_blocking_queue_t<Event> _rq;
  int _notify;
};

class Poller {
public:
  friend class Registration;
  friend class TcpStream;
  friend class TcpListener;
  friend class EventedFd;

  static Guard<Poller> new_instance();
  Poller(Poller &&) noexcept;
  ~Poller();

  void swap(Poller &other) noexcept;
  void register_evt(Evented &ev, Token tok, Ready interest, PollOpt opts);
  void reregister_evt(Evented &ev, Token tok, Ready interest, PollOpt opts);
  void deregister_evt(Evented &ev);

  int poll(std::vector<Event> &events, const Duration *timeout = nullptr);
  int user_poll(std::vector<Event> &events);

private:
  Poller();
  int fd() const { return _epfd; }
  ReadinessQueue *rq() { return _rq; }

  int _epfd;
  int _rq_notify;
  ReadinessQueue *_rq;
};
}

#endif // !BSNET_POLLER_EPOLL_HPP
