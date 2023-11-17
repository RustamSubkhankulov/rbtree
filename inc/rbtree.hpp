#pragma once

#include <ios>
#include <new>
#include <stack>
#include <tuple>
#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <utility>
#include <cstddef>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <initializer_list>

#include "node.hpp"
#include "iter.hpp"

namespace RBTREE {

namespace dtl = DETAIL;

/* Red-black tree. */
template <typename Key, typename Compare = std::less<Key>> 
class rbtree {

public:

  using key_type        = Key;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;
  using key_compare     = Compare;    

  using const_reference = const key_type&;
  using const_pointer = const key_type*;

private:

  /* Node structure */
  using node = dtl::node_t<key_type>;
  /* Static end node type used for implementing post-end iterator */
  using end_node = typename node::end_node;

  /* Root. */
  using root_type = dtl::root<node>;
  root_type root;

  /* Dynamically updated leftmost node pointer for constant complexity begin(). */
  end_node* leftmost = root.end_node_ptr();
  /* Dynamically updated rightmost node pointer for constant complexity iter incrementing. */
  end_node* rightmost = root.end_node_ptr();

  /* Comparator. */
  Compare cmp;

  /* Get pointer to end_node of the tree. */
  const end_node* end_node_ptr() const { return root.end_node_ptr(); }
  end_node* end_node_ptr() { return root.end_node_ptr(); }

  /* Checks wether given node is root. */
  bool is_root(const node* node) const noexcept { return (node == root.get()); }

public:
  /* Bidirectional iterator. */
  using const_iterator = dtl::const_iter<node>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  /* Default ctor. */
  rbtree(const Compare& compare = Compare()) 
  noexcept(std::is_nothrow_copy_constructible_v<Compare>)
  : cmp(compare) {}

  /* Ctor from range defined by two iterators. */
  template <typename InputIt>
  rbtree(InputIt first, InputIt last, const Compare& compare = Compare())
  : cmp(compare) {

    for (; first != last; ++first) {
      insert(*first);
    }
  }

  rbtree(std::initializer_list<key_type> init, const Compare& compare = Compare())
  : rbtree(init.begin(), init.end(), compare) {}

  /* Copy ctor. */
  rbtree(const rbtree& that)
  : cmp(that.cmp) {

    auto copy = that.copy_subtree(that.root.get());
    
    root.set(copy.root);
    leftmost = (copy.leftmost)? copy.leftmost : end_node_ptr();
    rightmost = (copy.rightmost)? copy.rightmost : end_node_ptr();

    node::stitch_subtree(root.get());
  }

  /* Move ctor. */
  rbtree(rbtree&& that)
  noexcept(std::is_nothrow_move_constructible_v<node> &&
           std::is_nothrow_move_constructible_v<Compare>)
  : root(std::move(that.root)),
    leftmost(std::exchange(that.leftmost, that.root.end_node_ptr())), 
    rightmost(std::exchange(that.rightmost, that.root.end_node_ptr())), 
    cmp(std::move(that.cmp)) {

    relink_side_nodes(that);
  }

  /* Copy assignment. */
  rbtree& operator=(const rbtree& that) {

    if (this == &that)
      return *this;

    auto temp{that};
    swap(temp);

    return *this;
  }

  /* Move assignment. */
  rbtree& operator=(rbtree&& that) 
  noexcept(std::is_nothrow_swappable_v<node> &&
           std::is_nothrow_swappable_v<Compare>) {

    swap(that);
    return *this;
  }

  virtual ~rbtree() {
    free_subtree(root.get());
  }

  /* Insertion. */
  std::pair<const_iterator, bool> insert(key_type&& key);
  std::pair<const_iterator, bool> insert(const key_type& key);
  template <typename InputIt>
  void insert(InputIt first, InputIt last);
  void insert(std::initializer_list<key_type> init);

  /* 
   * Emplacement - constructing element in-place. 
   * Args are forwarded to constructor of element.
   */
  template< class... Args >
  std::pair<const_iterator, bool> emplace( Args&&... args );

