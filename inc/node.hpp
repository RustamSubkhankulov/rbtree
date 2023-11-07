#pragma once 

#include <ios>
#include <string>
#include <memory>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <iostream>
#include <type_traits>

namespace RBTREE {

namespace DETAIL {

/* Node structure representing end node. */
template <typename Node>
class end_node_t {

public:

  using node_t = Node;

protected:

  bool left_is_thread = false;
  node_t* left = nullptr;

public:

  end_node_t(node_t* lft = nullptr) noexcept:
  left(lft) {};

  end_node_t(const end_node_t& that) = delete;
  end_node_t& operator=(const end_node_t& that) = delete;

  end_node_t(end_node_t&& that) noexcept
  : left(std::exchange(that.left, nullptr)),
    left_is_thread(std::exchange(that.left_is_thread, false)) {} 

  end_node_t& operator=(end_node_t&& that) noexcept {
    std::swap(left, that.left);
    std::swap(left_is_thread, that.left_is_thread);
    return *this;
  }

  virtual ~end_node_t() {};

  node_t* get_left() const noexcept { 
    return (left_is_thread)? nullptr : left; 
  }

  node_t* get_left_thread() const noexcept {
    return (left_is_thread)? left : nullptr;
  }

  node_t* get_left_unsafe() const noexcept {
    return left;
  }

  void set_left(node_t* nd) noexcept {

    left_is_thread = false;
    left = nd;
  }

  node_t* tie_left(node_t* child) noexcept {

    node_t* prev = left;

    left_is_thread = false;
    left = child;
    
    if (left != nullptr) {
      left->set_parent(this);
    }

    return prev;
  }

  node_t* stitch_left(node_t* nd) noexcept {

    node_t* prev = left;

    left_is_thread = true;
    left = nd;
    
    return prev;
  }

  bool has_left() const noexcept { 
    return (!left_is_thread) && (left != nullptr); 
  }

  bool is_thread_left() const noexcept {
    return left_is_thread;
  }
};

/* Node structure used in searching tree. */
template <typename Key>
class node_t : public end_node_t<node_t<Key>> {

public:

  /*Base class type*/
  using end_node = end_node_t<node_t>;

  /* Key value that node holds. */
  using key_type = Key;
  key_type value;

  /* 
   * Every node is either red or black. 
   * Inserted node is red by default.
   */  
  enum class color { RED, BLACK };
  color color = color::RED;

  using size_type = std::size_t;
  /* Subtree size. */
  size_type size = 1;

private:

  using end_node::left_is_thread;
  using end_node::left;

  bool right_is_thread = false;
  node_t* right = nullptr;

  end_node* parent_ = nullptr;

public:

  node_t(const node_t& that)
  noexcept(std::is_nothrow_copy_constructible_v<key_type>)
  : value(that.value),
    color(that.color),
    size(that.size) {}

  node_t& operator=(const node_t& that) = delete;

  node_t(node_t&& that) 
  noexcept(std::is_nothrow_move_constructible_v<key_type>)
  : end_node(std::move(that)), 
    value(std::move(that.value)),
    color(that.color), 
    size(std::exchange(that.size, 1)),
    right_is_thread(std::exchange(that.right_is_thread, false)),
    right(std::exchange(that.right, nullptr)),
    parent_(std::exchange(that.parent_, nullptr)) {}

  node_t& operator=(node_t&& that) 
  noexcept(std::is_nothrow_swappable_v<key_type>) {

    std::swap(static_cast<end_node>(*this), static_cast<end_node>(that));
    std::swap(value, that.value);
    std::swap(color, that.color);
    std::swap(size, that.size);
    std::swap(right_is_thread, that.right_is_thread);
    std::swap(right, that.right);
    std::swap(parent_, that.parent_);

  }

  /* Construct node that holds copy of key. */
  node_t(const key_type& key) 
  noexcept(std::is_nothrow_copy_constructible_v<key_type>)
  : value(key) {}

