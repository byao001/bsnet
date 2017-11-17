#pragma once
#include "event.hpp"
#include <chrono>
#include <vector>

namespace bsnet {

class Poller {
public:
  using Duration = std::chrono::milliseconds;

  virtual int register_evt(Evented &ev, Token tok, Ready interest,
                           PollOpt opts) = 0;
  virtual int reregister_evt(Evented &ev, Token tok, Ready interest,
                             PollOpt opts) = 0;
  virtual int deregister_evt(Evented &ev) = 0;

  virtual int poll(std::vector<Event> &events, Duration *timeout) = 0;
};
}