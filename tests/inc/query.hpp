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

template <template<typename...> class Set, typename Key>
typename Set<Key>::difference_type 
query_distance(Set<Key>& set, const Key& first, const Key& second) {

  auto comp = set.key_comp();
  auto equiv = [&comp](const Key& first, const Key& second) -> bool { 
    return !comp(first, second) && !comp(second, first);
  };

  if (comp(second, first)) {
    return 0;
  }

  auto it_first = set.lower_bound(first), it_second = set.upper_bound(second);  
  return std::distance(it_first, it_second);
}

/* fast q-query implementation using distance() method for RBTREE::rbtree. */
template <typename Key>
typename RBTREE::rbtree<Key>::difference_type 
query_distance_fast(RBTREE::rbtree<Key>& set, const Key& first, const Key& second) {

  auto comp = set.key_comp();
  auto equiv = [&comp](const Key& first, const Key& second) -> bool { 
    return !comp(first, second) && !comp(second, first);
  };

  if (comp(second, first)) {
    return 0;
  }

  auto it_first = set.lower_bound(first), it_second = set.upper_bound(second);  
  return set.distance(it_first, it_second);
}

