//
// Created by afei on 9/23/16.
//
#include "address.h"
#include <arpa/inet.h>

namespace bsnet {

invalid_address::invalid_address(const std::string &s) : runtime_error(s) {}

/*
 * Constructor:
 *    not specify ip address, use wildcard defaultly.
 */
AddrV4::AddrV4(in_port_t port) {
  auto addr = reinterpret_cast<struct sockaddr_in *>(_addr);
  bzero(addr, sizeof(*_addr));
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr =
      static_cast<decltype(addr->sin_addr.s_addr)>(INADDR_ANY);
  addr->sin_port = htons(port);
}

/*
 * Constructor:
 *    specifying ip address and port.
 */
AddrV4::AddrV4(const char *host, in_port_t port) {
  auto addr = reinterpret_cast<struct sockaddr_in *>(_addr);
  bzero(addr, sizeof(*_addr));
  addr->sin_family = AF_INET;
  ::inet_pton(AF_INET, host, &addr->sin_addr);
  addr->sin_port = htons(port);
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
  auto addr = reinterpret_cast<struct sockaddr_in6 *>(_addr);
  bzero(addr, sizeof(*_addr));
  addr->sin6_family = AF_INET6;
  addr->sin6_addr = in6addr_any;
  addr->sin6_port = htons(port);
}

/*
 * Constructor:
 *    specifying ip address and port.
 */
AddrV6::AddrV6(const char *host, u_int16_t port) {
  auto addr = reinterpret_cast<struct sockaddr_in6 *>(_addr);
  bzero(addr, sizeof(*_addr));
  addr->sin6_family = AF_INET6;
  ::inet_pton(AF_INET6, host, &addr->sin6_addr);
  addr->sin6_port = htons(port);
}

AddrV6 AddrV6::from(const std::string &addr) {
  int colon = -1;
  unsigned ip_start = 0;
  size_t ip_len = 0;
  for (int i = 0; i < addr.size(); ++i) {
    if (addr[i] == '[') {
      ip_start = static_cast<unsigned>(i + 1);
    } else if (addr[i] == ']') {
      colon = 0;
    } else if (addr[i] == ':') {
      if (colon == 0) {
        colon = i;
        break;
      } else {
        ip_len += 1;
      }
    } else if (ip_start != 0) {
      ip_len += 1;
    } else {
      throw invalid_address("not ipv6");
    }
  }
  const char *ip = addr.substr(ip_start, ip_len).c_str();
  uint16_t port = static_cast<uint16_t>(
      std::stoi(addr.substr(static_cast<unsigned>(colon + 1))));
  return AddrV6(ip, port);
}

Addr::Addr() : _addr(new struct sockaddr_storage) {}
Addr::~Addr() { delete _addr; }

void Addr::swap(Addr &other) noexcept {
  using std::swap;
  swap(_addr, other._addr);
}

/*
 * Construct from other Addr
 */
Addr::Addr(const Addr &other) : Addr() {
  memcpy(_addr, other._addr, sizeof(sockaddr_storage));
}
Addr::Addr(Addr &&other) noexcept : _addr(nullptr) {
  swap(other);
}
Addr &Addr::operator=(const Addr &other) {
  memcpy(_addr, other._addr, sizeof(sockaddr_storage));
  return *this;
}
Addr &Addr::operator=(Addr &&other) noexcept {
  swap(other);
  return *this;
}

/*
 * Convert to V4 and V6
 */
const AddrV4 &Addr::as_ipv4() const {
  if (_addr->ss_family != AF_INET) {
    throw invalid_address("not ipv4");
  }
  auto v4 = (const AddrV4 *)this;
  return *v4;
}

const AddrV6 &Addr::as_ipv6() const {
  if (_addr->ss_family != AF_INET6) {
    throw invalid_address("not ipv6");
  }
  auto v6 = (const AddrV6 *)this;
  return *v6;
}
bool Addr::is_ipv4() const { return _addr->ss_family == AF_INET; }
bool Addr::is_ipv6() const { return _addr->ss_family == AF_INET6; }
const struct sockaddr_storage *Addr::get_sockaddr() const { return _addr; }
}
