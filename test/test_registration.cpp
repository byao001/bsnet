//
// Created by byao on 12/20/17.
// Copyright (c) 2017 byao. All rights reserved.
//
#include "../src/poller_epoll.hpp"
#include "../src/registration.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <queue>
#include <random>
#include <thread>

using namespace std;
using namespace bsnet;

struct Deadline : public Evented {
  Deadline(chrono::time_point<chrono::system_clock> t) : reg() {
    SetReadiness sr = reg.new_set_readiness();
    thread th(
        [this, t](SetReadiness &&s) {
          this_thread::sleep_until(t);
          s.set_readiness(Ready::readable());
        },
        std::move(sr));
    th.detach();
  }

  void register_on(Poller &poller, Token tok, Ready interest,
                   PollOpt opts) override {
    reg.register_on(poller, tok, interest, opts);
  }

  void reregister_on(Poller &poller, Token tok, Ready interest,
                     PollOpt opts) override {
    reg.reregister_on(poller, tok, interest, opts);
  }

  void deregister_on(Poller &poller) override { reg.deregister_on(poller); }

  ~Deadline() noexcept override = default;

private:
  std::chrono::time_point<std::chrono::system_clock> when;
  Registration reg;
};

TEST(TestRegistration, registration) {
  using namespace std::chrono_literals;
  auto poller = Poller::new_instance();
  auto three_sec_later = chrono::system_clock::now() + 3s;

  Deadline deadline(three_sec_later);
  poller->register_evt(deadline, Token(3), Ready::readable(), PollOpt::empty());

  auto start = chrono::system_clock::now();
  vector<Event> events(10);
  int n = poller->poll(events);
  EXPECT_TRUE(n > 0);
  for (int i = 0; i < n; ++i) {
    if (events[i].token() == Token(0)) {
      int u = poller->user_poll(events);
      EXPECT_TRUE(u > 0);
      events[i] = events.back();
      events.pop_back();
      i--;
      continue;
    } else {
      EXPECT_EQ(events[i].token(), Token(3));
      EXPECT_TRUE(events[i].readiness().contains(Ready::readable()));
      auto dur = chrono::duration_cast<chrono::milliseconds>(
                     chrono::system_clock::now() - start)
                     .count();
      EXPECT_TRUE(dur < 3100) << "duration: " << dur;
    }
  }
}
