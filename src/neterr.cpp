#include "neterr.hpp"

namespace bsnet {
// poller error
IMPL_ERR(create_epoll_failed);
IMPL_ERR(epoll_wait_failed);
IMPL_ERR(poller_error);

IMPL_ERR(socket_error);

// tcp stream
IMPL_ERR(connecting_failed)
IMPL_MSG_ERR(tcp_error)

// tcp listener
IMPL_ERR(creating_acceptor_failed);
IMPL_ERR(binding_error);
}