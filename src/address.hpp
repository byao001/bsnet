//
// Created by afei on 9/23/16.
//
// Created by byao on 10/30/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_ADDRESS_HPP
#define BSNET_ADDRESS_HPP

#include "utility.hpp"
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

namespace bsnet {

class AddrV4;
class AddrV6;
class Addr;

class AddrV4 final {
public:
  friend class Addr;

  AddrV4() = default;
  explicit AddrV4(in_port_t port);
  explicit AddrV4(const char *host, in_port_t port);

  constexpr bool is_ipv4() const { return true; }
  constexpr bool is_ipv6() const { return false; }
  constexpr std::size_t size() const { return sizeof(struct sockaddr_in); }
  const struct sockaddr *get_sockaddr() const {
    return reinterpret_cast<const struct sockaddr *>(&_addr);
  }
  struct sockaddr *get_sockaddr() {
    return reinterpret_cast<struct sockaddr *>(&_addr);
  }

  static AddrV4 from(const std::string &addr);

private:
  struct sockaddr_in _addr;
};

class AddrV6 final {
public:
  friend class Addr;
  AddrV6() = default;
  explicit AddrV6(in_port_t port);
  explicit AddrV6(const char *host, in_port_t port);

  constexpr bool is_ipv4() const { return false; }
  constexpr bool is_ipv6() const { return true; }
  constexpr std::size_t size() const { return sizeof(struct sockaddr_in6); }
  const struct sockaddr *get_sockaddr() const {
    return reinterpret_cast<const struct sockaddr *>(&_addr);
  }
  struct sockaddr *get_sockaddr() {
    return reinterpret_cast<struct sockaddr *>(&_addr);
  }

  static AddrV6 from(const std::string &addr);

private:
  struct sockaddr_in6 _addr;
};

class Addr {
public:
  friend class TcpStream;
  friend class TcpListener;

  Addr();
  ~Addr();
  Addr(const Addr &);
  Addr(Addr &&) noexcept;
  Addr &operator=(const Addr &);
  Addr &operator=(Addr &&) noexcept;

  Addr(const AddrV4 &);
  Addr(const AddrV6 &);
  Addr &operator=(const AddrV4 &);
  Addr &operator=(const AddrV6 &);

  void swap(Addr &other) noexcept;

  bool is_ipv4() const { return _v == Version::V4; }
  bool is_ipv6() const { return _v == Version::V6; }
  std::size_t size() const;

  const AddrV4 *as_ipv4() const;
  const AddrV6 *as_ipv6() const;
  const struct sockaddr *get_sockaddr() const;
  struct sockaddr *get_sockaddr();

  enum struct Version : sa_family_t {
    V4 = AF_INET,
    V6 = AF_INET6,
    UnSpec = AF_UNSPEC,
  };

  typedef union {
    AddrV4 v4;
    AddrV6 v6;
  } _Addr;

private:
  template <typename Func> auto pick(Func &&f) const {
    return is_ipv4() ? f(_addr->v4) : f(_addr->v6);
  }

  Version _v;
  _Addr *_addr;
};

} // namespace bsnet

#endif // BSNET_ADDRESS_HPP
