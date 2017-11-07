//
// Created by pandabo on 10/29/17.
//
#define BOOST_TEST_MODULE AddrTest
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

BOOST_AUTO_TEST_SUITE(test_addr) // NOLINT

BOOST_AUTO_TEST_CASE(test_V4_from) { // NOLINT
  const char *ip = "127.0.0.1";
  uint16_t port = 4000;
  AddrV4 addr(ip, port);
  AddrV4 from_addr = AddrV4::from("127.0.0.1:4000");
  BOOST_CHECK(memcmp(addr.get_sockaddr(), from_addr.get_sockaddr(),
                     sizeof(struct sockaddr_in)) == 0);
  BOOST_CHECK_THROW(AddrV4::from("127.0.0.1"), invalid_address);
}

BOOST_AUTO_TEST_CASE(test_V6_from) { // NOLINT
  const char *ip = "53d:332:afe::13";
  uint16_t port = 4000;
  AddrV6 addr(ip, port);

  char buf[32];
  int s = snprintf(&buf[0], 32, "[%s]:%d", ip, port);

  std::string addrs(&buf[0], static_cast<unsigned long>(s));

  AddrV6 from_addr = AddrV6::from(addrs);

  auto a1 = reinterpret_cast<const struct sockaddr_in6*>(addr.get_sockaddr());
  auto a2 = reinterpret_cast<const struct sockaddr_in6*>(from_addr.get_sockaddr());
  BOOST_CHECK_EQUAL(a1->sin6_family, a2->sin6_family);
  BOOST_CHECK_EQUAL(a1->sin6_port, a2->sin6_port);
  BOOST_CHECK_EQUAL(a1->sin6_flowinfo,a2->sin6_flowinfo);
  BOOST_CHECK_EQUAL(a1->sin6_scope_id,a2->sin6_scope_id);
  BOOST_CHECK_EQUAL(memcmp(&a1->sin6_addr, &a2->sin6_addr, sizeof(struct in6_addr)), 0);
  
  BOOST_CHECK(memcmp(addr.get_sockaddr(), from_addr.get_sockaddr(),
                     sizeof(struct sockaddr_in6)) == 0);
  BOOST_CHECK_THROW(AddrV6::from(ip), invalid_address);
}

BOOST_AUTO_TEST_CASE(test_copy) { // NOLINT

  AddrV4 move_v4 = AddrV4::from("127.0.0.1:4000");
  AddrV4 v4_copy = move_v4; // NOLINT

  BOOST_CHECK(addr_equal(move_v4, v4_copy));
  BOOST_CHECK(!addr_same(move_v4, v4_copy));

  AddrV6 addr_v6 = AddrV6::from("[53d:332:afe::13]:1234");
  AddrV6 v6_copy = addr_v6; // NOLINT

  BOOST_CHECK(addr_equal(addr_v6, v6_copy));
  BOOST_CHECK(!addr_same(addr_v6, v6_copy));
}

BOOST_AUTO_TEST_CASE(test_check_v) { // NOLINT
  Addr *v4 = new AddrV4(AddrV4::from("127.0.0.1:4000"));
  BOOST_CHECK(v4->is_ipv4());
  BOOST_CHECK(!v4->is_ipv6());

  Addr *v6 = new AddrV6(AddrV6::from("[53d:332:afe::13]:1234"));
  BOOST_CHECK(v6->is_ipv6());
  BOOST_CHECK(!v6->is_ipv4());

  delete v4;
  delete v6;
}

BOOST_AUTO_TEST_CASE(test_conversion) { // NOLINT
  AddrV4 v4 = AddrV4::from("120.1.34.1:4000");
  AddrV6 v6 = AddrV6::from("[a1:11:128::1]:2445");
  Addr *addr4 = &v4;
  Addr *addr6 = &v6;

  const AddrV4 &nv4 = addr4->as_ipv4();
  const AddrV6 &nv6 = addr6->as_ipv6();
  BOOST_CHECK(addr_same(addr4->as_ipv4(), v4));
  BOOST_CHECK(addr_same(addr6->as_ipv6(), v6));
  BOOST_CHECK(addr_same(nv4, v4));
  BOOST_CHECK(addr_same(nv6, v6));
  BOOST_CHECK_THROW(addr4->as_ipv6(), invalid_address);
  BOOST_CHECK_THROW(addr6->as_ipv4(), invalid_address);
}

BOOST_AUTO_TEST_SUITE_END() // NOLINT
