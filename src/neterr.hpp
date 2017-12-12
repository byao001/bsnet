#ifndef BSNET_NETERR_HPP
#define BSNET_NETERR_HPP
#include "utility.hpp"
#include <cstring>
#include <stdexcept>
#include <string>

namespace bsnet {
DECL_MSG_ERR(invalid_address);

// poll error
DECL_ERR(poller_error);
DECL_ERR(create_epoll_failed);
DECL_ERR(epoll_wait_failed);

DECL_ERR(socket_error);

// tcp stream
DECL_ERR(connecting_failed);
DECL_MSG_ERR(tcp_error);

// tcp listener
DECL_ERR(creating_acceptor_failed);
DECL_ERR(binding_error);
}

#endif // !BSNET_NETERR_HPP
