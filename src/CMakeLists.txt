add_library(libbsnet
        address.hpp
        address.cpp
        tcp_stream.hpp
        tcp_stream.cpp
        ringbuf.hpp
        bytebuffer.hpp
        bytebuffer.cpp
        eventedfd.hpp
        eventedfd.cpp
        tcp_listener.hpp
        tcp_listener.cpp
        event.hpp
        blocking_queue.hpp
        poller_epoll.hpp
        poller_epoll.cpp
        utility.hpp
        neterr.hpp
        neterr.cpp
        registration.hpp
        registration.cpp token.hpp token.cpp poller.hpp)
