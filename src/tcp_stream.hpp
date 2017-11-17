//
// Created by byao on 10/30/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#pragma once

#include "address.hpp"
#include "buffer.hpp"
#include <cstring>
#include <stdexcept>

namespace bsnet {

struct connecting_failed : std::runtime_error {
  connecting_failed() : std::runtime_error(::strerror(errno)) {}
};

enum class Shutdown { Read = SHUT_RD, Write = SHUT_WR, Both = SHUT_RDWR };

/**
 * Nonblocking tcp stream.
 */
class TcpStream {
public:
  using Buf = Buffer<>;

  static TcpStream connect(const Addr &addr);

  TcpStream(TcpStream &&other);
  ~TcpStream();

  Addr peer_addr();
  Addr local_addr();
  void shutdown(Shutdown s);
  void set_nodelay(bool on);
  bool nodelay() const;
  void set_keepalive(bool on);
  bool keepalive() const;
  void set_ttl(std::uint32_t ms);
  std::uint32_t ttl() const;

  std::size_t read(Buf &buf);
  std::size_t write(const Buf &buf);

private:
  TcpStream() = default;
  TcpStream(const TcpStream &) = delete;
  TcpStream &operator=(const TcpStream &) = delete;

  int _sock;
};
}
