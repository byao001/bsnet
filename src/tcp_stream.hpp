//
// Created by byao on 10/30/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_TCPSTREAM_HPP
#define BSNET_TCPSTREAM_HPP

#include "bytebuffer.hpp"
#include "event.hpp"
#include "eventedfd.hpp"
#include "utility.hpp"
#include <stdexcept>
#include <sys/socket.h>

namespace bsnet {

enum class Shutdown { Read = SHUT_RD, Write = SHUT_WR, Both = SHUT_RDWR };

class Addr;

/**
 * Nonblocking tcp stream, implemented 'Evented' interface
 * TODO:
 *   Test socket functionality[COMPLETED]
 *   add 'Evented' behavior.
 */
class TcpStream : public EventedFd {
public:
  friend class TcpListener;
  using Buf = ByteBuffer;

  /*
   * throws 'connecting_failed' exception
   */
  static TcpStream connect(const Addr &addr, Duration *ms = nullptr);
  static TcpStream connect(const char *host, const char *service);
  static TcpStream connect(const std::string &host, const std::string &service);

  TcpStream(TcpStream &&other) noexcept;
  ~TcpStream() noexcept override = default;

  void swap(TcpStream &other) noexcept {
    using std::swap;
    swap(_fd, other._fd);
  }

  /**
   * Folowing methods can throw 'tcp_error' exception
   */
  void peer_addr(Addr &addr);
  void local_addr(Addr &addr);
  void shutdown(Shutdown s);
  void set_nodelay(bool on);
  bool nodelay() const;
  void set_keepalive(bool on, int idle, int interval, int maxpkt);
  bool keepalive(int &idle, int &interval, int &maxpkt) const;

  ssize_t read(Buf &buf);
  ssize_t write(Buf &buf);

private:
  TcpStream(int fd) : EventedFd(fd) {}
  TcpStream(const TcpStream &) = delete;
  TcpStream &operator=(const TcpStream &) = delete;

  static void connect_nob(int sock, const struct sockaddr *addr, socklen_t len,
                          int timeout);
};

inline void swap(TcpStream &lhs, TcpStream &rhs) noexcept { lhs.swap(rhs); }

} // namespace bsnet

#endif // BSNET_TCPSTREAM_HPP