  /* 
   * Erasure - element pointed by a iterator, a range of elements
   * defined by two iterators and element with a specific key.
   */
  const_iterator erase(const_iterator pos);
  const_iterator erase(const_iterator first, const_iterator last);
  bool erase(const key_type& key);

  /* Swap contents of two trees. No copying of elements are performed. */
  void swap(rbtree& that) 
  noexcept(std::is_nothrow_swappable_v<node> &&
           std::is_nothrow_swappable_v<Compare>) {

    std::swap(root, that.root);
    swap_side_nodes(that);
    std::swap(cmp, that.cmp);
  }

  /* Find element with key equivalent to a given argument. */
  const_iterator find (const key_type& key) const noexcept {
    return const_iterator(find_equiv_node(root.get(), key));
  }
  /* Check wether element with key equivalent to a given is in the tree. */
  bool contains(const key_type& key) const noexcept {
    return (find_equiv_node(root.get(), key) != end_node_ptr());
  }

  /* 
   * NOTE: no 'iterator' member type defined since 
   * no elements should be changed. Changing keys of elements
   * will violate its searching properties.
   */

  /* Iterator to the first element. */
  const_iterator cbegin() const noexcept { return const_iterator(leftmost); }
  const_iterator begin()  const noexcept { return cbegin(); }

  /* Reverise iterator to the first element. */
  const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
  const_reverse_iterator rbegin()  const noexcept { return crbegin();  }

  /* Past-end iterator. */
  const_iterator cend() const noexcept { return const_iterator(end_node_ptr()); }
  const_iterator end()  const noexcept { return cend(); }

  /* Reverse past-end iterator. */
  const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
  const_reverse_iterator rend()  const noexcept { return crend();  }

  /* Checks whether the container is empty */
  bool empty() const noexcept { return (root.get() == nullptr); }
  /* Returns the number of elements */
  size_type size() const noexcept { 
    return (empty())? 0 : root.get()->size; 
  }

  /* Clear contents of the tree. */
  void clear() noexcept;

  /* Distance between two nodes, defined by keys. */

  difference_type distance(const_iterator first, const_iterator second) const noexcept {
    return less_than(*second) - less_than(*first);
  }

  difference_type distance(const key_type& first, const key_type& second) const noexcept {
    return less_than(second) - less_than(first);
  }

  /* Returns an iterator to the first element not less than the given key */
  const_iterator lower_bound(const Key& key) const noexcept {
    return const_iterator(find_lower_bound_node(root.get(), key));
  }

  /* Returns an iterator to the first element greater than the given key */
  const_iterator upper_bound(const Key& key) const noexcept {
    return const_iterator(find_upper_bound_node(root.get(), key));
  }

  /* Returns range of elements matching a specific key */
  std::pair<const_iterator,const_iterator> equal_range(const Key& key) const {
    return std::make_pair(lower_bound(key), upper_bound(key));
  }

  /* Returns the function that compares keys. */
  key_compare key_comp() const { return cmp; }

  /* Graphical dump of the tree using graphviz dot to image with given name. */
  void graph_dump(const std::string& graph_name) const;

  /* Graphical dump of the tree using graphviz dot to ostream. */
  template <typename CharT>
  void graph_dump(std::basic_ostream<CharT>& os) const;

private:

  /* Helpers for swapping leftmost and rightmost node pointers. */
  void swap_side_nodes(rbtree& that) noexcept;
  void swap_leftmost(rbtree& that) noexcept;
  void swap_rightmost(rbtree& that) noexcept;

  /* Helpers for processing leftmost and rightmost pointer got from abother tree. */
  void relink_side_nodes(const rbtree& that) noexcept;
  void relink_leftmost(const rbtree& that) noexcept;
  void relink_rightmost(const rbtree& that) noexcept;

  /* Make a copy of subtree. */
  node::subtree_copy_t copy_subtree(const node* subtree) const;

  /* Free given subtree. */
  void free_subtree(node* subtree) const noexcept;

  /* Equivalence relationship deduced from compare function. */
  bool equiv(const key_type& lhs, const key_type& rhs) const {
    return !(cmp(lhs, rhs)) && !(cmp(rhs, lhs));
  }

