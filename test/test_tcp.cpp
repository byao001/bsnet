#include "../src/address.hpp"
#include "../src/bytebuffer.hpp"
#include "../src/poller_epoll.hpp"
#include "../src/tcp_listener.hpp"
#include "../src/tcp_stream.hpp"
#include "gtest/gtest.h"
#include <cctype>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace bsnet;

string upper_str(const string &s) {
  string res;
  for (auto c : s) {
    res.append(1, std::toupper(c));
  }
  return res;
}

struct TcpStreamTest : ::testing::Test {
  using Buffer = TcpStream::Buf;

  TcpStreamTest()
      : host("127.0.0.1"), service("8081"),
        server(TcpListener::bind(AddrV4::from(host + ":" + service), 128)),
        poller(Poller::new_instance()) {}

  ~TcpStreamTest() override {}

  void SetUp() override {
    rbuf.clear();
    wbuf.clear();
    crbuf.clear();
    cwbuf.clear();

    thread t([&]() {
      vector<Event> events(1);
      poller->register_evt(server, Token(0), Ready::readable(),
                           PollOpt::level());
      int n = poller->poll(events);

      TcpStream peer = server.accept();
      poller->register_evt(peer, Token(1), Ready::readable(), PollOpt::level());
      n = poller->poll(events);

      if (events[0].token() == 1 &&
          events[0].readiness() == Ready::readable()) {
        int len = peer.read(rbuf);
        fprintf(stderr, "input length: %d\n", len);
        string response = rbuf.take_string();
        fprintf(stderr, "%s\n", response.c_str());
        wbuf.put_string(upper_str(response));
        peer.write(wbuf);
      } else {
        fprintf(stderr, "Error reading inputs\n");
      }
    });
    t.detach();
  }

  void TearDown() override {}

  string host, service;
  Guard<Poller> poller;
  TcpListener server;
  Buffer rbuf;
  Buffer wbuf;
  Buffer crbuf;
  Buffer cwbuf;
};

TEST_F(TcpStreamTest, read_write) {
  using namespace std::chrono_literals;

  auto client_poller = Poller::new_instance();

  string msg = "hello, world!";
  cwbuf.put_string(msg);
  TcpStream client = TcpStream::connect(host.c_str(), service.c_str());
  // TcpStream oclient = TcpStream::connect(host.c_str(), service.c_str());

  vector<Event> events(1);
  client_poller->register_evt(client, Token(2), Ready::readable(),
                              PollOpt::edge());
  client.write(cwbuf);

  int n = client_poller->poll(events);
  EXPECT_EQ(n, 1);
  EXPECT_EQ(events[0].token(), Token(2));
  EXPECT_EQ(events[0].readiness(), Ready::readable());
  client.read(crbuf);

  string response = crbuf.take_string();
  EXPECT_EQ(response, upper_str(msg));

  client_poller->deregister_evt(client);
};