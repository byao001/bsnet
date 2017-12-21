//
// Created by byao on 10/31/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_TCPLISTENER_HPP
#define BSNET_TCPLISTENER_HPP

#include "event.hpp"
#include "eventedfd.hpp"
#include "utility.hpp"
#include <cstdint>

namespace bsnet {

class Addr;
class TcpStream;

class TcpListener : public EventedFd {
public:
  static TcpListener bind(const Addr &addr, std::size_t listen_backlog);

  TcpListener(TcpListener &&other) noexcept;

  ~TcpListener() noexcept override = default;

  void swap(TcpListener &other) noexcept {
    using std::swap;
    swap(_fd, other._fd);
  }

  TcpStream accept(Addr *peer = nullptr);
  void local_addr(Addr &addr);

private:
  TcpListener(int sock) : EventedFd(sock) {}
};

inline void swap(TcpListener &lhs, TcpListener &rhs) noexcept { lhs.swap(rhs); }
}
#endif // BSNET_TCPLISTENER_HPP
