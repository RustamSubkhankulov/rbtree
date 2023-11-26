#pragma once 

#include <ios>
#include <stack>
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

  /* Flag that shows that left pointer is thread, node left child. */
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

  /* Get letf child if it is present, otherwise nullptr. */
  node_t* get_left() const noexcept { 
    return (left_is_thread)? nullptr : left; 
  }

  /* Get link to prev node if it is present, otherwise nullptr. */
  node_t* get_left_thread() const {
    return (left_is_thread)? left : nullptr;
  }

  /* Get pointer to left node - child or prev. */
  node_t* get_left_unsafe() const {
    return left;
  }

  /* Set pointer to left child. */
  void set_left(node_t* nd) {

    left_is_thread = false;
    left = nd;
  }

  /* Set pointer to left child and set its parent to this. */
  node_t* tie_left(node_t* child) noexcept {

    node_t* prev = left;

    left_is_thread = false;
    left = child;
    
    if (left != nullptr) {
      left->set_parent(this);
    }

    return prev;
  }

  /* Make thread to prev node. */
  node_t* stitch_left(end_node_t* nd) noexcept {

    node_t* prev = left;

    left_is_thread = true;
    left = static_cast<node_t*>(nd);
    
    return prev;
  }

  /* Check whether left child is present. */
  bool has_left() const noexcept { 
    return (!left_is_thread) && (left != nullptr); 
  }

  /* Check whether thread to prev node is present. */
  bool is_thread_left() const {
    return left_is_thread;
  }

  /* 
   * Get parent node as end_node pointer or node_t pointer. 
   * Returns nullptr since end node has no parent.
   */
  virtual end_node_t* parent_as_end() const { return nullptr; }
  virtual node_t* parent() const { return nullptr; }
};

/* Root node temaplte class. */
template <typename Node>
class root final {

public:

  using node = Node;
  /* Corresponding end node type. */
  using end_node = typename Node::end_node;

private:

  /* End node of the tree. */
  end_node end;

public:

  root(node* root = nullptr) 
  noexcept(std::is_nothrow_default_constructible_v<end_node> && 
           noexcept(set(std::declval<node*>()))) {
    set(root);
  }

  root(const root& that) = delete;
  root& operator=(const root& that) = delete;

  root(root&& that) 
  noexcept(noexcept(set(std::declval<node*>()))) {
    set(that.set(nullptr));
  }

  void swap(root& that) 
  noexcept(noexcept(set(std::declval<node*>()))) {
    set(that.set(end.get_left()));
  }

  root& operator=(root&& that) 
  noexcept(noexcept(swap(std::declval<root&>()))) {
    
    swap(that);
    return *this;
  }

  ~root() {
    clear();
  }

  void clear() {
    node::free_subtree(get(), end_node_ptr());
    set(nullptr);
  }

  node* set(node* root) 
  noexcept(noexcept(std::declval<end_node>().tie_left(std::declval<node*>()))) { 
    return end.tie_left(root);
  }
  const node* get() const noexcept { return end.get_left(); }
  node* get() noexcept { return end.get_left(); }

  const end_node* end_node_ptr() const { return std::addressof(end); }
  end_node* end_node_ptr() { return std::addressof(end); }
};

/* Node structure used in searching tree. */
template <typename Key>
class node_t : public end_node_t<node_t<Key>> {

public:

  /* Root node type. */
  using root_type = root<node_t>;

  /* Base class type. */
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

  /* Flag that shows that left pointer is thread, node left child. */
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

  void swap(node_t& that)
  noexcept(std::is_nothrow_swappable_v<key_type>) {

    std::swap(static_cast<end_node>(*this), static_cast<end_node>(that));
    std::swap(value, that.value);
    std::swap(color, that.color);
    std::swap(size, that.size);
    std::swap(right_is_thread, that.right_is_thread);
    std::swap(right, that.right);
    std::swap(parent_, that.parent_);
  }

