#pragma once

#include <set>
#include <iterator>

#include "rbtree.hpp"

/* 
 * Queries types: 
 * k - insert element, takes one arg.
 * q - distance between two elements, takes 
 *     two args, second is greater than first. 
 */
enum class query_type : char {
  K_INSERT   = 'k',
  Q_DISTANCE = 'q'
};

/* k-query implementation for RBTREE::rbtree. */
template <typename Key>
void query_k_insert(RBTREE::rbtree<Key>& rbtree, const Key& key) {
  rbtree.insert(key);
}

/* k-query implementation for std::set. */
template <typename Key>
void query_k_insert(std::set<int>& set, const Key& key) {
  set.insert(key);
}

/* q-query implementation using std::distance() for RBTREE::rbtree. */
template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_q_distance(RBTREE::rbtree<Key>& rbtree, const Key& first, const Key& second) {

  return std::distance(rbtree.find(first), rbtree.find(second));
}

template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_q_distance(RBTREE::rbtree<Key>& rbtree, typename RBTREE::rbtree<Key>::const_iterator first, 
                                              typename RBTREE::rbtree<Key>::const_iterator second) {

  return std::distance(first, second);
}

/* q-query implementation using std::distance() for std::set. */
template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_q_distance(std::set<int>& set, const Key& first, const Key& second) {

  return std::distance(set.find(first), set.find(second));
}

template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_q_distance(std::set<int>& set, typename std::set<int>::const_iterator first,
                                     typename std::set<int>::const_iterator second) {

  return std::distance(first, second);
}

/* fast q-query implementation using distance() method for RBTREE::rbtree. */
template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_q_distance_fast(RBTREE::rbtree<Key>& rbtree, const Key& first, const Key& second) {

  // return rbtree.distance(rbtree.find(first), rbtree.find(second));
  return rbtree.distance(first, second);
}

template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_q_distance_fast(RBTREE::rbtree<Key>& rbtree, typename RBTREE::rbtree<Key>::const_iterator first, 
                                                   typename RBTREE::rbtree<Key>::const_iterator second) {

  return rbtree.distance(first, second);
}