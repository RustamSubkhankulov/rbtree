#include <gtest/gtest.h>
#include <iostream>
#include <iterator>

#include "rbtree.hpp"
#include "query.hpp"

using namespace RBTREE;
using tree = rbtree<int>;

TEST(QUERY_TESTS, TEST1) {

  tree t = {1, 3, 5};
  query_insert(t, 4);
  query_insert(t, 2);

  auto ilist = {1, 2, 3, 4, 5};
  EXPECT_EQ(t, ilist);
}

TEST(QUERY_TESTS, TEST2) {

  tree t = {1, 2, 3, 4, 5};
  
  EXPECT_EQ(query_distance(t, 1, 5), 4);
  EXPECT_EQ(query_distance_fast(t, 1, 5), 4);

  EXPECT_EQ(query_distance(t, 1, 1), 0);
  EXPECT_EQ(query_distance_fast(t, 1, 1), 0);
  EXPECT_EQ(query_distance(t, 5, 5), 0);
  EXPECT_EQ(query_distance_fast(t, 5, 5), 0);
}

int main(int argc, char** argv) {

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
