#include <gtest/gtest.h>
#include <ios>
#include <iostream>
#include <iterator>
#include <vector>

#include "rbtree.hpp"

using namespace RBTREE;
using tree = rbtree<int>;

TEST(UNIT_TESTING, EMPTY_TREE) {
  tree t;
  EXPECT_EQ(t.size(), 0);
  EXPECT_EQ(t.empty(), true);
  EXPECT_EQ(t.begin(), t.end());
  EXPECT_EQ(t.rbegin(), t.rend());
}

TEST(UNIT_TESTING, ITER_CTOR) {

  auto ilist = {1, 2, 3, 4, 5};
  tree tl = ilist;
  
  EXPECT_EQ(tl.size(), ilist.size());
  EXPECT_EQ(tl, ilist);

  std::vector<int> vec = ilist;
  tree tv(vec.begin(), vec.end());

  EXPECT_EQ(tv.size(), vec.size());
  EXPECT_EQ(tv, ilist);
}

TEST(UNIT_TESTING, ITERATORS) {

  auto ilist = {1, 2, 3, 4, 5};
  tree t = ilist;

  EXPECT_EQ(t.size(), std::distance(t.begin(), t.end()));
  EXPECT_TRUE(std::equal(t.begin(), t.end(), ilist.begin()));

  EXPECT_EQ(t.size(), std::distance(t.rbegin(), t.rend()));
  EXPECT_TRUE(std::equal(t.rbegin(), t.rend(), std::rbegin(ilist)));

  auto it1 = t.begin();
  auto it2 = std::next(std::next(it1, 1), -1);
  EXPECT_EQ(it1, it2);
}

TEST(UNIT_TESTING, COPY) {

  tree t1 = {1, 2, 3, 4, 5};
  tree t2 = t1;
  EXPECT_EQ(t1, t2);

  tree t3;
  t3 = t1;
  EXPECT_EQ(t1, t3);
}

TEST(UNIT_TESTING, MOVE) {

  auto ilist = {1, 2, 3, 4, 5};
  tree t = ilist;
  tree moved1 = std::move(t);
  
  EXPECT_EQ(moved1, ilist);

  tree moved2;
  moved2 = std::move(moved1);

  EXPECT_EQ(moved2, ilist);
}

TEST(UNIT_TESTING, INSERT) {

  tree t1;

  auto res1 = t1.insert(10);
  EXPECT_NE(res1.first, t1.end());
  EXPECT_TRUE(res1.second);

  auto res2 = t1.insert(10);
  EXPECT_EQ(res2.first, t1.end());
  EXPECT_FALSE(res2.second);
  
  EXPECT_EQ(t1.size(), 1);
  EXPECT_EQ(*t1.begin(), 10);

  t1.insert({50, 20, 40, 30});
  auto l = {10, 20, 30, 40, 50};

  EXPECT_EQ(t1.size(), 5);
  EXPECT_EQ(t1, l);

  tree t2;
  std::vector<int> vec = {1, 2, 3};
  t2.insert(vec.begin(), vec.end());
  EXPECT_TRUE(std::equal(t2.begin(), t2.end(), vec.begin()));
}

TEST(UNIT_TESTING, ERASE) {

  tree t = {10, 20, 30, 40, 50};

  t.erase(std::next(t.begin(), 2));
  t.erase(std::next(t.begin()));

  auto l = {10, 40, 50};
  EXPECT_TRUE(std::equal(t.begin(), t.end(), l.begin()));

  t.erase(t.begin(), t.end());
  EXPECT_TRUE(t.empty());

  t.insert({60, 70, 80});
  EXPECT_TRUE(t.erase(60));
  EXPECT_FALSE(t.erase(65));
}

TEST(UNIT_TESTING, SWAP) {

  auto ilist1 = {1, 2, 3, 4, 5}; 
  tree t1 = ilist1;

  auto ilist2 = {1, 2, 3, 4, 5};
  tree t2 = ilist2;

  t1.swap(t2);

  EXPECT_EQ(t1, ilist2);
  EXPECT_EQ(t2, ilist1);
}

TEST(UNIT_TESTING, FIND) {

  tree t = {1, 2, 3, 4, 5};

  for (auto it = t.begin(), end = t.end(); it != end; ++it) {
    EXPECT_EQ(t.find(*it), it);
  }
  
  EXPECT_EQ(t.find(10), t.end());
}

TEST(UNIT_TESTING, CONTAINS) {
  
  tree t = {1, 2, 3, 4, 5};

  for (auto it = t.begin(), end = t.end(); it != end; ++it) {
    EXPECT_TRUE(t.contains(*it));
  }

  EXPECT_FALSE(t.contains(10));
}

TEST(UNIT_TESTING, CLEAR) {
  
  tree t = {1, 2, 3, 4, 5};
  t.clear();
  EXPECT_TRUE(t.empty());

  tree empty_tree;
  empty_tree.clear();
  EXPECT_TRUE(empty_tree.empty());
}

TEST(UNIT_TESTING, UPPER_BOUND) {
  
  tree t = {1, 3, 5, 7, 9};
  EXPECT_EQ(*t.upper_bound(1), 3);
  EXPECT_EQ(*t.upper_bound(0), 1);
  EXPECT_EQ(t.upper_bound(9), t.end());
  EXPECT_EQ(t.upper_bound(10), t.end());
}

TEST(UNIT_TESTING, LOWER_BOUND) {
  
  tree t = {1, 3, 5, 7, 9};
  EXPECT_EQ(*t.lower_bound(1), 1);
  EXPECT_EQ(*t.lower_bound(2), 3);
  EXPECT_EQ(*t.lower_bound(9), 9);
  EXPECT_EQ(t.lower_bound(10), t.end());
}

TEST(UNIT_TESTING, EQUAL_RANGE) {
  
  tree t = {1, 3, 5, 7, 9};

  auto r1 = t.equal_range(1);
  EXPECT_EQ(*r1.first, 1);
  EXPECT_EQ(*r1.second, 3);

  auto r2 = t.equal_range(1);
  EXPECT_EQ(*r2.first, 1);
  EXPECT_EQ(*r2.second, 3);

  auto r3 = t.equal_range(9);
  EXPECT_EQ(*r3.first, 9);
  EXPECT_EQ(r3.second, t.end());

  auto r4 = t.equal_range(10);
  EXPECT_EQ(r4.first, t.end());
  EXPECT_EQ(r4.second, t.end());
}

TEST(UNIT_TESTING, DISTANCE) {
  
  tree t = {1, 3, 5, 7, 9};

  EXPECT_EQ(t.distance(t.begin(), std::next(t.begin(), 4)), 4);
  EXPECT_EQ(t.distance(1, 9), 4);

  EXPECT_EQ(t.distance(t.begin(), t.begin()), 0);
  EXPECT_EQ(t.distance(1, 1), 0);
}

int main(int argc, char** argv) {

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}