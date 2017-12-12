//
// Created by byao on 10/29/17.
// Copyright (c) 2017 byao. All rights reserved.
//
#include "address.hpp"
#include "neterr.hpp"
#include "gtest/gtest.h"

using namespace bsnet;

bool addr_equal(Addr lhs, Addr rhs) {
  return lhs.size() == rhs.size() &&
         memcmp(lhs.get_sockaddr(), rhs.get_sockaddr(), lhs.size()) == 0;
}

TEST(AddrTest, test_V4_from) { // NOLINT
  const char *ip = "127.0.0.1";
  uint16_t port = 4000;
  AddrV4 addr(ip, port);
  AddrV4 from_addr = AddrV4::from("127.0.0.1:4000");
  EXPECT_TRUE(addr_equal(addr, from_addr));
  EXPECT_THROW(AddrV4::from("127.0.0.1"), invalid_address);
}

TEST(AddrTest, test_V6_from) { // NOLINT
  const char *ip = "53d:332:afe::13";
  uint16_t port = 4000;
  AddrV6 addr(ip, port);

  char buf[32];
  int s = snprintf(&buf[0], 32, "[%s]:%d", ip, port);

  std::string addrs(&buf[0], static_cast<unsigned long>(s));

  AddrV6 from_addr = AddrV6::from(addrs);

  auto a1 = reinterpret_cast<const struct sockaddr_in6 *>(addr.get_sockaddr());
  auto a2 =
      reinterpret_cast<const struct sockaddr_in6 *>(from_addr.get_sockaddr());
  EXPECT_EQ(a1->sin6_family, a2->sin6_family);
  EXPECT_EQ(a1->sin6_port, a2->sin6_port);
  EXPECT_EQ(a1->sin6_flowinfo, a2->sin6_flowinfo);
  EXPECT_EQ(a1->sin6_scope_id, a2->sin6_scope_id);
  EXPECT_EQ(memcmp(&a1->sin6_addr, &a2->sin6_addr, sizeof(struct in6_addr)), 0);

  EXPECT_EQ(memcmp(addr.get_sockaddr(), from_addr.get_sockaddr(),
                   sizeof(struct sockaddr_in6)),
            0);
  EXPECT_THROW(AddrV6::from(ip), invalid_address);
}

TEST(AddrTest, test_copy) { // NOLINT

  AddrV4 move_v4 = AddrV4::from("127.0.0.1:4000");
  AddrV4 v4_copy = move_v4; // NOLINT

  EXPECT_EQ(addr_equal(move_v4, v4_copy), true);

  AddrV6 addr_v6 = AddrV6::from("[53d:332:afe::13]:1234");
  AddrV6 v6_copy = addr_v6; // NOLINT

  EXPECT_EQ(addr_equal(addr_v6, v6_copy), true);
}

TEST(AddrTest, test_check_v) { // NOLINT
  Addr v4 = AddrV4::from("127.0.0.1:4000");
  EXPECT_EQ(v4.is_ipv4(), true);
  EXPECT_EQ(v4.is_ipv6(), false);

  Addr v6 = AddrV6::from("[53d:332:afe::13]:1234");
  EXPECT_EQ(v6.is_ipv6(), true);
  EXPECT_EQ(v6.is_ipv4(), false);
}

TEST(AddrTest, test_conversion) { // NOLINT
  AddrV4 v4 = AddrV4::from("120.1.34.1:4000");
  AddrV6 v6 = AddrV6::from("[a1:11:128::1]:2445");
  Addr addr4 = v4;
  Addr addr6 = v6;

  const AddrV4 *nv4 = addr4.as_ipv4();
  const AddrV6 *nv6 = addr6.as_ipv6();
  EXPECT_EQ(addr_equal(*nv4, v4), true);
  EXPECT_EQ(addr_equal(*nv6, v6), true);
  EXPECT_EQ(addr4.as_ipv6(), nullptr);
  EXPECT_EQ(addr6.as_ipv4(), nullptr);
}
