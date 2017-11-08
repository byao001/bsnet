//
// Created by byao on 11/7/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#include "gtest/gtest.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}