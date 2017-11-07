//
// Created by afei on 9/23/16.
//

#ifndef NETWORK_ADDRESS_H
#define NETWORK_ADDRESS_H

#include <netdb.h>
#include <string>
#include <stdexcept>
#include <sys/socket.h>

namespace bsnet {

class invalid_address : public std::runtime_error {
public:
  explicit invalid_address(const std::string &);
};

class AddrV4;
class AddrV6;
class Addr;

class Addr {
public:
  Addr();
  ~Addr();
  Addr(const Addr &);
  Addr(Addr &&) noexcept;
  Addr &operator=(const Addr &);
  Addr &operator=(Addr &&) noexcept;

  void swap(Addr &other) noexcept;

  bool is_ipv4() const;
  bool is_ipv6() const;
  const AddrV4& as_ipv4() const;
  const AddrV6& as_ipv6() const;
  const sockaddr_storage *get_sockaddr() const;

protected:
  sockaddr_storage *_addr;
};

class AddrV4 final : public Addr {
public:
  explicit AddrV4(in_port_t port);
  explicit AddrV4(const char *host, in_port_t port);

  static AddrV4 from(const std::string &addr);

  friend class Addr;
};

class AddrV6 final : public Addr {
public:
  explicit AddrV6(in_port_t port);
  explicit AddrV6(const char *host, in_port_t port);

  static AddrV6 from(const std::string &addr);

  friend class Addr;
};
}

#endif // NETWORK_ADDRESS_H
