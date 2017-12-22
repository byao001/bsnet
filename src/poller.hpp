//
// Created by byao on 12/21/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#pragma once

#ifdef __linux__
#include "poller_epoll.hpp"
#elif __APPLE__
// #include "poller_kqueue.hpp"
#elif _WIN32

#ifdef _WIN64
// win64
#else
// win32
#endif

#else
#error "Unknow platform"
#endif