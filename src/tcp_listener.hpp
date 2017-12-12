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

class TcpListener : public Evented {
public:
  static TcpListener bind(const Addr &addr, std::size_t listen_backlog);

  TcpListener(TcpListener &&other) noexcept;

  ~TcpListener() noexcept override = default;

  void swap(TcpListener &other) noexcept { _acceptor.swap(other._acceptor); }

  TcpStream accept(Addr *peer = nullptr);
  void local_addr(Addr &addr);

  /**
   * The following methods are from interface 'Evented'
   */
  void register_on(Poller &poller, Token tok, Ready interest,
                   PollOpt opts) override {
    _acceptor.register_on(poller, tok, interest, opts);
  }
  void reregister_on(Poller &poller, Token tok, Ready interest,
                     PollOpt opts) override {
    _acceptor.reregister_on(poller, tok, interest, opts);
  }
  void deregister_on(Poller &poller) override {
    _acceptor.deregister_on(poller);
  }
  int fd() const override { return _acceptor.fd(); }

private:
  TcpListener(int sock) : _acceptor(sock) {}
  EventedFd _acceptor;
};

inline void swap(TcpListener &lhs, TcpListener &rhs) noexcept { lhs.swap(rhs); }
}
#endif // BSNET_TCPLISTENER_HPP
