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
  
  EXPECT_EQ(query_distance(t, 1, 5), 5);
  EXPECT_EQ(query_distance(t, 1, 1), 0);
  EXPECT_EQ(query_distance(t, 5, 5), 0);
}

TEST(QUERY_TESTS, TEST3) {

  tree t = {1, 2, 3, 4, 5};
  
  EXPECT_EQ(query_distance_fast(t, 1, 5), 5);
  EXPECT_EQ(query_distance_fast(t, 1, 1), 0);
  EXPECT_EQ(query_distance_fast(t, 5, 5), 0);
}

TEST(QUERY_TESTS, TEST4) {

  tree t;

  t.insert(10);
  t.insert(20);

  EXPECT_EQ(query_distance(t, 8, 31), 2);
  EXPECT_EQ(query_distance(t, 6, 9), 0);

  t.insert(30);
  t.insert(40);

  EXPECT_EQ(query_distance(t, 15, 40), 3);
}

TEST(QUERY_TESTS, TEST5) {

  tree t;

  t.insert(10);
  t.insert(20);

  EXPECT_EQ(query_distance_fast(t, 8, 31), 2);
  EXPECT_EQ(query_distance_fast(t, 6, 9), 0);

  t.insert(30);
  t.insert(40);

  EXPECT_EQ(query_distance_fast(t, 15, 40), 3);
}

TEST(QUERY_TESTS, TEST6) {

  tree t = {1, 3, 4, 5, 6, 8, 10};

  EXPECT_EQ(query_distance(t,  5,  0), 0);
  EXPECT_EQ(query_distance(t,  0,  6), 5);
  EXPECT_EQ(query_distance(t,  7,  9), 1);
  EXPECT_EQ(query_distance(t,  6,  6), 0);
  EXPECT_EQ(query_distance(t, 10,  0), 0);
  EXPECT_EQ(query_distance(t, 10, 10), 0);
  EXPECT_EQ(query_distance(t, 10,  9), 0);
  EXPECT_EQ(query_distance(t,  6, 10), 3);
  EXPECT_EQ(query_distance(t,  5,  2), 0);
  EXPECT_EQ(query_distance(t, 10,  6), 0);
}

TEST(QUERY_TESTS, TEST7) {

  tree t = {1, 3, 4, 5, 6, 8, 10};

  EXPECT_EQ(query_distance_fast(t,  5,  0), 0);
  EXPECT_EQ(query_distance_fast(t,  0,  6), 5);
  EXPECT_EQ(query_distance_fast(t,  7,  9), 1);
  EXPECT_EQ(query_distance_fast(t,  6,  6), 0);
  EXPECT_EQ(query_distance_fast(t, 10,  0), 0);
  EXPECT_EQ(query_distance_fast(t, 10, 10), 0);
  EXPECT_EQ(query_distance_fast(t, 10,  9), 0);
  EXPECT_EQ(query_distance_fast(t,  6, 10), 3);
  EXPECT_EQ(query_distance_fast(t,  5,  2), 0);
  EXPECT_EQ(query_distance_fast(t, 10,  6), 0);
}

int main(int argc, char** argv) {

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