  /* Move-constructs value from key. */
  node_t(key_type&& key)
  noexcept(std::is_nothrow_move_constructible_v<key_type>)
  : value(std::move(key)) {}

  using end_node::get_left;
  using end_node::get_left_thread;
  using end_node::get_left_unsafe;

  node_t* get_right() const { 
    return (right_is_thread)? nullptr : right; 
  }

  node_t* get_right_thread() const { 
    return (right_is_thread)? right : nullptr; 
  }

  node_t* get_right_unsafe() const { 
    return right; 
  }

  using end_node::set_left;
  void set_right(node_t* nd) noexcept {

    right_is_thread = false;
    right = nd;
  }

  /* Get parent node as end_node pointer or node_t pointer. */
  end_node* parent_as_end() const { return parent_; }
  node_t* parent() const { return static_cast<node_t*>(parent_); }

  /* Set parent node using end_node pointer. */
  void set_parent(end_node* parent) { parent_ = parent; }

  bool is_leaf() const noexcept { return ((left == nullptr) && (right == nullptr));}

  using end_node::has_left;
  bool has_right() const noexcept { 
    return (!right_is_thread) && (right != nullptr);
  }

  using end_node::is_thread_left;
  bool is_thread_right() const noexcept {
    return right_is_thread;
  }

  bool is_red() const noexcept { return (color == color::RED); }
  bool is_black() const noexcept { return (color == color::BLACK); }

  /* 
   * NOTE: nullptr node is also a black one. 
   * Methods for getting color of node are made both my member functions and 
   * outstanding ones so it will be easier to check whether pointer 
   * points to black node or not without additional check for pointer not to be nullptr.
   */
  static bool is_red  (const node_t* nd) noexcept { return (nd != nullptr && nd->is_red()); }
  static bool is_black(const node_t* nd) noexcept { return (nd == nullptr || nd->is_black()); }

  /* 
   * paint() method so no need to write 'color' 
   * twice in situations like the following: 
   * node_t->color = node_t::color::BLACK
   * we can just write: 'node_t->paint(node_t::color::BLACK)' instead. 
   */
  void paint (enum color clr) noexcept { color = clr; }

  node_t* tie_right(node_t* child) noexcept {

    node_t* prev = right;

    right_is_thread = false;
    right = child;

    if (right != nullptr) {
      right->set_parent(this);      
    }

    return prev;
  }

  using end_node::stitch_left;
  node_t* stitch_right(node_t* nd) noexcept {

    node_t* prev = right;

    right_is_thread = true;
    right = nd;
    
    return prev;
  }

  bool on_left() const noexcept {
    return (parent_ == nullptr)? false : this == parent_->get_left();
  }

  bool on_right() const noexcept {
    return (parent_ == nullptr)? false : this == parent()->get_right();
  }

  node_t* sibling() const noexcept {
    return (parent_ == nullptr)? nullptr : (on_left())? parent()->get_right() : parent_->get_left();
  }

  node_t* uncle() const noexcept {
    return (parent_ == nullptr)? nullptr : parent()->sibling();
  }

  static const node_t* get_leftmost_desc (const node_t* subtree_root) noexcept;
  static node_t* get_leftmost_desc (node_t* subtree_root) noexcept;
  
  static const node_t* get_rightmost_desc(const node_t* subtree_root) noexcept;
  static node_t* get_rightmost_desc(node_t* subtree_root) noexcept;

  static size_type subtree_size(const node_t* subtree_root) noexcept {
    return (subtree_root != nullptr)? subtree_root->size : 0;
  }

  const node_t* get_prev(const end_node* root) const noexcept;
  node_t* get_prev(const end_node* root) noexcept;

  const node_t* get_next(const end_node* root) const noexcept;
  node_t* get_next(const end_node* root) noexcept;

  void stitch(const end_node* root) noexcept;

