#include <algorithm>
#include <memory>

class TruncatedNode {
 public:
  TruncatedNode* next;
  TruncatedNode* prev;

  TruncatedNode() : next(this), prev(this) {}

  TruncatedNode(TruncatedNode* next, TruncatedNode* prev)
      : next(next), prev(prev) {}

  TruncatedNode& operator=(const TruncatedNode& other) {
    next = other.next;
    prev = other.prev;
    return *this;
  }

  ~TruncatedNode() = default;
};

template <typename T>
class Node : public TruncatedNode {
 private:
  T val_;

 public:
  Node() {}

  Node(const T& val) : val_(val), TruncatedNode() {}

  Node(const T& val, TruncatedNode* next, TruncatedNode* prev)
      : val_(val), TruncatedNode(next, prev) {}

  Node(const Node& other)
      : val_(other.val), TruncatedNode(other.next, other.prev) {}

  Node& operator=(const Node& other) {
    val_ = other.val_;
    this->next = other.next;
    this->prev = other.prev;
    return *this;
  }

  Node& operator=(const TruncatedNode& other) {
    this->next = other.next;
    this->prev = other.prev;
    return *this;
  }

  ~Node() = default;

  TruncatedNode* get_next() { return this->next; }

  TruncatedNode* get_prev() { return this->prev; }

  T& get_val() { return val_; }
};

