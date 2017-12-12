//
// Created by byao on 10/31/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#include "tcp_stream.hpp"
#include "address.hpp"
#include "neterr.hpp"
#include "poller_epoll.hpp"
#include "utility.hpp"
#include <cassert>
#include <cstring>
#include <netdb.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

namespace bsnet {

// use a nonblocking socket to connect to remote peer,
void TcpStream::connect_nob(int sock, const struct sockaddr *addr,
                            socklen_t len, int timeout) {
  int r = ::connect(sock, addr, len);
  if (r == 0)
    return;
  if (r == -1 && errno != EINPROGRESS)
    throw connecting_failed();
  struct pollfd pfd;
  pfd.fd = sock;
  pfd.events = POLLOUT;
  int n = ::poll(&pfd, 1, timeout);
  if (n == 0)
    throw connecting_failed("timeout");
  if (n < 0)
    throw connecting_failed();
  assert(n == 1);
  assert(pfd.revents & POLLOUT);
}

TcpStream TcpStream::connect(const Addr &addr, Duration *ms) {
  int domain = addr.is_ipv4() ? AF_INET : AF_INET6;
  // create an non-blocking socket
  int sock = ::socket(domain, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (sock == -1)
    throw connecting_failed();

  // when ms is not specified, ::poll will blocking.
  int timeout = -1;
  if (ms)
    timeout = static_cast<int>(ms->count());

  // connect the socket to the address
  try {
    connect_nob(sock, addr.get_sockaddr(), addr.size(), timeout);
  } catch (connecting_failed &ex) {
    ::close(sock);
    throw;
  }

  // create a TcpStream using the connected socket.
  TcpStream conn(sock);
  return conn;
}

TcpStream TcpStream::connect(const char *host, const char *service) {
  struct addrinfo hints, *result, *rp;
  std::memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int r = ::getaddrinfo(host, service, &hints, &result);
  if (r != 0)
    throw connecting_failed(::gai_strerror(r));

  int s;
  for (rp = result; rp != nullptr; rp = rp->ai_next) {
    s = ::socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK | SOCK_CLOEXEC,
                 rp->ai_protocol);
    if (s < 0)
      continue;

    try {
      connect_nob(s, rp->ai_addr, rp->ai_addrlen, -1);
    } catch (connecting_failed &ex) {
      ::close(s);
      continue;
    }

    break;
  }
  ::freeaddrinfo(result);
  if (rp == nullptr)
    throw connecting_failed("no valid address");
  return TcpStream(s);
}

TcpStream TcpStream::connect(const std::string &host,
                             const std::string &service) {
  return connect(host.c_str(), service.c_str());
}

TcpStream::TcpStream(TcpStream &&other) noexcept : _sock(-1) {
  _sock.swap(other._sock);
}

void TcpStream::peer_addr(Addr &addr) {
  socklen_t socklen;
  CHECKED_TCPOP(::getpeername(_sock.fd(), addr.get_sockaddr(), &socklen) != -1);
}

void TcpStream::local_addr(Addr &addr) {
  socklen_t socklen;
  CHECKED_TCPOP(::getsockname(_sock.fd(), addr.get_sockaddr(), &socklen) != -1);
}

void TcpStream::shutdown(Shutdown s) {
  CHECKED_TCPOP(::shutdown(_sock.fd(), static_cast<int>(s)) != -1);
}

void TcpStream::set_nodelay(bool on) {
  int optval = on ? 1 : 0;
  CHECKED_TCPOP(::setsockopt(_sock.fd(), IPPROTO_TCP, TCP_NODELAY, &optval,
                             sizeof(optval)) != -1);
}

bool TcpStream::nodelay() const {
  int on;
  socklen_t size;
  CHECKED_TCPOP(
      ::getsockopt(_sock.fd(), IPPROTO_TCP, TCP_NODELAY, &on, &size) != -1);
  return on == 1;
}

void TcpStream::set_keepalive(bool on, int idle, int interval, int maxpkt) {
  int ion = on ? 1 : 0;
  CHECKED_TCPOP(::setsockopt(_sock.fd(), SOL_SOCKET, SO_KEEPALIVE, &ion,
                             sizeof(ion)) != -1);
  CHECKED_TCPOP(::setsockopt(_sock.fd(), IPPROTO_TCP, TCP_KEEPIDLE, &idle,
                             sizeof(idle)) != -1)
  CHECKED_TCPOP(::setsockopt(_sock.fd(), IPPROTO_TCP, TCP_KEEPINTVL, &interval,
                             sizeof(interval)) != -1);
  CHECKED_TCPOP(::setsockopt(_sock.fd(), IPPROTO_TCP, TCP_KEEPCNT, &maxpkt,
                             sizeof(maxpkt)) != -1);
}

bool TcpStream::keepalive(int &idle, int &interval, int &maxpkt) const {
  int on;
  socklen_t size;
  CHECKED_TCPOP(
      ::getsockopt(_sock.fd(), SOL_SOCKET, SO_KEEPALIVE, &on, &size) != -1);
  if (on == 0)
    return false;

  // keepalive is enabled
  CHECKED_TCPOP(
      ::getsockopt(_sock.fd(), IPPROTO_TCP, TCP_KEEPIDLE, &idle, &size) != -1);
  CHECKED_TCPOP(::getsockopt(_sock.fd(), IPPROTO_TCP, TCP_KEEPINTVL, &interval,
                             &size) != -1);
  CHECKED_TCPOP(
      ::getsockopt(_sock.fd(), IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, &size) != -1);
  return true;
}

ssize_t TcpStream::read(TcpStream::Buf &buf) {
  return buf.read_from(_sock.fd());
}

ssize_t TcpStream::write(TcpStream::Buf &buf) {
  return buf.write_to(_sock.fd());
}

} // namespace bsnet