  /* Validate node - checks its rRB-properties. */
  bool debug_validate() const;

  /* Helper functions used for graphical dump of the tree. */
  void write_dot(std::ofstream& of) const;
  static void write_nill_dot(std::ofstream& of, uintptr_t node_num);
  static void write_pastend_dot(std::ofstream& of, uintptr_t node_num);

};

template <typename Key>
node_t<Key>* node_t<Key>::get_leftmost_desc(node_t* cur) noexcept {

  while (cur != nullptr && cur->has_left()) {
    cur = cur->get_left();
  }

  return cur;
}

template <typename Key>
const node_t<Key>* node_t<Key>::get_leftmost_desc(const node_t* cur) noexcept {

  while (cur != nullptr && cur->has_left()) {
    cur = cur->get_left();
  }

  return cur;
}

template <typename Key>
node_t<Key>* node_t<Key>::get_rightmost_desc(node_t* cur) noexcept {

  while (cur != nullptr && cur->has_right()) {
    cur = cur->get_right();
  }

  return cur;
}

template <typename Key>
const node_t<Key>* node_t<Key>::get_rightmost_desc(const node_t* cur) noexcept {

  while (cur != nullptr && cur->has_right()) {
    cur = cur->get_right();
  }

  return cur;
}

template <typename Key>
const node_t<Key>* node_t<Key>::get_prev(const end_node* root) const noexcept {

  if (has_left()) {
    return node_t::get_rightmost_desc(left);

  } else {

    const node_t* prev = this;
    const end_node* node_ptr = parent_as_end();

    while (node_ptr != root) {

      auto nd = static_cast<const node_t*>(node_ptr);

      if (prev == nd->right) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return static_cast<const node_t*>(node_ptr);
  }
}

template <typename Key>
node_t<Key>* node_t<Key>::get_prev(const end_node* root) noexcept {

  if (has_left()) {
    return node_t::get_rightmost_desc(left);

  } else {

    node_t* prev = this;
    end_node* node_ptr = parent_as_end();

    while (node_ptr != root) {

      auto nd = static_cast<node_t*>(node_ptr);

      if (prev == nd->right) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return static_cast<node_t*>(node_ptr);
  }
}

template <typename Key>
const node_t<Key>* node_t<Key>::get_next(const end_node* root) const noexcept {

  if (has_right()) {
    return node_t::get_leftmost_desc(right);

  } else {

    const node_t* prev = this;
    const end_node* node_ptr = parent_as_end();

    while (node_ptr != root) {

      auto nd = static_cast<const node_t*>(node_ptr);

      if (prev == nd->left) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return static_cast<const node_t*>(node_ptr);
  }
}

template <typename Key>
node_t<Key>* node_t<Key>::get_next(const end_node* root) noexcept {

  if (has_right()) {
    return node_t::get_leftmost_desc(right);

  } else {

    node_t* prev = this;
    end_node* node_ptr = parent_as_end();

    while (node_ptr != root) {

      auto nd = static_cast<node_t*>(node_ptr);

      if (prev == nd->left) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return static_cast<node_t*>(node_ptr);
  }
}

template <typename Key>
void node_t<Key>::stitch(const end_node* root) noexcept {

  if (!has_left()) {
    stitch_left(get_prev(root));
  }

  if (!has_right()) {
    stitch_right(get_next(root));
  } 
}


template <typename Key>
bool node_t<Key>::debug_validate() const {

  if (has_left()) {

    if (color == color::RED && left->color != color::BLACK) {

      std::cerr << "Debug validation: descendant of a red node is not black. \n";
      return false;
    }

    if (!left->debug_validate())
      return false;
  }

  if (has_right()) {

    if (color == color::RED && right->color != color::BLACK) {
      
      std::cerr << "Debug validation: descendant of a red node is not black. \n";
      return false;
    }

    if (!right->debug_validate())
      return false;
  }

  return true;
}

template <typename Node>
class root final {

public:

  using node = Node;
  using end_node = typename Node::end_node;

private:

  end_node end;

public:

  root(node* root = nullptr) noexcept {
    end.tie_left(root);
  }

  root(const root& that) = delete;
  root& operator=(const root& that) = delete;

  root(root&& that) noexcept {
    end.tie_left(that.end.tie_left(nullptr));
  }

  root& operator=(root&& that) noexcept {
    
    end.tie_left(that.end.tie_left(end.get_left()));
    return *this;
  }

  void set(node* root) noexcept { 
    end.tie_left(root);
  }
  
  const node* get() const noexcept { return end.get_left(); }
  node* get() noexcept { return end.get_left(); }

  const end_node* end_node_ptr() const { return std::addressof(end); }
  end_node* end_node_ptr() { return std::addressof(end); }
};

/* Write node desctiption in dot format to temporary text file. */
template <typename Key>
void DETAIL::node_t<Key>::write_dot(std::ofstream& of) const {

  of << "NODE" << this << " ["
     << " label = < " << value << " <BR /> "
     << " <FONT POINT-SIZE=\"10\"> size: " << size << " </FONT> <BR /> "
     << " <FONT POINT-SIZE=\"10\"> addr: " << (void*) this << " </FONT>> "
     << " color = \"" << (is_red()? "#FD0000" : "#000000") << "\""
     << " fontcolor = \"" << (is_black()? "#FFFFFF" : "#000000") << "\""
     << " ]; \n";

  of << "NODE" << this << " -> "
     << "NODE" << parent_ << " ["
     << " style = \"dashed\""
     << " label = \"P\" ]; \n";

  const void *l, *r; 

  if (!has_left()) {

    write_nill_dot(of, reinterpret_cast<uintptr_t>(&left));
    l = reinterpret_cast<const void*>(&left);

  } else {
    l = reinterpret_cast<const void*>(left);
  }

  if (!has_right()) {

    write_nill_dot(of, reinterpret_cast<uintptr_t>(&right));
    r = reinterpret_cast<const void*>(&right);

  } else {
    r = reinterpret_cast<const void*>(right);
  }

  of << "NODE" << this << " -> "
     << "NODE" << l << " [ label = \"L\" ]; \n";

  of << "NODE" << this << " -> "
     << "NODE" << r << " [ label = \"R\" ]; \n";

  if (left_is_thread) {

    of << "NODE" << this << " -> "
       << "NODE" << left
       << " [ label = \"PREV\" style = \"dotted\" " 
       << " fontcolor = \"#a3a3c2\" color = \"#a3a3c2\" ]; \n"; 
  }

  if (right_is_thread) {

    of << "NODE" << this << " -> "
       << "NODE" << right
       << " [ label = \"NEXT\" style = \"dotted\" " 
       << " fontcolor = \"#a3a3c2\" color = \"#a3a3c2\" ]; \n"; 
  }
}

/* Helper function to add nill nodes. */
template <typename Key>
void DETAIL::node_t<Key>::write_nill_dot(std::ofstream& of, uintptr_t node_num) {

  of << "NODE" << std::hex << std::showbase << node_num << std::dec << " ["
     << " label = \"nill\" color = \"#000000\" width=0.1" 
     << " fontcolor = \"#FFFFFF\" fontsize = \"10\" shape = \"oval\" ]; \n";
}

/* Helper function to add past-end node. */
template <typename Key>
void DETAIL::node_t<Key>::write_pastend_dot(std::ofstream& of, uintptr_t node_num) {

  of << "NODE" << std::hex << std::showbase << node_num << std::dec << " ["
     << " label = \"PAST-END\" color = \"#00FFFF\" width=0.1" 
     << " fontcolor = \"#000000\" fontsize = \"10\" shape = \"diamond\" ]; \n";
}

}; /* namespace DETAIL */

}; /* namespace RBTREE */