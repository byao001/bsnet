//
// Created by byao on 11/7/17.
// Copyright (c) 2017 byao. All rights reserved.
//
#include "../src/event.hpp"
#include <gtest/gtest.h>

using namespace bsnet;

TEST(PollOptTest, construct) { // NOLINT
  PollOpt empty = PollOpt::empty();
  EXPECT_EQ(static_cast<uint32_t>(empty), 0);
  EXPECT_TRUE(empty.is_empty());
  PollOpt edge = PollOpt::edge();
  EXPECT_EQ(static_cast<uint32_t>(edge), EPOLLET);
  EXPECT_TRUE(edge.is_edge());
  PollOpt level = PollOpt::level();
  EXPECT_EQ(static_cast<uint32_t>(level), 0);
  EXPECT_TRUE(level.is_level());
  PollOpt oneshot = PollOpt::oneshot();
  EXPECT_EQ(static_cast<uint32_t>(oneshot), EPOLLONESHOT);
  EXPECT_TRUE(oneshot.is_oneshot());

  EXPECT_EQ(empty | edge, edge);
  EXPECT_EQ(empty | level, level);
  EXPECT_EQ(static_cast<uint32_t>(level | oneshot), EPOLLONESHOT);
  EXPECT_EQ((edge | oneshot) - edge, oneshot);

  PollOpt opt = level | oneshot;
  opt -= oneshot;
  EXPECT_EQ(opt, level);

  opt |= oneshot;
  EXPECT_EQ(opt, level | oneshot);
  EXPECT_TRUE(opt.is_level() && opt.is_oneshot());
}

TEST(ReadyTest, construct) { // NOLINT
  Ready empty = Ready::empty();
  EXPECT_TRUE(empty.is_empty());
  EXPECT_EQ(static_cast<uint32_t>(empty), 0);

  Ready readable = Ready::readable();
  EXPECT_TRUE(readable.is_readable());
  EXPECT_EQ(static_cast<uint32_t>(readable), EPOLLIN);

  Ready writable = Ready::writable();
  EXPECT_TRUE(writable.is_writable());
  EXPECT_EQ(static_cast<uint32_t>(writable), EPOLLOUT);

  Ready r = readable;
  r |= writable;
  EXPECT_TRUE(r.is_writable() && r.is_readable());
  r -= readable;
  EXPECT_TRUE(r.is_writable());
  EXPECT_FALSE(r.is_readable());
}
