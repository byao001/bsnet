#pragma once

#include "NonCopyable.hpp"
#include "poller.hpp"
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace bsnet {

class create_epoll_failed : public std::runtime_error {
public:
  create_epoll_failed() : std::runtime_error(strerror(errno)) {}
};

class epoll_wait_failed : public std::runtime_error {
public:
  epoll_wait_failed() : std::runtime_error(strerror(errno)) {}
};

class Epoller : public Poller {
public:
  friend class Registration;
  friend class TcpStream;
  friend class TcpListener;
  friend class EventedFd;

  static Epoller new_instance();
  Epoller();
  Epoller(Epoller &&) noexcept;
  ~Epoller();

  int register_evt(Evented &ev, Token tok, Ready interest, PollOpt opts);
  int reregister_evt(Evented &ev, Token tok, Ready interest, PollOpt opts);
  int deregister_evt(Evented &ev);

  int poll(std::vector<Event> &events, Duration *timeout = nullptr);

private:
  int _epfd;
};
}
