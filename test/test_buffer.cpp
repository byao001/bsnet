//
// Created by byao on 10/31/17.
// Copyright (c) 2017 byao. All rights reserved.
//
#include "buffer.hpp"
#include "gtest/gtest.h"
#include <queue>
#include <random>
using namespace std;
using namespace bsnet;

struct BufferFixture : ::testing::Test {
  BufferFixture() : s(100, 'a'), cap(128), default_buffer_size(65535) {}
  ~BufferFixture() override = default;

  void SetUp() override {}
  void TearDown() override {}

  std::string s;
  ringbuf_t::size_type cap;
  int default_buffer_size;
};

void fill_random(void *data, int len) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<byte_t> dis(0);

  auto p = static_cast<byte_t *>(data);
  for (int i = 0; i < len; ++i) {
    *p++ = dis(gen);
  }
}

TEST_F(BufferFixture, construct) // NOLINT
{
  ringbuf_t buf(cap);
  EXPECT_EQ(buf.capacity(), cap);
  EXPECT_EQ(buf.size(), 0);
  EXPECT_TRUE(buf.empty());
  EXPECT_FALSE(buf.full());
  EXPECT_EQ(buf.readable_bytes(), 0);
  EXPECT_EQ(buf.writable_bytes(), cap);
}

TEST_F(BufferFixture, append_retrieve) // NOLINT
{
  byte_t inputbuf[default_buffer_size];
  byte_t buf_output[default_buffer_size];
  byte_t que_output[default_buffer_size];

  ringbuf_t buf(cap);
  std::queue<byte_t> q;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);
  std::uniform_int_distribution<> len_dis(1, default_buffer_size);

  int loop = 1000;
  while (loop--) {
    EXPECT_EQ(buf.size(), q.size());

    if (dis(gen)) {
      auto len = len_dis(gen);
      fill_random(&inputbuf[0], len);

      // push data to queue.
      for (int i = 0; i < len; ++i) {
        q.push(inputbuf[i]);
      }
      // append data to buf.
      if (buf.writable_bytes() < len)
        if (buf.expand().writable_bytes() < len)
          buf.reserve(buf.size() + len);
      buf.append(&inputbuf[0], len);
    } else {
      auto len = std::min(buf.size(), len_dis(gen));
      // take data from queue.
      for (int i = 0; i < len; ++i) {
        que_output[i] = q.front();
        q.pop();
      }
      // retrieve data from buf.
      buf.retrieve(&buf_output[0], len);
      ASSERT_EQ(memcmp(&buf_output[0], &que_output[0], len), 0);
    }
  }
}
