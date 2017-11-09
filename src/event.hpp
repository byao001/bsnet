//
// Created by byao on 11/7/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#pragma once
#include <cstdint>
#include <type_traits>

#ifdef __linux__
#include <sys/epoll.h>
#endif

namespace bsnet {

/**
 * PollOpt class has 4 base enum value, and can be combined by
 * operator '|', and remove value use operator '-'.
 */
class PollOpt {
  enum { Empty = 0, Edge = 1, Level = 1 << 1, Oneshot = 1 << 2 };

public:
  static PollOpt empty() { return {Empty}; }
  static PollOpt edge() { return {Edge}; }
  static PollOpt level() { return {Level}; }
  static PollOpt oneshot() { return {Oneshot}; }

  explicit operator uint8_t() const { return _value; }

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
  PollOpt(uint8_t o) : _value(o) {} // NOLINT
  uint8_t _value;
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
  enum { Empty = 0, Read = 1, Write = 1 << 1, Error = 1 << 2, Hup = 1 << 3 };

public:
  static Ready empty() { return {Empty}; }
  static Ready readable() { return {Read}; }
  static Ready writable() { return {Write}; }

  explicit operator uint8_t() const { return _value; }

  bool contains(Ready r) const { return (_value & r._value) == r._value; }
  bool is_empty() const { return _value == 0; }
  bool is_readable() const { return contains(Read); }
  bool is_writable() const { return contains(Write); }

  friend bool operator==(Ready r1, Ready r2);
  friend bool operator!=(Ready r1, Ready r2);

  Ready &operator|=(Ready r);
  friend Ready operator|(Ready r1, Ready r2);
  Ready &operator-=(Ready r);
  friend Ready operator-(Ready r1, Ready r2);

private:
  Ready(uint8_t r) : _value(r) {} // NOLINT
  uint8_t _value;
};

inline bool operator==(Ready r1, Ready r2) {
  return r1._value == r2._value;
}
inline bool operator!=(Ready r1, Ready r2) {
  return !(r1 == r2);
}
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

}
