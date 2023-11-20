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

  std::cerr << "first " << first << " second " << second << "\n";

  auto comp = set.key_comp();

  if (comp(second, first)) {
    return 0;
  }

  auto it_first = set.lower_bound(first), it_second = set.lower_bound(second);
  
  std::cerr << "*first " << *it_first << " *second " << *it_second << "\n";

  auto res = std::distance(it_first, it_second);

  std::cerr << "res " << res << "\n";

  return res;
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
