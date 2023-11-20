#pragma once 

#include <iterator>
#include <cstddef>

#include "node.hpp"

namespace RBTREE {

template < typename Key, typename Compare> 
class rbtree;

namespace DETAIL {

/* 
 * Iterator for the tree. 
 */
template <typename Node>
class const_iter final {

  /* Underlying node pointer. */
  using node = Node;
  /* End node type of the node type. */
  using end_node = typename node::end_node;
  
  const end_node* node_ptr_;

public:

  /* Member types so iterator can be used in standard algorithms. */
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = typename node::key_type;
  using pointer           = const value_type*;
  using reference         = const value_type&;

  explicit const_iter(const end_node* node_ptr = nullptr) noexcept 
  : node_ptr_(node_ptr) {}

  /* Implicit conversion to bool. */
  operator bool() const { 
    return static_cast<bool>(node_ptr_); 
  }

  reference operator*() const { return static_cast<const node*>(node_ptr_)->value; }
  pointer operator->() const { return &static_cast<const node*>(node_ptr_)->value; }

  const_iter& operator++();
  const_iter& operator--();

  const_iter operator++(int) { auto temp(*this); operator++(); return temp; }
  const_iter operator--(int) { auto temp(*this); operator--(); return temp; }

  friend bool operator==(const const_iter& lhs, const const_iter& rhs) {
    return (lhs.node_ptr_ == rhs.node_ptr_);
  }

  friend bool operator!=(const const_iter& lhs, const const_iter& rhs) {
    return (lhs.node_ptr_ != rhs.node_ptr_);
  }

  template <typename Key, typename Compare>
  friend class ::RBTREE::rbtree;
};

template <typename Node>
const_iter<Node>& const_iter<Node>::operator++() { 

  auto nd = static_cast<const node*>(node_ptr_);
  node* next = nd->get_right_thread();
  
  if (next != nullptr) {
    node_ptr_ = next;
  } else {
    node_ptr_ = nd->get_next();
  }

  return *this;
}

template <typename Node>
const_iter<Node>& const_iter<Node>::operator--() { 
  
  auto nd = static_cast<const node*>(node_ptr_);
  node* prev = nd->get_left_thread();
  
  if (prev != nullptr) {
    node_ptr_ = prev;
  } else {
    node_ptr_ = nd->get_prev();
  }

  return *this;
}

}; /* namespace DETAIL */

}; /* namespace RBTREE */
