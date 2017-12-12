//
// Created by byao on 11/7/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_EVENT_HPP
#define BSNET_EVENT_HPP

#include <cstdint>
#include <sys/epoll.h>
#include <utility>

namespace bsnet {

using Token = std::uint64_t;

/**
 * PollOpt class has 4 base enum value, and can be combined by
 * operator '|', and remove value use operator '-'.
 */
class PollOpt {
#ifdef __linux__
  enum { Empty = 0, Level = 0, Edge = EPOLLET, Oneshot = EPOLLONESHOT };
#endif

public:
  static PollOpt empty() { return {Empty}; }
  static PollOpt edge() { return {Edge}; }
  static PollOpt level() { return {Level}; }
  static PollOpt oneshot() { return {Oneshot}; }

  explicit operator std::uint32_t() const { return _value; }

  PollOpt &operator|=(PollOpt opt);
  friend PollOpt operator|(PollOpt opt1, PollOpt opt2);
  PollOpt &operator-=(PollOpt opt);
  friend PollOpt operator-(PollOpt opt1, PollOpt opt2);

  friend bool operator==(PollOpt opt1, PollOpt opt2);
  friend bool operator!=(PollOpt opt1, PollOpt opt2);

  bool contains(PollOpt opt) const {
    return (_value & opt._value) == opt._value;
  }

  bool is_empty() const { return _value == 0; }
  bool is_edge() const { return contains(Edge); }
  bool is_level() const { return contains(Level); }
  bool is_oneshot() const { return contains(Oneshot); }

private:
  PollOpt(std::uint32_t o) : _value(o) {} // NOLINT
  std::uint32_t _value;
};

inline PollOpt &PollOpt::operator|=(PollOpt opt) {
  _value |= opt._value;
  return *this;
}
inline PollOpt operator|(PollOpt opt1, PollOpt opt2) {
  opt1 |= opt2;
  return opt1;
}
inline PollOpt &PollOpt::operator-=(PollOpt opt) {
  _value &= (~opt._value);
  return *this;
}
inline PollOpt operator-(PollOpt opt1, PollOpt opt2) {
  opt1 -= opt2;
  return opt1;
}
inline bool operator==(PollOpt opt1, PollOpt opt2) {
  return opt1._value == opt2._value;
}
inline bool operator!=(PollOpt opt1, PollOpt opt2) {
  return opt1._value != opt2._value;
}

/**
 * Ready class has 4 base enum value, and can be combined by
 * operator '|', and remove value use operator '-'.
 */
class Ready {
#ifdef __linux__
  enum {
    Empty = 0,
    Read = EPOLLIN,
    Write = EPOLLOUT,
    Error = EPOLLERR,
    Hup = EPOLLHUP,
    RdHup = EPOLLRDHUP
  };
#endif
public:
  friend class Poller;
  friend class Event;

  static Ready empty() { return {Empty}; }
  static Ready readable() { return {Read}; }
  static Ready writable() { return {Write}; }
  static Ready error() { return {Error}; }
  static Ready hup() { return {Hup}; }

  explicit operator uint32_t() const { return _value; }

  bool contains(Ready r) const { return (_value & r._value) == r._value; }
  bool is_empty() const { return _value == 0; }
  bool is_readable() const { return contains(Read); }
  bool is_writable() const { return contains(Write); }
  bool is_error() const { return contains(Error); }
  bool is_hup() const { return contains(Hup); }

  friend bool operator==(Ready r1, Ready r2);
  friend bool operator!=(Ready r1, Ready r2);

  Ready &operator|=(Ready r);
  friend Ready operator|(Ready r1, Ready r2);
  Ready &operator-=(Ready r);
  friend Ready operator-(Ready r1, Ready r2);

private:
  Ready(uint32_t r) : _value(r) {} // NOLINT
  uint32_t _value;
};

inline bool operator==(Ready r1, Ready r2) { return r1._value == r2._value; }
inline bool operator!=(Ready r1, Ready r2) { return !(r1 == r2); }
inline Ready &Ready::operator|=(Ready r) {
  _value |= r._value;
  return *this;
}
inline Ready operator|(Ready r1, Ready r2) {
  r1 |= r2;
  return r1;
}
inline Ready &Ready::operator-=(Ready r) {
  _value &= (~r._value);
  return *this;
}
inline Ready operator-(Ready r1, Ready r2) {
  r1 -= r2;
  return r1;
}

class Event {
public:
  Event() = default;
  Event(Ready rd, Token tok) {
    _ev.events = static_cast<std::uint32_t>(rd);
    _ev.data.u64 = tok;
  }

  Ready readiness() const { return Ready(_ev.events); }
  Token token() const { return _ev.data.u64; }

private:
  struct epoll_event _ev;
};

class Poller;

class Evented {
public:
  virtual void register_on(Poller &poller, Token tok, Ready interest,
                           PollOpt opts) = 0;
  virtual void reregister_on(Poller &poller, Token tok, Ready interest,
                             PollOpt opts) = 0;
  virtual void deregister_on(Poller &poller) = 0;
  virtual int fd() const = 0;
  virtual ~Evented() noexcept {}
};
} // namespace bsnet

#endif // !BSNET_EVENT_HPP
