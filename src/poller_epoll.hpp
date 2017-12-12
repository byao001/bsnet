#ifndef BSNET_POLLER_EPOLL_HPP
#define BSNET_POLLER_EPOLL_HPP

#include "poller.hpp"
#include "utility.hpp"
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace bsnet {

class Epoller : public Poller {
public:
  friend class Registration;
  friend class TcpStream;
  friend class TcpListener;
  friend class EventedFd;

  static Epoller *new_instance();
  Epoller();
  Epoller(Epoller &&) noexcept;
  ~Epoller() override;

  void register_evt(Evented &ev, Token tok, Ready interest,
                    PollOpt opts) override;
  void reregister_evt(Evented &ev, Token tok, Ready interest,
                      PollOpt opts) override;
  void deregister_evt(Evented &ev) override;

  int poll(std::vector<Event> &events,
           const Duration *timeout = nullptr) override;

private:
  int fd() const override { return _epfd; }

  int _epfd;
};
}

#endif // !BSNET_POLLER_EPOLL_HPP
