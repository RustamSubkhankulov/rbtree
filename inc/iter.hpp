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
  operator bool() const noexcept { 
    return static_cast<bool>(node_ptr_); 
  }

  reference operator*() const noexcept { return static_cast<const node*>(node_ptr_)->value; }
  pointer operator->() const noexcept { return &static_cast<const node*>(node_ptr_)->value; }

  const_iter& operator++() noexcept;
  const_iter& operator--() noexcept;

  const_iter operator++(int) noexcept { auto temp(*this); operator++(); return temp; }
  const_iter operator--(int) noexcept { auto temp(*this); operator--(); return temp; }

  friend bool operator==(const const_iter& lhs, const const_iter& rhs) noexcept {
    return (lhs.node_ptr_ == rhs.node_ptr_);
  }

  friend bool operator!=(const const_iter& lhs, const const_iter& rhs) noexcept {
    return (lhs.node_ptr_ != rhs.node_ptr_);
  }

  template <typename Key, typename Compare>
  friend class ::RBTREE::rbtree;
};

template <typename Node>
const_iter<Node>& const_iter<Node>::operator++() noexcept { 

  if (static_cast<const node*>(node_ptr_)->has_right()) {
    node_ptr_ = node::get_leftmost_desc(static_cast<const node*>(node_ptr_)->get_right());

  } else {

    const end_node* prev = node_ptr_;
    node_ptr_ = static_cast<const node*>(node_ptr_)->parent_as_end();

    while (static_cast<const node*>(node_ptr_)->parent_as_end() != nullptr) {

      if (prev == node_ptr_->get_left()) {
        break;
      }

      prev = node_ptr_;
      node_ptr_ = static_cast<const node*>(node_ptr_)->parent_as_end();  
    }
  }

  return *this;
}

template <typename Node>
const_iter<Node>& const_iter<Node>::operator--() noexcept { 
  
  if (node_ptr_->has_left()) {
    node_ptr_ = node::get_rightmost_desc(node_ptr_->get_left());

  } else {

    const end_node* prev = node_ptr_;
    node_ptr_ = static_cast<const node*>(node_ptr_)->parent_as_end();

    while (static_cast<const node*>(node_ptr_)->parent_as_end() != nullptr) {

      if (prev == static_cast<const node*>(node_ptr_)->get_right()) {
        break;
      }

      prev = node_ptr_;
      node_ptr_ = static_cast<const node*>(node_ptr_)->parent_as_end();  
    }
  }

  return *this;
}

}; /* namespace DETAIL */

}; /* namespace RBTREE */