  /* 
   *Link 'rhs' node into 'lhs' place in the tree. 
   * Does not change its children
   */
  void transplant(node* lhs, node* rhs) noexcept;

  /* Rotation methods for rebalancing. */
  void right_rotate(node* subtree_root) noexcept;
  void left_rotate (node* subtree_root) noexcept;

  /* Insert node and perform fixes to maintain invariants of the RB-tree. */
  bool insert_node(node* inserting);  
  /* Insert node into a tree just like in a regular BST tree. */
  bool insert_node_bst(node* subtree_root, node* inserting) noexcept;
  
  /* Fixing functions used on insertion. */
  void insert_rb_fix(node* inserted) noexcept;
  node* parent_grand_recolor(node* parent) noexcept;
  node* uncle_parent_grand_recolor(node* uncle, node* parent) noexcept;

  /* Delete given node and perform fixes to maintain invariants of the RB-tree. */
  void delete_node(node* deleting);  

  /* Fixing functions used on deletion. */
  node* delete_rb_fix(node* erased) noexcept;  

  /* Update stitches on deletion. */
  void update_stitches(end_node* prev, end_node* next) noexcept;
  void update_prev(end_node* prev) noexcept;
  void update_next(end_node* next) noexcept;
  
  /* Helper rebalancing functions. */
  std::pair<node*, node*> get_y_and_its_decs(node* y) noexcept;
  void  delete_rb_rebalance(node* x, node* parent_of_x) noexcept;
  node* delete_rb_rebalance_w_is_red(node* w, bool x_on_left, node* parent_of_x) noexcept;
  void  delete_rb_update_leftmost(node* z, node* x) noexcept;
  void  delete_rb_update_rightmost(node* z, node* x) noexcept;

  /* Helper function for finding nodes. */
  const end_node* find_equiv_node(const node* subtree_root, key_type key) const noexcept;

  const end_node* find_lower_bound_node(const node* subtree_root, key_type key) const noexcept;
  const end_node* find_upper_bound_node(const node* subtree_root, key_type key) const noexcept;
  
  /* Increase subtree size for each node in route from nd to root by 1. */
  void incr_subtree_sizes(end_node* nd) noexcept;

  /* Decrease subtree size for each node in route from nd to root by 1. */
  void decr_subtree_sizes(end_node* nd) noexcept;

  /* Get number of element smaller thatn given. */
  size_type less_than(const key_type& key) const noexcept;

  /* Validate tree - checks its rRB-properties. */
  bool debug_validate() const noexcept;

  /* Helper function for graphical dump. */
  template <typename CharT>
  void write_dot(std::basic_ostream<CharT>& os) const;

