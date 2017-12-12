//
// Created by byao on 9/23/16.
// Copyright (c) 2017 byao. All rights reserved.
//
#include "address.hpp"
#include "neterr.hpp"
#include "utility.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>

using namespace std;

namespace bsnet {

invalid_address::invalid_address(const std::string &s) : runtime_error(s) {}

/*
 * Constructor:
 *    not specify ip address, use wildcard defaultly.
 */
AddrV4::AddrV4(in_port_t port) {
  memset(&_addr, 0, sizeof(_addr));
  _addr.sin_family = AF_INET;
  _addr.sin_addr.s_addr =
      static_cast<decltype(_addr.sin_addr.s_addr)>(INADDR_ANY);
  _addr.sin_port = htons(port);
}

/*
 * Constructor:
 *    specifying ip address and port.
 */
AddrV4::AddrV4(const char *host, in_port_t port) {
  memset(&_addr, 0, sizeof(_addr));
  _addr.sin_family = AF_INET;
  ::inet_pton(AF_INET, host, &_addr.sin_addr);
  _addr.sin_port = htons(port);
}

/*
 * Create AddrV4 from string, ex. "127.0.0.1:5000"
 */
AddrV4 AddrV4::from(const std::string &addr) {
  auto pos = addr.find(':');
  if (pos == std::string::npos)
    throw invalid_address("not ipv4");
  std::string ip = addr.substr(0, pos);
  in_port_t port = static_cast<in_port_t>(std::stoi(addr.substr(pos + 1)));
  return AddrV4(ip.c_str(), port);
}

/*
 * Constructor:
 *    not specify ip address, use wildcard defaultly.
 */
AddrV6::AddrV6(in_port_t port) {
  memset(&_addr, 0, sizeof(_addr));
  _addr.sin6_family = AF_INET6;
  _addr.sin6_addr = in6addr_any;
  _addr.sin6_port = htons(port);
}

/*
 * Constructor:
 *    specifying ip address and port.
 */
AddrV6::AddrV6(const char *host, uint16_t port) {
  memset(&_addr, 0, sizeof(_addr));
  _addr.sin6_family = AF_INET6;
  inet_pton(AF_INET6, host, &_addr.sin6_addr);
  _addr.sin6_port = htons(port);
}

AddrV6 AddrV6::from(const std::string &addr) {
  int colon = -1;
  unsigned ip_start = 0;
  size_t ip_len = 0;
  for (int i = 0; i < addr.size(); ++i) {
    if (addr[i] == '[') {
      ip_start = static_cast<unsigned>(i + 1);
    } else if (addr[i] == ']') {
      ip_len = i - ip_start;
      colon = 0;
    } else if (addr[i] == ':') {
      if (colon == 0) {
        colon = i;
        break;
      }
    }
  }
  if (ip_start == 0 || ip_len == 0 || colon <= 0)
    throw invalid_address("not valid ipv6");

  string ip = addr.substr(ip_start, ip_len);
  uint16_t port = static_cast<uint16_t>(
      std::stoi(addr.substr(static_cast<unsigned>(colon + 1))));

  AddrV6 ad(ip.c_str(), port);
  return ad;
}

/**
 * Addr
 */
Addr::Addr() : _v(Version::UnSpec), _addr(new _Addr) {}
Addr::~Addr() { delete _addr; }

Addr::Addr(const Addr &other) : _v(other._v), _addr(new _Addr) {
  _v = other._v;
  memcpy(_addr, other._addr, sizeof(_Addr));
}
Addr &Addr::operator=(const Addr &other) {
  _v = other._v;
  memcpy(_addr, other._addr, sizeof(_Addr));
  return *this;
}
Addr::Addr(Addr &&other) noexcept : _v(), _addr(nullptr) { swap(other); }
Addr &Addr::operator=(Addr &&other) noexcept {
  swap(other);
  return *this;
}

Addr::Addr(const AddrV4 &other) : _v(Version::V4), _addr(new _Addr) {
  memcpy(&(_addr->v4._addr), &other._addr, sizeof(struct sockaddr_in));
}
Addr::Addr(const AddrV6 &other) : _v(Version::V6), _addr(new _Addr) {
  memcpy(&(_addr->v6._addr), &other._addr, sizeof(struct sockaddr_in6));
}
Addr &Addr::operator=(const AddrV4 &other) {
  _v = Version::V4;
  memcpy(&(_addr->v4._addr), &other._addr, sizeof(struct sockaddr_in));
  return *this;
}
Addr &Addr::operator=(const AddrV6 &other) {
  _v = Version::V6;
  memcpy(&(_addr->v6._addr), &other._addr, sizeof(struct sockaddr_in6));
  return *this;
}

void Addr::swap(Addr &other) noexcept {
  using std::swap;
  _v = other._v;
  swap(_addr, other._addr);
}

size_t Addr::size() const {
  return pick([&](auto &_) { return _.size(); });
}

const AddrV4 *Addr::as_ipv4() const {
  if (is_ipv4())
    return &_addr->v4;
  return nullptr;
}
const AddrV6 *Addr::as_ipv6() const {
  if (is_ipv6())
    return &_addr->v6;
  return nullptr;
}

const struct sockaddr *Addr::get_sockaddr() const {
  return pick([&](auto &_) { return _.get_sockaddr(); });
}
struct sockaddr *Addr::get_sockaddr() {
  return pick([&](auto &_) { return _.get_sockaddr(); });
}

} // namespace bsnet
