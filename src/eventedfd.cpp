/*
 * @Author: Bo Yao
 * @Date: 2017-11-23 15:16:34
 * @Last Modified by: Bo Yao
 * @Last Modified time: 2017-11-23 15:45:25
 */
#include "eventedfd.hpp"
#include "neterr.hpp"
#include "poller.hpp"
#include "utility.hpp"
#include <sys/epoll.h>
#include <unistd.h>

namespace bsnet {

void EventedFd::register_on(Poller &poller, Token tok, Ready interest,
                            PollOpt opts) {
  Event evt(interest, tok);
#ifdef __linux__
  CHECKED(::epoll_ctl(poller.fd(), EPOLL_CTL_ADD, _fd,
                      reinterpret_cast<struct epoll_event *>(&evt)) == 0,
          poller_error);
#endif
}

void EventedFd::reregister_on(Poller &poller, Token tok, Ready interest,
                              PollOpt opts) {
  Event evt(interest, tok);
#ifdef __linux__
  CHECKED(::epoll_ctl(poller.fd(), EPOLL_CTL_MOD, _fd,
                      reinterpret_cast<struct epoll_event *>(&evt)) == 0,
          poller_error);
#endif
}

void EventedFd::deregister_on(Poller &poller) {
#ifdef __linux__
  CHECKED(::epoll_ctl(poller.fd(), EPOLL_CTL_DEL, _fd, nullptr) == 0,
          poller_error);
#endif
}

EventedFd::~EventedFd() {
  if (_fd > 0)
    ::close(_fd);
}

void EventedFd::swap(EventedFd &other) noexcept {
  using std::swap;
  swap(_fd, other._fd);
}
}