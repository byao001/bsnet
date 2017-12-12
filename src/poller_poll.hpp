//
// Created by byao on 11/9/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_POLLER_POLL_HPP
#define BSNET_POLLER_POLL_HPP

#include "event.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <poll.h>
#include <unordered_map>
#include <vector>

namespace bsnet {

using Events = std::vector<Event>;

class Poller {
public:
  using PollfdList = std::vector<struct pollfd>;
  using TokenMap = std::unordered_map<int, Token>;

  int register_event(Evented &ev, Token tok, Ready rd, PollOpt opts) {
    int fd = ev.fd();
    short event = 0;
    if (rd.is_readable())
      event |= POLLIN;
    if (rd.is_writable())
      event |= POLLOUT;

    if (opts.is_oneshot())
      return register_fd(fd, event, tok, true);

    return register_fd(fd, event, tok);
  }

  int reregister_event(Evented &ev, Token tok, Ready rd, PollOpt opts) {
    int fd = ev.fd();
    short event = 0;
    if (rd.is_readable())
      event |= POLLIN;
    if (rd.is_writable())
      event |= POLLOUT;

    if (opts.is_oneshot())
      return reregister_fd(fd, event, tok, true);

    return reregister_fd(fd, event, tok);
  }

  int deregister_event(Evented &ev) { return deregister_fd(ev.fd()); }

  template <typename Rep, typename Period>
  int poll(Events &events,
           std::chrono::duration<Rep, Period> *timeout = nullptr) {
    int tm = -1;
    if (timeout) {
      tm = static_cast<int>(
          std::chrono::duration_cast<std::chrono::milliseconds>(*timeout)
              .count());
    }
    int num_events = ::poll(_fds.data(), static_cast<int>(_fds.size()), tm);
    if (num_events == 0)
      return 0;
    for (auto it = _fds.cbegin(); it != _fds.cend() && num_events > 0; ++it) {
      if (it->revents > 0) {
        // create Ready for event.
        Ready readiness = Ready::empty();
        if (it->revents & (POLLERR | POLLNVAL))
          readiness |= Ready::error();
        if (it->revents & (POLLIN | POLLPRI | POLLHUP))
          readiness |= Ready::readable();
        if (it->revents & POLLOUT)
          readiness |= Ready::writable();

        // find token of the event
        auto tok_it = _tok_map.find(it->fd);
        assert(tok_it != _tok_map.end());
        Token tok = tok_it->second;
        events.emplace_back(readiness, tok);

        num_events -= 1;
      }
    }

    for (auto idx : _oneshots) {
      _fds[idx].events = 0;
    }

    return static_cast<int>(events.size());
  }

private:
  int register_fd(int fd, short event, Token tok, bool clear = false) {
    struct pollfd pfd {
      fd, event
    };
    _fds.push_back(pfd);
    _tok_map[fd] = tok;
    if (clear) {
      _oneshots.push_back(static_cast<int>(_fds.size() - 1));
    }
    return 0;
  }

  int reregister_fd(int fd, short event, Token tok, bool clear = false) {
    int idx;
    for (idx = 0; idx < _oneshots.size(); ++idx) {
      if (_fds[idx].fd == fd) {
        _fds[idx].events = event;
      }
    }
    if (idx >= _oneshots.size())
      return -1;

    _tok_map[fd] = tok;
    if (!clear) {
      auto it = _oneshots.begin();
      std::advance(it, idx);
      _oneshots.erase(it);
    }
    return 0;
  }
  int deregister_fd(int fd) {
    auto it = std::find_if(_fds.cbegin(), _fds.cend(),
                           [fd](auto &pfd) { return pfd.fd == fd; });
    assert(it != _fds.end());
    _fds.erase(it);
    return 0;
  }

private:
  PollfdList _fds;
  TokenMap _tok_map;
  std::vector<int> _oneshots;
};

class EventedFd : public Evented {
public:
  explicit EventedFd(int fd) : _fd(fd) {}

  int register_on(Poller &poller, Token tok, Ready interest,
                  PollOpt opts) override {
    return poller.register_event(*this, tok, interest, opts);
  }

  int reregister_on(Poller &poller, Token tok, Ready interest,
                    PollOpt opts) override {
    return poller.reregister_event(*this, tok, interest, opts);
  }

  int deregister_on(Poller &poller) override {
    return poller.deregister_event(*this);
  }

  int fd() const override { return _fd; }

private:
  int _fd;
};
} // namespace bsnet

#endif // BSNET_POLLER_POLL_HPP