  node_t& operator=(node_t&& that) 
  noexcept(noexcept(swap(std::declval<node_t&>()))) {

    swap(that);
    return *this;
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

  /* Get right child if it is present, otherwise nullptr. */
  node_t* get_right() const noexcept { 
    return (right_is_thread)? nullptr : right; 
  }

  /* Get link to next node if it is present, otherwise nullptr. */
  node_t* get_right_thread() const { 
    return (right_is_thread)? right : nullptr; 
  }

  /* Get pointer to right node - child or prev. */
  node_t* get_right_unsafe() const { 
    return right; 
  }

  /* Set pointer to left child. */
  using end_node::set_left;

  /* Set pointer to right child. */
  void set_right(node_t* nd) {

    right_is_thread = false;
    right = nd;
  }

  /* Get parent node as end_node pointer or node_t pointer. */
  end_node* parent_as_end() const override { return parent_; }
  node_t* parent() const override { return static_cast<node_t*>(parent_); }

  /* Set parent node using end_node pointer. */
  void set_parent(end_node* parent) { parent_ = parent; }

  bool is_leaf() const { return ((left == nullptr) && (right == nullptr));}

  /* Check whether left child is present. */
  using end_node::has_left;

  /* Check whether right child is present. */
  bool has_right() const noexcept { 
    return (!right_is_thread) && (right != nullptr);
  }

  /* Check whether thread to prev node is present. */
  using end_node::is_thread_left;

  /* Check whether thread to next node is present. */
  bool is_thread_right() const {
    return right_is_thread;
  }

  bool is_red() const { return (color == color::RED); }
  bool is_black() const { return (color == color::BLACK); }

  /* 
   * NOTE: nullptr node is also a black one. 
   * Methods for getting color of node are made both my member functions and 
   * outstanding ones so it will be easier to check whether pointer 
   * points to black node or not without additional check for pointer not to be nullptr.
   */
  static bool is_red  (const node_t* nd) { return (nd != nullptr && nd->is_red()); }
  static bool is_black(const node_t* nd) { return (nd == nullptr || nd->is_black()); }

  /* 
   * paint() method so no need to write 'color' 
   * twice in situations like the following: 
   * node_t->color = node_t::color::BLACK
   * we can just write: 'node_t->paint(node_t::color::BLACK)' instead. 
   */
  void paint (enum color clr) { color = clr; }

  /* Set pointer to right child and set its parent to this. */
  node_t* tie_right(node_t* child) noexcept {

    node_t* prev = right;

    right_is_thread = false;
    right = child;

    if (right != nullptr) {
      right->set_parent(this);      
    }

    return prev;
  }

  /* Make thread to prev node. */
  using end_node::stitch_left;

  /* Make thread to next node. */
  node_t* stitch_right(end_node* nd) noexcept {

    node_t* prev = right;

    right_is_thread = true;
    right = static_cast<node_t*>(nd);
    
    return prev;
  }

  bool on_left() const {
    return (parent_ == nullptr)? false : this == parent_->get_left();
  }

  bool on_right() const {
    return (parent_ == nullptr)? false : this == parent()->get_right();
  }

  node_t* sibling() const {
    return (parent_ == nullptr)? nullptr : (on_left())? parent()->get_right() : parent_->get_left();
  }

  node_t* uncle() const {
    return (parent_ == nullptr)? nullptr : parent()->sibling();
  }

  /* Get smallest element in subtree. */
  static const node_t* get_leftmost_desc (const node_t* subtree_root);
  static node_t* get_leftmost_desc (node_t* subtree_root);
  
  /* Get greatest element in subtree. */
  static const node_t* get_rightmost_desc(const node_t* subtree_root);
  static node_t* get_rightmost_desc(node_t* subtree_root);

  static size_type subtree_size(const node_t* subtree_root) {
    return (subtree_root != nullptr)? subtree_root->size : 0;
  }

  /* Structure holding info about subtree: root, leftmost and rightmost nodes */
  struct subtree_info_t {

    const node_t* root;
    const end_node* leftmost;
    const end_node* rightmost;
    const end_node* end_node_ptr;
  };

  /* Structure holding info about newly made subtree copy. */
  struct subtree_copy_t {

    root_type root;
    end_node* leftmost  = nullptr;
    end_node* rightmost = nullptr;
  };

  /* Make a copy of subtree. */
  static void copy_subtree(subtree_copy_t& subtree_copy, const subtree_info_t& subtree_info);

  static void copy_subtree_impl(subtree_copy_t& subtree_copy, const subtree_info_t& subtree_info,
                                                              const node_t* subtree, node_t* copy);

  /* Stitch each node in subtree. */
  static void stitch_subtree(node_t* subtree) noexcept;

  /* Free given subtree. */
  static void free_subtree(node_t* subtree, const end_node* end_node_ptr) noexcept;

  /* Increase subtree size for each node in route from nd to root by 1. */
  static void incr_subtree_sizes(end_node* nd, const end_node* end_node_ptr);

  /* Decrease subtree size for each node in route from nd to root by 1. */
  static void decr_subtree_sizes(end_node* nd, const end_node* end_node_ptr);

  /* Get previous node. */
  const end_node* get_prev() const noexcept;
  end_node* get_prev() noexcept;

  /* Get next node. */
  const end_node* get_next() const noexcept;
  end_node* get_next() noexcept;

  /* 
   * Make stitches to next and previous nodes.
   * Stitches are only made if pointer to left or right elements 
   * accrodingly is nullptr - no such descnedant. 
   */
  void stitch() noexcept;

  /* Validate node. Call function below. */
  bool debug_validate() const;
  
  /* Checks red-black tree properties of node. */
  bool debug_validate_rb() const;

  /* Checks subtree sizes of tree. */
  bool debug_validate_size() const;

  /* Helper functions used for graphical dump of the tree. */
  template <typename CharT>
  void write_dot(std::basic_ostream<CharT>& os) const;

  template <typename CharT>
  static void write_nill_dot(std::basic_ostream<CharT>& os, uintptr_t node_num);
  
  template <typename CharT>
  static void write_pastend_dot(std::basic_ostream<CharT>& os, uintptr_t node_num);
};

template <typename Key>
void node_t<Key>::copy_subtree(subtree_copy_t& subtree_copy, const subtree_info_t& subtree_info) {

  if (subtree_info.root == nullptr) {
    return;
  }

  const node_t* subtree = subtree_info.root;

  node_t* copy = new node_t(*subtree), *child;
  subtree_copy.root.set(copy);
  
  copy_subtree_impl(subtree_copy, subtree_info, subtree, copy);
}

template <typename Key>
void node_t<Key>::copy_subtree_impl(subtree_copy_t& subtree_copy, const subtree_info_t& subtree_info,
                                                                  const node_t* subtree, node_t* copy) {

  end_node *parent;

  do {

    if (subtree->has_left() && !copy->has_left()) {

      subtree = subtree->get_left_unsafe();
      copy->tie_left(new node_t(*subtree));
      copy = copy->get_left_unsafe();

    } else if (subtree->has_right() && !copy->has_right()) {

      subtree = subtree->get_right_unsafe();
      copy->tie_right(new node_t(*subtree));      
      copy = copy->get_right_unsafe();

    } else {

      if (subtree == subtree_info.leftmost) {
        subtree_copy.leftmost = copy;
      }

      if (subtree == subtree_info.rightmost) {
        subtree_copy.rightmost = copy;
      }

      copy = copy->parent();
      parent = subtree->parent_as_end();
      subtree = subtree->parent();
    }

  } while (parent != subtree_info.end_node_ptr);
}

template <typename Key>
void node_t<Key>::stitch_subtree(node_t* subtree) noexcept {

  std::stack<node_t*> stack;

  while (subtree != nullptr || !stack.empty()) {

    if (!stack.empty()) {
      subtree = stack.top();
      stack.pop();
    }

    while (subtree != nullptr) {

      subtree->stitch();

      auto right = subtree->get_right();
      if (right != nullptr) {
        stack.push(right);
      }

      subtree = subtree->get_left();
    }
  }
}

template <typename Key>
void node_t<Key>::free_subtree(node_t* subtree, const end_node* end_node_ptr) noexcept {

  if (subtree == nullptr) {
    return;
  }

  end_node* parent;

  do {

    if (subtree->has_left()) {

      subtree = subtree->get_left_unsafe();
      continue;

    } else if (subtree->has_right()) {

      subtree = subtree->get_right_unsafe();
      continue;

    } else {

      node_t* deleting = subtree;
      parent = subtree->parent_as_end();
      subtree = subtree->parent();

      if (deleting->on_left()) {
        subtree->set_left(nullptr);

      } else {
        subtree->set_right(nullptr);
      }

      delete deleting;
    }

  } while (parent != end_node_ptr);
}

template <typename Key>
void node_t<Key>::incr_subtree_sizes(end_node* nd, const end_node* end_node_ptr) {

  if (nd == nullptr) {
    return;
  }

  node_t* cur;
  while (nd != end_node_ptr) {

    cur = static_cast<node_t*>(nd);
    ++cur->size;
    nd = cur->parent_as_end();
  }
}

template <typename Key>
void node_t<Key>::decr_subtree_sizes(end_node* nd, const end_node* end_node_ptr) {
  
  if (nd == nullptr) {
    return;
  }

  node_t* cur;
  while (nd != end_node_ptr) {

    cur = static_cast<node_t*>(nd);
    --cur->size;
    nd = cur->parent_as_end();
  }
}

template <typename Key>
node_t<Key>* node_t<Key>::get_leftmost_desc(node_t* cur) {

  while (cur != nullptr && cur->has_left()) {
    cur = cur->get_left();
  }

  return cur;
}

template <typename Key>
const node_t<Key>* node_t<Key>::get_leftmost_desc(const node_t* cur) {

  while (cur != nullptr && cur->has_left()) {
    cur = cur->get_left();
  }

  return cur;
}

template <typename Key>
node_t<Key>* node_t<Key>::get_rightmost_desc(node_t* cur) {

  while (cur != nullptr && cur->has_right()) {
    cur = cur->get_right();
  }

  return cur;
}

template <typename Key>
const node_t<Key>* node_t<Key>::get_rightmost_desc(const node_t* cur) {

  while (cur != nullptr && cur->has_right()) {
    cur = cur->get_right();
  }

  return cur;
}

template <typename Key>
const typename node_t<Key>::end_node* 
node_t<Key>::get_prev() const noexcept {

  if (has_left()) {
    return node_t::get_rightmost_desc(left);

  } else {

    const node_t* prev = this;
    const end_node* node_ptr = parent_as_end();

    while (node_ptr->parent_as_end() != nullptr) {

      auto nd = static_cast<const node_t*>(node_ptr);

      if (prev == nd->right) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return node_ptr;
  }
}

template <typename Key>
typename node_t<Key>::end_node* 
node_t<Key>::get_prev() noexcept {

  if (has_left()) {
    return node_t::get_rightmost_desc(left);

  } else {

    node_t* prev = this;
    end_node* node_ptr = parent_as_end();

    while (node_ptr->parent_as_end() != nullptr) {

      auto nd = static_cast<node_t*>(node_ptr);

      if (prev == nd->right) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return node_ptr;
  }
}

template <typename Key>
const typename node_t<Key>::end_node* 
node_t<Key>::get_next() const noexcept {

  if (has_right()) {
    return node_t::get_leftmost_desc(right);

  } else {

    const node_t* prev = this;
    const end_node* node_ptr = parent_as_end();

    while (node_ptr->parent_as_end() != nullptr) {

      auto nd = static_cast<const node_t*>(node_ptr);

      if (prev == nd->left) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return node_ptr;
  }
}

template <typename Key>
typename node_t<Key>::end_node* 
node_t<Key>::get_next() noexcept {

  if (has_right()) {
    return node_t::get_leftmost_desc(right);

  } else {

    node_t* prev = this;
    end_node* node_ptr = parent_as_end();

    while (node_ptr->parent_as_end() != nullptr) {

      auto nd = static_cast<node_t*>(node_ptr);

      if (prev == nd->left) {
        return nd;
      }

      prev = nd;
      node_ptr = nd->parent();  
    }

    return node_ptr;
  }
}

template <typename Key>
void node_t<Key>::stitch() noexcept {

  if (!has_left()) {
    stitch_left(get_prev());
  }

  if (!has_right()) {
    stitch_right(get_next());
  } 
}

template <typename Key>
bool node_t<Key>::debug_validate_rb() const {

  if (is_black()) {
    return true;
  }

  bool res = true;

  if (has_left() && left->color != color::BLACK) {

    std::cerr << "Debug validation:"
              << " left descendant " << left 
              << " of a red node " << this
              << " is not black. \n";
    res = false;
  }

  if (has_right() && right->color != color::BLACK) {
    
    std::cerr << "Debug validation:"
              << " right descendant " << right
              << " of a red node " << this
              << " is not black."
              << std::endl;
    res = false;
  }

  return res;
}

template <typename Key>
bool node_t<Key>::debug_validate_size() const {

  size_t sz = 0;

  bool has_l = has_left();
  if (has_l) {
    sz += left->size;
  }

  bool has_r = has_right();
  if (has_r) {
    sz += right->size;
  }

  if (sz + 1 != size) {
    std::cerr << "Debug validation: invalid subtree sizes." 
              << " Size of node " << this << " is " << size;
      
    if (has_l) {
      std::cerr << " Size of left descendant " << left 
                << " is " << left->size;
    }

    if (has_r) {
      std::cerr << " Size of right descendant " << right 
                << " is " << right->size;
    }

    std::cerr << std::endl;
  } 

  return true;
}

template <typename Key>
bool node_t<Key>::debug_validate() const {

  auto rb_res   = debug_validate_rb();
  auto size_res = debug_validate_size();

  return (rb_res && size_res);
}

/* Write node desctiption in dot format to temporary text file. */
template <typename Key>
  template <typename CharT>
  void DETAIL::node_t<Key>::write_dot(std::basic_ostream<CharT>& os) const {

    os << "NODE" << this << " ["
       << " label = < " << value << " <BR /> "
       << " <FONT POINT-SIZE=\"10\"> size: " << size << " </FONT> <BR /> "
       << " <FONT POINT-SIZE=\"10\"> addr: " << (void*) this << " </FONT>> "
       << " color = \"" << (is_red()? "#FD0000" : "#000000") << "\""
       << " fontcolor = \"" << (is_black()? "#FFFFFF" : "#000000") << "\""
       << " ]; \n";

    os << "NODE" << this << " -> "
       << "NODE" << parent_ << " ["
       << " style = \"dashed\""
       << " label = \"P\" ]; \n";

    const void *l, *r; 

    if (!has_left()) {

      write_nill_dot(os, reinterpret_cast<uintptr_t>(&left));
      l = reinterpret_cast<const void*>(&left);

    } else {
      l = reinterpret_cast<const void*>(left);
    }

    if (!has_right()) {

      write_nill_dot(os, reinterpret_cast<uintptr_t>(&right));
      r = reinterpret_cast<const void*>(&right);

    } else {
      r = reinterpret_cast<const void*>(right);
    }

    os << "NODE" << this << " -> "
       << "NODE" << l << " [ label = \"L\" ]; \n";

    os << "NODE" << this << " -> "
       << "NODE" << r << " [ label = \"R\" ]; \n";

    if (left_is_thread) {

      os << "NODE" << this << " -> "
         << "NODE" << left
         << " [ label = \"PREV\" style = \"dotted\" " 
         << " fontcolor = \"#a3a3c2\" color = \"#a3a3c2\" ]; \n"; 
    }

    if (right_is_thread) {

      os << "NODE" << this << " -> "
         << "NODE" << right
         << " [ label = \"NEXT\" style = \"dotted\" " 
         << " fontcolor = \"#a3a3c2\" color = \"#a3a3c2\" ]; \n"; 
    }
  }

/* Helper function to add nill nodes. */
template <typename Key>
  template <typename CharT>
  void DETAIL::node_t<Key>::write_nill_dot(std::basic_ostream<CharT>& os, uintptr_t node_num) {

    os << "NODE" << std::hex << std::showbase << node_num << std::dec << " ["
       << " label = \"nill\" color = \"#000000\" width=0.1" 
       << " fontcolor = \"#FFFFFF\" fontsize = \"10\" shape = \"oval\" ]; \n";
  }

/* Helper function to add past-end node. */
template <typename Key>
  template <typename CharT>
  void DETAIL::node_t<Key>::write_pastend_dot(std::basic_ostream<CharT>& os, uintptr_t node_num) {

    os << "NODE" << std::hex << std::showbase << node_num << std::dec << " ["
       << " label = \"PAST-END\" color = \"#00FFFF\" width=0.1" 
       << " fontcolor = \"#000000\" fontsize = \"10\" shape = \"diamond\" ]; \n";
  }

}; /* namespace DETAIL */

}; /* namespace RBTREE */