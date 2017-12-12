#ifndef BSNET_POLLER_HPP
#define BSNET_POLLER_HPP

#include "event.hpp"
#include "utility.hpp"
#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsnet {

class Poller {
public:
  friend class Registration;
  friend class TcpStream;
  friend class TcpListener;
  friend class EventedFd;

  virtual ~Poller() {}
  virtual void register_evt(Evented &ev, Token tok, Ready interest,
                            PollOpt opts) = 0;
  virtual void reregister_evt(Evented &ev, Token tok, Ready interest,
                              PollOpt opts) = 0;
  virtual void deregister_evt(Evented &ev) = 0;

  virtual int poll(std::vector<Event> &events,
                   const Duration *timeout = nullptr) = 0;

private:
  virtual int fd() const = 0;
};
}
#endif // !BSNET_POLLER_HPP
