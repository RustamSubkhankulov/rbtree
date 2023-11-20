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

/* k-query implementation */
template <template<typename...> class Set, typename Key>
void query_insert(Set<Key>& set, const Key& key) {
  set.insert(key);
}

/* q-query implementation */
template <template<typename...> class Set, typename Key>
typename Set<Key>::difference_type 
query_distance(Set<Key>& set, typename Set<Key>::const_iterator first, 
                                typename Set<Key>::const_iterator second) {

  auto comp = set.key_comp();
  return comp(*first, *second)? std::distance(first, second) : 0;
}

template <template<typename...> class Set, typename Key>
typename Set<Key>::difference_type 
query_distance(Set<Key>& set, const Key& first, const Key& second) {

  auto comp = set.key_comp();
  return comp(first, second)? std::distance(set.find(first), set.find(second)) : 0;
}

/* fast q-query implementation using distance() method for RBTREE::rbtree. */
template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_distance_fast(RBTREE::rbtree<Key>& rbtree, const Key& first, const Key& second) {

  return rbtree.distance(first, second);
}

template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_distance_fast(RBTREE::rbtree<Key>& rbtree, typename RBTREE::rbtree<Key>::const_iterator first, 
                                                   typename RBTREE::rbtree<Key>::const_iterator second) {

  return rbtree.distance(first, second);
}