template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  TruncatedNode initial_node_;
  size_t size_ = 0;

  Alloc list_alloc_;
  using alloc_traits = std::allocator_traits<Alloc>;
  typename alloc_traits::template rebind_alloc<Node<T>> node_alloc_;
  using node_alloc_traits =
      typename alloc_traits::template rebind_traits<Node<T>>;

 public:
  template <bool IsConst, bool IsReversed>
  class Iterator;

  using value_type = T;
  using allocator_type = Alloc;
  using iterator = Iterator<false, false>;
  using const_iterator = Iterator<true, false>;
  using reverse_iterator = Iterator<false, true>;
  using const_reverse_iterator = Iterator<true, true>;

  List() = default;

  void full_destroy(size_t upper_lim) {
    auto cur = static_cast<Node<T>*>(initial_node_.next->next);

    for (size_t i = 0; i < upper_lim; i++) {
      node_alloc_traits::destroy(node_alloc_, static_cast<Node<T>*>(cur->prev));
      node_alloc_traits::deallocate(node_alloc_,
                                    static_cast<Node<T>*>(cur->prev), 1);
      cur = static_cast<Node<T>*>(cur->next);
    }
  }

  List(size_t count, const T& value, const Alloc& alloc = Alloc())
      : list_alloc_(alloc) {
    Node<T>* cur = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, cur, value);
      size_++;

      initial_node_.next = cur;
      cur->prev = static_cast<Node<T>*>(&initial_node_);

      for (size_t i = 0; i < count - 1; i++, size_++) {
        cur->next = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(node_alloc_,
                                     static_cast<Node<T>*>(cur->next), value);
        cur->next->prev = cur;
        cur = static_cast<Node<T>*>(cur->next);
      }
    } catch (...) {
      full_destroy(size_ - 1);
      node_alloc_traits::deallocate(node_alloc_,
                                    static_cast<Node<T>*>(cur->next), 1);
      node_alloc_traits::destroy(node_alloc_, cur);
      node_alloc_traits::deallocate(node_alloc_, cur, 1);
      throw 1;
    }

    cur->next = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev = cur;
  }

  explicit List(size_t count, const Alloc& alloc = Alloc())
      : list_alloc_(alloc) {
    Node<T>* cur = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, cur);
      size_++;

      initial_node_.next = cur;
      cur->prev = static_cast<Node<T>*>(&initial_node_);

      for (size_t i = 0; i < count - 1; i++, size_++) {
        cur->next = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(node_alloc_,
                                     static_cast<Node<T>*>(cur->next));
        cur->next->prev = cur;
        cur = static_cast<Node<T>*>(cur->next);
      }
    } catch (...) {
      full_destroy(size_ - 1);
      node_alloc_traits::deallocate(node_alloc_,
                                    static_cast<Node<T>*>(cur->next), 1);
      node_alloc_traits::destroy(node_alloc_, cur);
      node_alloc_traits::deallocate(node_alloc_, cur, 1);

      throw 1;
    }

    cur->next = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev = cur;
  }

  List(const List<T, Alloc>& other) {
    auto beg = other.begin();
    auto end = other.end();

    Node<T>* cur = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, cur, *beg);
      size_++;

      initial_node_.next = cur;
      cur->prev = static_cast<Node<T>*>(&initial_node_);

      ++beg;
      for (; beg != end; ++beg, size_++) {
        cur->next = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(node_alloc_,
                                     static_cast<Node<T>*>(cur->next), *beg);
        cur->next->prev = cur;

        cur = static_cast<Node<T>*>(cur->next);
      }

      cur->next = static_cast<Node<T>*>(&initial_node_);
      initial_node_.prev = cur;
    } catch (...) {
      full_destroy(size_ - 1);
      node_alloc_traits::deallocate(node_alloc_,
                                    static_cast<Node<T>*>(cur->next), 1);
      node_alloc_traits::destroy(node_alloc_, cur);
      node_alloc_traits::deallocate(node_alloc_, cur, 1);
      throw 1;
    }

    list_alloc_ =
        alloc_traits::select_on_container_copy_construction(other.list_alloc_);
    node_alloc_ =
        alloc_traits::select_on_container_copy_construction(other.node_alloc_);
  }

  List(std::initializer_list<T> init, const Alloc& alloc = Alloc())
      : list_alloc_(alloc) {
    auto iter = init.begin();
    auto init_end = init.end();

    Node<T>* cur = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, cur, *iter);
      size_++;

      initial_node_.next = cur;
      cur->prev = static_cast<Node<T>*>(&initial_node_);

      iter++;
      for (; iter < init_end; iter++, size_++) {
        cur->next = node_alloc_traits::allocate(node_alloc_, 1);
        node_alloc_traits::construct(node_alloc_,
                                     static_cast<Node<T>*>(cur->next), *iter);
        cur->next->prev = cur;

        cur = static_cast<Node<T>*>(cur->next);
      }

      cur->next = static_cast<Node<T>*>(&initial_node_);
      initial_node_.prev = cur;
    } catch (...) {
      full_destroy(size_ - 1);
      node_alloc_traits::deallocate(node_alloc_,
                                    static_cast<Node<T>*>(cur->next), 1);
      node_alloc_traits::destroy(node_alloc_, cur);
      node_alloc_traits::deallocate(node_alloc_, cur, 1);
      throw 1;
    }
  }

  List& operator=(const List<T, Alloc>& other) {
    List<T, Alloc> temp(other);

    std::swap(size_, temp.size_);
    std::swap(initial_node_, temp.initial_node_);

    initial_node_.next->prev = &initial_node_;
    initial_node_.prev->next = &initial_node_;

    if (temp.size_ == 0) {
      temp.initial_node_.next = &temp.initial_node_;
      temp.initial_node_.prev = &temp.initial_node_;
    } else {
      temp.initial_node_.next->prev = &temp.initial_node_;
      temp.initial_node_.prev->next = &temp.initial_node_;
    }

    if (node_alloc_traits::propagate_on_container_copy_assignment::value &&
        list_alloc_ != other.list_alloc_) {
      list_alloc_ = other.list_alloc_;
    }

    return *this;
  }

  size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  ~List() { full_destroy(size_); }

  Alloc get_allocator() const { return list_alloc_; }

  T& front() { return static_cast<Node<T>>(initial_node_.next).get_val(); }

  const T& front() const {
    return static_cast<Node<T>>(initial_node_.next).get_val();
  }

  T& back() { return static_cast<Node<T>>(initial_node_.prev).get_val(); }

  const T& back() const {
    return static_cast<Node<T>>(initial_node_.prev).get_val();
  }

  void push_back(const T& val) {
    try {
      push_back(std::move(val));
    } catch (int) {
      throw 1;
    }
  }

  void push_front(const T& val) {
    try {
      push_front(std::move(val));
    } catch (int) {
      throw 1;
    }
  }

  void push_back(T&& val) {
    Node<T>* node_to_push = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, node_to_push, val);
    } catch (...) {
      node_alloc_traits::deallocate(node_alloc_, node_to_push, 1);
    }

    node_to_push->prev = static_cast<Node<T>*>(initial_node_.prev);
    initial_node_.prev->next = node_to_push;

    node_to_push->next = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev = node_to_push;

    size_++;
  }

  void push_front(T&& val) {
    Node<T>* node_to_push = node_alloc_traits::allocate(node_alloc_, 1);

    try {
      node_alloc_traits::construct(node_alloc_, node_to_push, val);
    } catch (...) {
      node_alloc_traits::deallocate(node_alloc_, node_to_push, 1);
    }

    node_to_push->next = static_cast<Node<T>*>(initial_node_.next);
    initial_node_.next->prev = node_to_push;

    node_to_push->prev = static_cast<Node<T>*>(&initial_node_);
    initial_node_.next = node_to_push;

    size_++;
  }

  void pop_back() noexcept {
    initial_node_.prev = initial_node_.prev->prev;

    node_alloc_traits::destroy(node_alloc_,
                               static_cast<Node<T>*>(initial_node_.prev->next));
    node_alloc_traits::deallocate(
        node_alloc_, static_cast<Node<T>*>(initial_node_.prev->next), 1);

    initial_node_.prev->next = &initial_node_;

    size_--;
  }

  void pop_front() noexcept {
    initial_node_.next = initial_node_.next->next;

    node_alloc_traits::destroy(node_alloc_,
                               static_cast<Node<T>*>(initial_node_.next->prev));
    node_alloc_traits::deallocate(
        node_alloc_, static_cast<Node<T>*>(initial_node_.next->prev), 1);

    initial_node_.next->prev = &initial_node_;

    size_--;
  }

  iterator begin() { return iterator(initial_node_.next); }

  iterator end() {
    return iterator(const_cast<TruncatedNode*>(&initial_node_));
  }

  const_iterator begin() const { return const_iterator(initial_node_.next); }

  const_iterator end() const {
    return const_iterator(const_cast<TruncatedNode*>(&initial_node_));
  }

  const_iterator cbegin() const { return const_iterator(initial_node_.next); }

  const_iterator cend() const {
    return const_iterator(const_cast<TruncatedNode*>(&initial_node_));
  }

  reverse_iterator rbegin() { return reverse_iterator(initial_node_.prev); }

  reverse_iterator rend() {
    return reverse_iterator(const_cast<TruncatedNode*>(&initial_node_));
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(initial_node_.prev);
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(const_cast<TruncatedNode*>(&initial_node_));
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(initial_node_.prev);
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(const_cast<TruncatedNode*>(&initial_node_));
  }
};