  static void generate_graph(const std::string& dot_file, 
                             const std::string& graph_name);
}; 

/* Equality comparison between two trees. */
template <typename Key, typename Compare>
bool operator==(const rbtree<Key, Compare>& lhs, const rbtree<Key, Compare>& rhs) {

  return (lhs.size() == rhs.size()) 
       && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/* Equality comparison betweeb tree and initilizer_list. */
template <typename Key, typename Compare>
bool operator==(const rbtree<Key, Compare>& lhs, const std::initializer_list<Key>& rhs) {

  return (lhs.size() == rhs.size()) 
       && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

/* Equality comparison betweeb tree and initilizer_list. */
template <typename Key, typename Compare>
bool operator==(const std::initializer_list<Key>& lhs, const rbtree<Key, Compare>& rhs) {

  return (lhs.size() == rhs.size()) 
       && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::swap_side_nodes(rbtree& that) noexcept {

  swap_leftmost(that);
  swap_rightmost(that);
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::swap_leftmost(rbtree& that) noexcept {

  std::swap(leftmost, that.leftmost);
  relink_leftmost(that);
  that.relink_leftmost(*this);
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::swap_rightmost(rbtree& that) noexcept {

  std::swap(rightmost, that.rightmost);
  relink_rightmost(that);
  that.relink_rightmost(*this);
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::relink_side_nodes(const rbtree& that) noexcept {

  relink_leftmost(that);  
  relink_rightmost(that);  
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::relink_leftmost(const rbtree& that) noexcept {

  if (leftmost == that.end_node_ptr()) {
    leftmost = end_node_ptr();
  } else {
    leftmost->stitch_left(end_node_ptr());
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::relink_rightmost(const rbtree& that) noexcept {

  if (rightmost == that.end_node_ptr()) {
    rightmost = end_node_ptr();
  } else {
    static_cast<node*>(rightmost)->stitch_right(end_node_ptr());
  }
}

template <typename Key, typename Compare>
std::pair<typename rbtree<Key, Compare>::const_iterator, bool>
rbtree<Key, Compare>::insert(key_type&& key) {
  
  if (find_equiv_node(root.get(), key) != end_node_ptr()) {
    return std::make_pair(cend(), false);
  } 

  node* nd = new node(std::move(key));
  insert_node(nd);
  return std::make_pair(const_iterator(nd), true);
}

template <typename Key, typename Compare>
std::pair<typename rbtree<Key, Compare>::const_iterator, bool>
rbtree<Key, Compare>::insert(const key_type& key) {

  if (find_equiv_node(root.get(), key) != end_node_ptr()) {
    return std::make_pair(cend(), false);
  }

  node* nd = new node(key);
  insert_node(nd);
  return std::make_pair(const_iterator(nd), true);
}

template <typename Key, typename Compare>
template <typename InputIt>
void rbtree<Key, Compare>::insert(InputIt first, InputIt last) {

  for (auto it = first; it != last; ++it) {
    insert(*it);
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::insert(std::initializer_list<key_type> init) {
  insert(init.begin(), init.end());
}

template <typename Key, typename Compare>
template< class... Args >
std::pair<typename rbtree<Key, Compare>::const_iterator, bool> 
rbtree<Key, Compare>::emplace( Args&&... args ) {

  node* nd = new node(std::forward<Args>(args)...);
  if (insert_node(nd)) {
    return std::make_pair(const_iterator(nd), true);
  }

  delete nd;
  return std::make_pair(cend(), false);
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::const_iterator 
rbtree<Key, Compare>::erase(const_iterator pos) {

  const_iterator next = std::next(pos);
  delete_node(const_cast<node*>(static_cast<const node*>(pos.node_ptr_)));
  return next;
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::const_iterator 
rbtree<Key, Compare>::erase(const_iterator first, const_iterator last) {

  while (first != last) {
    first = erase(first);
  }
  
  return first;
}

template <typename Key, typename Compare>
bool rbtree<Key, Compare>::erase(const key_type& key) {

  const end_node* nd = find_equiv_node(root.get(), key);
  if (nd == end_node_ptr()) {
    return false;
  }

  delete_node(const_cast<node*>(static_cast<const node*>(nd)));
  return true;
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::clear() noexcept {

  free_subtree(root.get());
  root.set(nullptr);
  leftmost = root.end_node_ptr();
  rightmost = root.end_node_ptr();
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::node::subtree_copy_t 
rbtree<Key, Compare>::copy_subtree(const node* subtree) const {

  return node::copy_subtree((struct node::subtree_info_t){subtree, leftmost, rightmost}, end_node_ptr());
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::free_subtree(node* subtree) const noexcept {

  node::free_subtree(subtree, end_node_ptr());
}

template <typename Key, typename Compare>
const typename rbtree<Key, Compare>::end_node* 
rbtree<Key, Compare>::find_equiv_node(const node* subtree_root, key_type key) const noexcept {

  while (subtree_root != nullptr) {

    if (cmp(key, subtree_root->value)) {
      subtree_root = subtree_root->get_left();

    } else if (cmp(subtree_root->value, key)) {
      subtree_root = subtree_root->get_right();

    } else {
      return subtree_root;
    }
  }

  return end_node_ptr();
}

template <typename Key, typename Compare>
const typename rbtree<Key, Compare>::end_node* 
rbtree<Key, Compare>::find_lower_bound_node(const node* subtree_root, 
                                                        key_type key) const noexcept {
  
  const end_node* res = end_node_ptr();
  while (subtree_root != nullptr) {

    if (!cmp(subtree_root->value, key)) {
      res = std::exchange(subtree_root, subtree_root->get_left());
    } else {
      subtree_root = subtree_root->get_right();
    }
  }

  return res;
}

template <typename Key, typename Compare>
const typename rbtree<Key, Compare>::end_node* 
rbtree<Key, Compare>::find_upper_bound_node(const node* subtree_root, 
                                                        key_type key) const noexcept {

  const end_node* res = end_node_ptr();
  while (subtree_root != nullptr) {

    if (cmp(key, subtree_root->value)) {
      res = std::exchange(subtree_root, subtree_root->get_left());
    } else {
      subtree_root = subtree_root->get_right();
    }
  }

  return res;
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::transplant(node* u, node* v) noexcept {

  if (is_root(u)) {
    root.set(v);

  } else if (u->on_left()) {
    u->parent()->tie_left(v);

  } else {
    u->parent()->tie_right(v);
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::right_rotate(node* subtree_root) noexcept {

  if (subtree_root == nullptr || !subtree_root->has_left())
    return;

  node* rotating = subtree_root->get_left_unsafe();

  if (is_root(subtree_root)) {
    root.set(rotating);

  } else {

    node* parent = subtree_root->parent();
    
    if (subtree_root->on_left()) {
      parent->tie_left(rotating);

    } else { /* parent->right == subtree_root */
      parent->tie_right(rotating);
    }
  }

  auto right = rotating->get_right_unsafe();

  if (rotating->has_right()) {
    subtree_root->tie_left(right);
  } else {
    subtree_root->stitch_left(subtree_root->get_prev());
  }

  rotating->tie_right(subtree_root);

  subtree_root->size -= 1 + ((rotating->has_left())? rotating->get_left_unsafe()->size : 0);
  rotating->size += 1 + ((subtree_root->has_right())? subtree_root->get_right_unsafe()->size : 0);
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::left_rotate(node* subtree_root) noexcept {

  if (subtree_root == nullptr || !subtree_root->has_right())
    return;

  node* rotating = subtree_root->get_right();

  if (is_root(subtree_root)) {
    root.set(rotating);

  } else {

    node* parent = subtree_root->parent();
    
    if (subtree_root->on_left()) {
      parent->tie_left(rotating);

    } else {
      parent->tie_right(rotating);
    }
  }

  auto left = rotating->get_left_unsafe();

  if (rotating->has_left()) {
    subtree_root->tie_right(left);
  } else {
    subtree_root->stitch_right(subtree_root->get_next());
  }

  rotating->tie_left(subtree_root);

  subtree_root->size -= 1 + ((rotating->has_right())? rotating->get_right_unsafe()->size : 0);
  rotating->size += 1 + ((subtree_root->has_left())? subtree_root->get_left_unsafe()->size : 0);
}

template <typename Key, typename Compare>
bool rbtree<Key, Compare>::insert_node(node* inserting) {

  if (empty()) {

    root.set(inserting);
    
    leftmost = rightmost = inserting;
    
    inserting->paint(node::color::BLACK);

  } else {
    
    if (!insert_node_bst(root.get(), inserting)) {
      return false;
    }

    incr_subtree_sizes(inserting->parent());

    if (inserting == leftmost->get_left()) {
      leftmost = inserting;
    }

    if (inserting == static_cast<node*>(rightmost)->get_right()) {
      rightmost = inserting;
    }
  }

  inserting->stitch();
  insert_rb_fix(inserting);

  assert(debug_validate());
  return true;
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::node* 
rbtree<Key, Compare>::parent_grand_recolor(node* parent) noexcept {

  using color_t = enum node::color;

  parent->paint(color_t::BLACK);

  node* grand = parent->parent();
  if (!is_root(parent)) {
    grand->paint(color_t::RED);
  }

  return grand;
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::node* 
rbtree<Key, Compare>::uncle_parent_grand_recolor(node* uncle, node* parent) noexcept {

  using color_t = enum node::color;

  uncle->paint(color_t::BLACK);
  parent->paint(color_t::BLACK);

  node* grand = parent->parent();
  if (!is_root(parent)) {
    grand->paint(color_t::RED);
  }

  return grand;
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::insert_rb_fix(node* new_node) noexcept {

  node *uncle, *parent = new_node->parent();

  while (!is_root(new_node) && parent->is_red()) {

    if (parent->on_left()) {

      uncle = new_node->uncle();
      if (uncle != nullptr && uncle->is_red()) {

        /* new_node = new_node->parent->parent */
        new_node = uncle_parent_grand_recolor(uncle, parent);
      
      } else {

        if (new_node->on_right()) {

          left_rotate(parent);
          parent = new_node;
        }

        right_rotate(parent_grand_recolor(parent));
        break;
      }

    } else {

      uncle = new_node->uncle();
      if (uncle != nullptr && uncle->is_red()) {

        /* new_node = new_node->parent->parent */
        new_node = uncle_parent_grand_recolor(uncle, parent);

      } else {

        if (new_node->on_left()) {

          right_rotate(parent);
          parent = new_node;
        }

        left_rotate(parent_grand_recolor(parent));
        break;
      }
    }

    parent = new_node->parent();
  }

  root.get()->paint(node::color::BLACK);
}

template <typename Key, typename Compare>
bool rbtree<Key, Compare>::insert_node_bst(node* subtree_root, node* inserting) noexcept {

  node* current = subtree_root;
  node* parent = subtree_root->parent();

  bool on_right = false;

  while (current != nullptr) {

    parent = current;

    if (!cmp(inserting->value, current->value)) {

      on_right = true;
      current = current->get_right();

    } else if (!cmp(current->value, inserting->value)) {

      on_right = false;
      current = current->get_left();

    } else {
      return false;
    }
  }

  inserting->set_parent(parent);

  if (on_right) {
    parent->tie_right(inserting);
  } else {
    parent->tie_left(inserting);
  }

  return true;
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::delete_node(node* deleting) {

  node* nd = delete_rb_fix(deleting);

  decr_subtree_sizes(nd);
  delete nd;

  assert(debug_validate());
}

template <typename Key, typename Compare>
std::pair<typename rbtree<Key, Compare>::node*, 
          typename rbtree<Key, Compare>::node*>
rbtree<Key, Compare>::get_y_and_its_decs(node* y) noexcept {

  if (!y->has_left()) {
    return std::make_pair(y, y->get_right());

  } else {

    if (!y->has_right()) {
      return std::make_pair(y, y->get_left());
    
    } else { /* z has two non-null children */

      y = node::get_leftmost_desc(y->get_right());
      return std::make_pair(y, y->get_right());
    }
  }
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::node* 
rbtree<Key, Compare>::delete_rb_rebalance_w_is_red(node* w, bool x_on_left, 
                                                         node* parent_of_x) noexcept {

  using color_t = enum node::color;

  w->paint(color_t::BLACK);
  parent_of_x->paint(color_t::RED);

  if (x_on_left) {
      
    left_rotate(parent_of_x);
    return parent_of_x->get_right();

  } else {
    
    right_rotate(parent_of_x);
    return parent_of_x->get_left();
  }

  return w;
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::delete_rb_rebalance(node* x, node* parent_of_x) noexcept {

  using color_t = enum node::color;

  while (!is_root(x) && (x == nullptr || x->color == color_t::BLACK)) {

    auto parent_of_x_l = parent_of_x->get_left();

    bool x_on_left = (x == parent_of_x_l);
    node* w = (x_on_left)? parent_of_x->get_right() : parent_of_x_l;
    
    if (w == nullptr) {
      break;
    }

    if (w->is_red()) {
      w = delete_rb_rebalance_w_is_red(w, x_on_left, parent_of_x);
    }

    if (w == nullptr) {
      break;
    }

    /* NOTE: nullptr node is also black one */
    if (node::is_black(w->get_left()) && node::is_black(w->get_right())) {

      w->paint(color_t::RED);
      x = parent_of_x;
      parent_of_x = parent_of_x->parent();

    } else {

      if (x_on_left) {

        auto w_r = w->get_right();
        if (w_r == nullptr || w_r->color == color_t::BLACK) {

          w->get_left()->color = color_t::BLACK;
          w->color = color_t::RED;
          right_rotate(w);
          w = parent_of_x->get_right();
        }

      } else {

        auto w_l = w->get_left();
        if (w_l == nullptr || w_l->color == color_t::BLACK) {

          w->get_right()->color = color_t::BLACK;
          w->color = color_t::RED;
          left_rotate(w);
          w = parent_of_x->get_left();
        }
      }

      w->color = parent_of_x->color;
      parent_of_x->paint(color_t::BLACK);

      node* nd = (x_on_left)? w->get_right() : w->get_left();
      if (nd != nullptr) {
        nd->paint(color_t::BLACK);
      }

      if (x_on_left) {
        left_rotate(parent_of_x);
      } else {
        right_rotate(parent_of_x);
      }
      break;
    }
  }

  if (x != nullptr) {
    x->color = color_t::BLACK;
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::delete_rb_update_leftmost(node* z, node* x) noexcept {

  if (!z->has_right()) {
    
    if (!is_root(z)) {
      leftmost = z->parent();
    } else {
      leftmost = end_node_ptr();
    }
  
  } else {
    leftmost = node::get_leftmost_desc(x);
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::delete_rb_update_rightmost(node* z, node* x) noexcept {

  if (!z->has_left()) {
    
    if (!is_root(z)) {
      rightmost = z->parent();
    } else {
      rightmost = end_node_ptr();
    }
  
  } else {
    rightmost = node::get_rightmost_desc(x);
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::update_stitches(end_node* prev, end_node* next) noexcept {

  update_prev(prev);
  update_next(next);
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::update_prev(end_node* prev) noexcept {

  if (prev != end_node_ptr()) {

    auto nd = static_cast<node*>(prev);
    if (!nd->has_right()) {
      nd->stitch_right(nd->get_next());
    }
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::update_next(end_node* next) noexcept {

  if (next != end_node_ptr()) {
    auto nd = static_cast<node*>(next);
    if (!nd->has_left()) {
      nd->stitch_left(nd->get_prev());
    }
  }
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::node*
rbtree<Key, Compare>::delete_rb_fix(node* z) noexcept {

  auto next = z->get_next();
  auto prev = z->get_prev();

  node* parent_of_x = nullptr;
  auto [y, x] = get_y_and_its_decs(z);

  if (y != z) {

    auto z_left = z->get_left_unsafe();
    z_left->set_parent(y); /* relink y in place of z, y is z's desc */
    y->set_left(z_left);
    
    auto z_right = z->get_right();
    if (y != z_right) {

      parent_of_x = y->parent();
      if (x != nullptr) {
        x->set_parent(y->parent());
      }

      y->parent()->set_left(x);
      y->set_right(z_right);
      z_right->set_parent(y);
    
    } else {
      parent_of_x = y;
    }

    transplant(z, y);

    std::swap(y->color, z->color);
    y = z; /* y now points to node to be actually deleted */
  
  } else {

    /* y == z */
    parent_of_x = y->parent();
    if (x != nullptr) {
      x->set_parent(y->parent());
    }

    transplant(z, x);
    
    if (parent_of_x != end_node_ptr()) {
      parent_of_x->stitch();
    }

    if (leftmost == z) {
      delete_rb_update_leftmost(z, x);
    }

    if (rightmost == z) {
      delete_rb_update_rightmost(z, x);
    }
  }

  update_stitches(prev, next);

  if (y->is_black()) {
    delete_rb_rebalance(x, parent_of_x);
  }

  return z;
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::incr_subtree_sizes(end_node* nd) noexcept {

  if (nd == nullptr) {
    return;
  }

  node* cur;
  while (nd != end_node_ptr()) {

    cur = static_cast<node*>(nd);
    ++cur->size;
    nd = cur->parent_as_end();
  }
}

template <typename Key, typename Compare>
void rbtree<Key, Compare>::decr_subtree_sizes(end_node* nd) noexcept {

  if (nd == nullptr) {
    return;
  }

  node* cur;
  while (nd != end_node_ptr()) {

    cur = static_cast<node*>(nd);
    --cur->size;
    nd = cur->parent_as_end();
  }
}

template <typename Key, typename Compare>
typename rbtree<Key, Compare>::size_type 
rbtree<Key, Compare>::less_than(const key_type& key) const noexcept {

  const end_node* current = find_lower_bound_node(root.get(), key);

  if (current == end_node_ptr()) {
    return size();
  }

  size_type number = node::subtree_size(current->get_left());

  while (current != end_node_ptr()) {

    auto nd = static_cast<const node*>(current);
    if (nd->on_right()) {
      number += 1 + node::subtree_size(nd->sibling());
    }

    current = static_cast<const node*>(current)->parent_as_end();
  }

  return number;
}

template <typename Key, typename Compare>
bool rbtree<Key, Compare>::debug_validate() const noexcept {

  const node* root_node = root.get();

  if (root_node == nullptr) {
    return true;
  }

  bool res = true;

  if (root_node->color != node::color::BLACK) {

    std::cerr << "Debug validation: root is not black. \n";
    res = false;
  }

  for (auto it = cbegin(), end_it = cend(); it != end_it; ++it) {

    auto nd = static_cast<const node*>(it.node_ptr_);
    if (!nd->debug_validate()) {
      res = false;
    }
  }

  if (!res) {
    std::cerr << "Debug validation: FAILED \n";
  }

  return res;
}

/* 
 * Makes graphical dump of the tree using graphviz dot
 * Generates file with name 'graph_name' in png format in current 
 * working directory. 
 */
template <typename Key, typename Compare>
void rbtree<Key, Compare>::graph_dump(const std::string& graph_name) const {

  char dot_file_name[] = "graphXXXXXX";
  if (mkstemp(dot_file_name) == -1) {
    
    std::cerr << "graph_dump(): mkstemp() failed.\n";
    return;
  }

  std::ofstream dot_file(dot_file_name, std::ios_base::out 
                                      | std::ios_base::trunc);
  if (!dot_file.is_open()) {

    std::cerr << "graph_dump(): failed to open temp file.\n";
    return; 
  }
  
  graph_dump(dot_file);

  generate_graph(dot_file_name, graph_name);
  remove(dot_file_name);
}

template <typename Key, typename Compare>
  template <typename CharT>
  void rbtree<Key, Compare>::graph_dump(std::basic_ostream<CharT>& os) const {

    os << "digraph G{\n rankdir=TB;\n "
       << "node[ shape = doubleoctagon; style = filled ];\n"
       << "edge[ arrowhead = vee ];\n";

    write_dot(os);

    os << "\n}\n";
  }

/* Call dot to generate png image from txt source. */
template <typename Key, typename Compare>
void rbtree<Key, Compare>::generate_graph(const std::string& dot_file, 
                                          const std::string& graph_name) {

  std::string cmnd = "dot " + dot_file + " -Tpng -o " + graph_name;
  std::system(cmnd.c_str());
}

/* Write tree desctiption in dot format to temporary text file. */
template <typename Key, typename Compare>
  template <typename CharT>
  void rbtree<Key, Compare>::write_dot(std::basic_ostream<CharT>& os) const {

    using std::size_t;

    node::write_pastend_dot(os, reinterpret_cast<uintptr_t>(end_node_ptr()));

    if (empty()) {
      return;
    }

    if (root.get() != nullptr) {

      os << "NODE" << end_node_ptr() << " -> "
         << "NODE" << root.get() << " ["
         << " label = \"L\" ]; \n";
    }

    for (auto it = cbegin(), end_it = cend(); it != end_it; ++it) {

      static_cast<const node*>(it.node_ptr_)->write_dot(os);
    }
  }

}; /* namespace RBTREE */

namespace std {

template <typename Key, typename Compare>
constexpr void swap(::RBTREE::rbtree<Key, Compare>& lhs, 
                    ::RBTREE::rbtree<Key, Compare>& rhs) 
noexcept(std::is_nothrow_move_constructible_v<::RBTREE::rbtree<Key, Compare>> && 
         std::is_nothrow_move_assignable_v<::RBTREE::rbtree<Key, Compare>>) {

  lhs.swap(rhs);
}

}; /* namespace std */
