/*
 * @Author: Bo Yao
 * @Date: 2017-11-23 15:10:57
 * @Last Modified by: Bo Yao
 * @Last Modified time: 2017-11-23 16:01:39
 */
#ifndef BSNET_EVENTEDFD_HPP
#define BSNET_EVENTEDFD_HPP

#include "event.hpp"

namespace bsnet {

class EventedFd : public Evented {
public:
  EventedFd(int fd) : _fd(fd) {}

  void register_on(Poller &poller, Token tok, Ready interest,
                   PollOpt opts) override;

  void reregister_on(Poller &poller, Token tok, Ready interest,
                     PollOpt opts) override;

  void deregister_on(Poller &poller) override;

  int fd() const override { return _fd; }
  ~EventedFd() noexcept override;

  void swap(EventedFd &other) noexcept;

private:
  int _fd;
};

inline void swap(EventedFd &lhs, EventedFd &rhs) noexcept { lhs.swap(rhs); }
}

#endif // !BSNET_EVENTEDFD_HPP