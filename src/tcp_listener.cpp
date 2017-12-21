/*
 * @Author: Bo Yao
 * @Date: 2017-11-21 20:16:52
 * @Last Modified by: Bo Yao
 * @Last Modified time: 2017-11-23 15:53:03
 */
#include "tcp_listener.hpp"
#include "address.hpp"
#include "neterr.hpp"
#include "tcp_stream.hpp"
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

namespace bsnet {

TcpListener TcpListener::bind(const Addr &addr, size_t listen_backlog) {
  int domain = addr.is_ipv4() ? AF_INET : AF_INET6;
  int sock = ::socket(domain, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (sock == -1)
    throw creating_acceptor_failed();

  // create acceptor
  auto sockad = reinterpret_cast<const struct sockaddr *>(addr._addr);
  if (::bind(sock, sockad, addr.size()) < 0) {
    // close the socket if binding failed;
    ::close(sock);
    throw binding_error();
  }

  // start listening
  if (::listen(sock, listen_backlog) < 0) {
    ::close(sock);
    throw creating_acceptor_failed();
  }

  return TcpListener(sock);
}

TcpListener::TcpListener(TcpListener &&other) noexcept : EventedFd(-1) {
  this->swap(other);
}

TcpStream TcpListener::accept(Addr *addr) {
  int sock;
  socklen_t socklen, *socklenp = nullptr;
  struct sockaddr *ad = nullptr;
  if (addr) {
    ad = addr->get_sockaddr();
    socklenp = &socklen;
  }
  CHECKED((sock = ::accept4(_fd, ad, socklenp, SOCK_NONBLOCK)) > 0 ||
              errno == EAGAIN || errno == EWOULDBLOCK,
          creating_acceptor_failed);
  if (socklenp) {
    addr->_v = socklen == sizeof(struct sockaddr_in) ? Addr::Version::V4
                                                     : Addr::Version::V6;
  }
  return TcpStream(sock);
}

void TcpListener::local_addr(Addr &addr) {
  socklen_t socklen;
  CHECKED_TCPOP(::getsockname(_fd, addr.get_sockaddr(), &socklen) != -1);
}
}