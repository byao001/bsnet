//
// Created by pandabo on 10/29/17.
//
#define BOOST_TEST_MODULE AddrTests
#include <boost/test/unit_test.hpp>

#include "address.h"

using namespace bsnet;

bool addr_equal(const Addr &lhs, const Addr &rhs) {
  return memcmp(lhs.get_sockaddr(), rhs.get_sockaddr(),
                sizeof(sockaddr_storage)) == 0;
}

bool addr_same(const Addr &lhs, const Addr &rhs) {
  return lhs.get_sockaddr() == rhs.get_sockaddr();
}

BOOST_AUTO_TEST_CASE(AddrV4_from) {
  const char *ip = "127.0.0.1";
  uint16_t port = 4000;
  AddrV4 addr(ip, port);
  AddrV4 from_addr = AddrV4::from("127.0.0.1:4000");
  BOOST_CHECK(memcmp(addr.get_sockaddr(), from_addr.get_sockaddr(),
                     sizeof(struct sockaddr_in)) == 0);
  BOOST_CHECK_THROW(AddrV4::from("127.0.0.1"), invalid_address);
}

BOOST_AUTO_TEST_CASE(AddrV6_from) {
  const char *ip = "53d:332:afe::13";
  in_port_t port = 4000;
  AddrV6 addr(ip, port);

  char buf[32];
  int s = snprintf(&buf[0], 32, "[%s]:%d", ip, port);

  AddrV6 from_addr = AddrV6::from(std::string(&buf[0], s));

  BOOST_CHECK(memcmp(addr.get_sockaddr(), from_addr.get_sockaddr(),
                     sizeof(struct sockaddr_in6)) == 0);
  BOOST_CHECK_THROW(AddrV6::from(ip), invalid_address);
}

BOOST_AUTO_TEST_CASE(Addr_copy) {

  AddrV4 move_v4 = AddrV4::from("127.0.0.1:4000");
  AddrV4 v4_copy = move_v4;

  BOOST_CHECK(addr_equal(move_v4, v4_copy));
  BOOST_CHECK(!addr_same(move_v4, v4_copy));

  AddrV6 addr_v6 = AddrV6::from("[53d:332:afe::13]:1234");
  AddrV6 v6_copy = addr_v6;

  BOOST_CHECK(addr_equal(addr_v6, v6_copy));
  BOOST_CHECK(!addr_same(addr_v6, v6_copy));
}

BOOST_AUTO_TEST_CASE(Addr_check_v) {
  Addr *v4 = new AddrV4(AddrV4::from("127.0.0.1:4000"));
  BOOST_CHECK(v4->is_ipv4());
  BOOST_CHECK(!v4->is_ipv6());

  Addr *v6 = new AddrV6(AddrV6::from("[53d:332:afe::13]:1234"));
  BOOST_CHECK(v6->is_ipv6());
  BOOST_CHECK(!v6->is_ipv4());

  delete v4;
  delete v6;
}

BOOST_AUTO_TEST_CASE(Addr_conversion) {
  AddrV4 v4 = AddrV4::from("120.1.34.1:4000");
  AddrV6 v6 = AddrV6::from("[a1:11:128::1]:2445");
  Addr *addr4 = &v4;
  Addr *addr6 = &v6;

  AddrV4 nv4 = addr4->as_ipv4();
  AddrV6 nv6 = addr6->as_ipv6();
  BOOST_CHECK(addr_same(addr4->as_ipv4(), v4));
  BOOST_CHECK(addr_same(addr6->as_ipv6(), v6));
  BOOST_CHECK(addr_equal(nv4, v4));
  BOOST_CHECK(addr_equal(nv6, v6));
  BOOST_CHECK_THROW(addr4->as_ipv6(), invalid_address);
  BOOST_CHECK_THROW(addr6->as_ipv4(), invalid_address);
}