template <typename T, typename Alloc>
template <bool IsConst, bool IsReversed>
class List<T, Alloc>::Iterator {
 private:
  Node<T>* cur_node_;

 public:
  using is_const = std::conditional_t<IsConst, const T, T>;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = std::remove_cv_t<T>;
  using difference_type = std::ptrdiff_t;
  using pointer = is_const*;
  using reference = is_const&;

  Iterator() = default;

  Iterator(TruncatedNode* node) : cur_node_(static_cast<Node<T>*>(node)) {}

  ~Iterator() = default;

  Iterator& operator++() {
    if (IsReversed) {
      cur_node_ = static_cast<Node<T>*>(cur_node_->prev);
    } else {
      cur_node_ = static_cast<Node<T>*>(cur_node_->next);
    }
    return *this;
  }

  Iterator operator++(int) {
    auto temp(*this);
    ++*this;
    return temp;
  }

  Iterator& operator--() {
    if (IsReversed) {
      cur_node_ = static_cast<Node<T>*>(cur_node_->next);
    } else {
      cur_node_ = static_cast<Node<T>*>(cur_node_->prev);
    }
    return *this;
  }

  Iterator operator--(int) {
    auto temp(*this);
    --*this;
    return temp;
  }

  reference operator*() const { return cur_node_->get_val(); }

  pointer operator->() const { return &(cur_node_->get_val()); }

  bool operator==(const Iterator<IsConst, IsReversed>& other) const {
    return cur_node_ == other.cur_node_;
  }

  bool operator!=(const Iterator<IsConst, IsReversed>& other) const {
    return cur_node_ != other.cur_node_;
  }
};