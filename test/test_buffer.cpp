//
// Created by pandabo on 10/31/17.
//

#define BOOST_TEST_MODULE BufferTest
#include <boost/test/unit_test.hpp>
#include <queue>
#include <random>

#include "buffer.h"

using namespace boost::unit_test;
using namespace bsnet;


struct BufferFixture {
  BufferFixture() : s(100, 'a'), cap(128) {
    BOOST_TEST_MESSAGE("setup ring buffer fixture");
  }
  ~BufferFixture() { BOOST_TEST_MESSAGE("teardown ring buffer fixture"); }

  std::string s;
  ringbuf_t::size_type cap;
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

BOOST_FIXTURE_TEST_SUITE(test_ringbuf, BufferFixture) // NOLINT

BOOST_AUTO_TEST_CASE(construct) // NOLINT
{
  ringbuf_t buf(cap);
  BOOST_CHECK_EQUAL(buf.capacity(), cap);
  BOOST_CHECK_EQUAL(buf.size(), 0);
  BOOST_CHECK(buf.empty());
  BOOST_CHECK(!buf.full());
  BOOST_CHECK_EQUAL(buf.readable_bytes(), 0);
  BOOST_CHECK_EQUAL(buf.writable_bytes(), cap);
}

BOOST_AUTO_TEST_CASE(append_retrieve) // NOLINT
{
  byte_t inputbuf[65535];
  byte_t buf_output[65535];
  byte_t que_output[65535];

  ringbuf_t buf(cap);
  std::queue<byte_t> q;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);
  std::uniform_int_distribution<> len_dis(1, 65535);

  int loop = 1000;
  while (loop--) {
    BOOST_ASSERT(buf.size() == q.size());

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
      BOOST_ASSERT(memcmp(&buf_output[0], &que_output[0], len) == 0);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END() // NOLINT

BOOST_FIXTURE_TEST_SUITE(test_buffer, BufferFixture) // NOLINT

BOOST_AUTO_TEST_CASE(append_retrieve) // NOLINT
{
  byte_t inputbuf[65535];
  byte_t buf_output[65535];
  byte_t que_output[65535];

  Buffer<ringbuf_t> buf(cap);
  std::queue<byte_t> q;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);
  std::uniform_int_distribution<> len_dis(1, 65535);

  int loop = 1000;
  while (loop--) {
    BOOST_ASSERT(buf.size() == q.size());

    if (dis(gen)) {
      int len = len_dis(gen);
      fill_random(&inputbuf[0], len);

      // push data to queue.
      for (int i = 0; i < len; ++i) {
        q.push(inputbuf[i]);
      }
      // append data to buf.
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
      BOOST_ASSERT(memcmp(&buf_output[0], &que_output[0], len) == 0);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END() // NOLINT
