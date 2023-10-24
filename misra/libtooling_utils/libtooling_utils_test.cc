/*
Copyright 2022 Naive Systems Ltd.

This software contains information and intellectual property that is
confidential and proprietary to Naive Systems Ltd. and its affiliates.
*/

#include "libtooling_utils.h"

#include "gtest/gtest.h"

TEST(CleanPathTest, CleanPath) {
  EXPECT_EQ(misra::libtooling_utils::CleanPath("/src/test/../test.c"),
            "/src/test.c");
  EXPECT_EQ(misra::libtooling_utils::CleanPath("/src/test/../lib/../lib.c"),
            "/src/lib.c");
  EXPECT_EQ(misra::libtooling_utils::CleanPath("/src/test/lib/../../test.c"),
            "/src/test.c");
  EXPECT_EQ(misra::libtooling_utils::CleanPath("/src/./lib/./test.c"),
            "/src/lib/test.c");
  EXPECT_EQ(misra::libtooling_utils::CleanPath("/src/././test.c"),
            "/src/test.c");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
