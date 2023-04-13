#include <memory>
#include <algorithm>


class TruncatedNode {
 public:
  TruncatedNode* next_;
  TruncatedNode* prev_;

  TruncatedNode() : next_(this), prev_(this) {}

  TruncatedNode(TruncatedNode* next, TruncatedNode* prev): next_(next), prev_(prev) {}

  TruncatedNode& operator=(const TruncatedNode& other) {
    next_ = other.next_;
    prev_ = other.prev_;
    return *this;
  }

  ~TruncatedNode() = default;
};

template<typename T>
class Node: public TruncatedNode {
 private:
  T val_;

 public:
  Node() {}

  Node(const T& val): val_(val), TruncatedNode() {}

  Node(const T& val, TruncatedNode* next, TruncatedNode* prev) : val_(val), TruncatedNode(next, prev) {}

  Node(const Node& other) : val_(other.val), TruncatedNode(other.next_, other.prev_) {}

  Node& operator=(const Node& other) {
    val_ = other.val_;
    this->next_ = other.next_;
    this->prev_ = other.prev_;
    return *this;
  }

  Node& operator=(const TruncatedNode other) {
    this->next_ = other.next_;
    this->prev_ = other.prev_;
    return *this;
  }

  ~Node() = default;

  TruncatedNode* get_next() {
    return this->next_;
  }

  TruncatedNode* get_prev() {
    return this->prev_;
  }

  T& get_val() {
    return val_;
  }
};

template<typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  TruncatedNode initial_node_;
  size_t size_;

  Alloc list_alloc_;
  using AllocTraits = std::allocator_traits<Alloc>;
  typename AllocTraits::template rebind_alloc<Node<T>> node_alloc_;
  using NodeAllocTraits = typename AllocTraits::template rebind_traits<Node<T>>;

 public:
  template<bool IsConst, bool IsReversed>
  class Iterator;

  using value_type = T;
  using allocator_type = Alloc;
  using iterator = Iterator<false, false>;
  using const_iterator = Iterator<true, false>;
  using reverse_iterator = Iterator<false, true>;
  using const_reverse_iterator = Iterator<true, true>;


  List() : size_(0) {}

  List(size_t count, const T& value, const Alloc& alloc = Alloc()) : size_(count) {
    Node<T>* cur = NodeAllocTraits::allocate(node_alloc_, 1);
    NodeAllocTraits::construct(node_alloc_, cur, value);
    initial_node_.next_ = cur;
    cur->prev_ = static_cast<Node<T>*>(&initial_node_);

    for (size_t i = 0; i < count - 1; i++) {
      cur->next_ = NodeAllocTraits::allocate(node_alloc_, 1);
      NodeAllocTraits::construct(node_alloc_, static_cast<Node<T>*>(cur->next_), value);
      cur->next_->prev_ = cur;

      cur = static_cast<Node<T>*>(cur->next_);
    }

    cur->next_ = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev_ = cur;
  }

  explicit List(size_t count, const Alloc& alloc = Alloc()): size_(count) {
    Node<T>* cur = NodeAllocTraits::allocate(node_alloc_, 1);
    NodeAllocTraits::construct(node_alloc_, cur);
    initial_node_.next_ = cur;
    cur->prev_ = static_cast<Node<T>*>(&initial_node_);

    for (size_t i = 0; i < count - 1; i++) {
      cur->next_ = NodeAllocTraits::allocate(node_alloc_, 1);
      NodeAllocTraits::construct(node_alloc_, static_cast<Node<T>*>(cur->next_));
      cur->next_->prev_ = cur;

      cur = static_cast<Node<T>*>(cur->next_);
    }

    cur->next_ = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev_ = cur;
  }

  List(const List<T, Alloc>& other) : size_(other.size_) {
    auto beg = other.begin();
    auto end = other.end();

    Node<T>* cur = NodeAllocTraits::allocate(node_alloc_, 1);
    NodeAllocTraits::construct(node_alloc_, cur, *beg);
    initial_node_.next_ = cur;
    cur->prev_ = static_cast<Node<T>*>(&initial_node_);

    ++beg;
    for (; beg != end; ++beg) {
      cur->next_ = NodeAllocTraits::allocate(node_alloc_, 1);
      NodeAllocTraits::construct(node_alloc_, static_cast<Node<T>*>(cur->next_), *beg);
      cur->next_->prev_ = cur;

      cur = static_cast<Node<T>*>(cur->next_);
    }

    cur->next_ = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev_ = cur;

    list_alloc_ = AllocTraits::select_on_container_copy_construction(other.list_alloc_);
    node_alloc_ = AllocTraits::select_on_container_copy_construction(other.node_alloc_);
  }

  List(std::initializer_list<T> init, const Alloc& alloc = Alloc()): size_(init.size()) {
    auto iter = init.begin();
    auto init_end = init.end();
    Node<T>* cur = NodeAllocTraits::allocate(node_alloc_, 1);
    NodeAllocTraits::construct(node_alloc_, cur, *iter);
    initial_node_.next_ = cur;
    cur->prev_ = static_cast<Node<T>*>(&initial_node_);

    iter++;
    for (; iter < init_end; iter++) {
      cur->next_ = NodeAllocTraits::allocate(node_alloc_, 1);
      NodeAllocTraits::construct(node_alloc_, static_cast<Node<T>*>(cur->next_), *iter);
      cur->next_->prev_ = cur;

      cur = static_cast<Node<T>*>(cur->next_);
    }

    cur->next_ = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev_ = cur;
  }

  List& operator=(const List<T, Alloc>& other) {
//    std::cout << "Other node: " << &other.initial_node_ << '\n';

    List<T, Alloc> temp(other);

//    std::cout << "Before swap\nTemp: " << temp << "\nthis: " << *this << '\n';

    std::swap(size_, temp.size_);

//    std::cout << "My node: " << &initial_node_ << " initial node of son of mother's friend: " << &(temp.initial_node_) << '\n';

    std::swap(initial_node_, temp.initial_node_);

//    std::cout << "References after swap: " << &initial_node_ << " || " << initial_node_.next_->prev_ << " || " << initial_node_.prev_->next_ << '\n';
//
//    std::cout << "After swap\nTemp: " << temp << "\nthis: " << *this << '\n';

//    std::cout << "My node: " << &initial_node_ << " initial node of son of mother's friend: " << &(temp.initial_node_) << '\n';

    initial_node_.next_->prev_ = &initial_node_;
    initial_node_.prev_->next_ = &initial_node_;

//    std::cout << "After rebinding 1\nTemp: " << temp << "\nthis: " << *this << '\n';

    if (temp.size_ == 0) {
      temp.initial_node_.next_ = &temp.initial_node_;
      temp.initial_node_.prev_ = &temp.initial_node_;
    } else {
      temp.initial_node_.next_->prev_ = &temp.initial_node_;
      temp.initial_node_.prev_->next_ = &temp.initial_node_;
    }

//    std::cout << "After rebinding 2\nTemp: " << temp << "\nthis: " << *this << '\n';

    if (NodeAllocTraits::propagate_on_container_copy_assignment::value && list_alloc_ != other.list_alloc_) {
      list_alloc_ = other.list_alloc_;
    }

    return *this;
  }

  size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  ~List() {
    // std::cout << "Ya dolboeb!\n";

    auto cur = static_cast<Node<T>*>(initial_node_.next_->next_);

    for (size_t i = 0; i < size_; i++) {
      NodeAllocTraits::destroy(node_alloc_, static_cast<Node<T>*>(cur->prev_));
      NodeAllocTraits::deallocate(node_alloc_, static_cast<Node<T>*>(cur->prev_), 1);
      cur = static_cast<Node<T>*>(cur->next_);
    }
  }

  Alloc get_allocator() const {
    return list_alloc_;
  }

  T& front() {
    return static_cast<Node<T>>(initial_node_.next_).get_val();
  }

  const T& front() const {
    return static_cast<Node<T>>(initial_node_.next_).get_val();
  }

  T& back() {
    return static_cast<Node<T>>(initial_node_.prev_).get_val();
  }

  const T& back() const {
    return static_cast<Node<T>>(initial_node_.prev_).get_val();
  }

  void push_back(const T& val) {
    push_back(std::move(val));
  }

  void push_front(const T& val) {
    push_front(std::move(val));
  }

  void push_back(T&& val) {
    Node<T>* node_to_push = NodeAllocTraits::allocate(node_alloc_, 1);
    NodeAllocTraits::construct(node_alloc_, node_to_push, val);

    node_to_push->prev_ = static_cast<Node<T>*>(initial_node_.prev_);
    initial_node_.prev_->next_ = node_to_push;

    node_to_push->next_ = static_cast<Node<T>*>(&initial_node_);
    initial_node_.prev_ = node_to_push;

    size_++;
  }

  void push_front(T&& val) {
    Node<T>* node_to_push = NodeAllocTraits::allocate(node_alloc_, 1);
    NodeAllocTraits::construct(node_alloc_, node_to_push, val);

    node_to_push->next_ = static_cast<Node<T>*>(initial_node_.next_);
    initial_node_.next_->prev_ = node_to_push;

    node_to_push->prev_ = static_cast<Node<T>*>(&initial_node_);
    initial_node_.next_ = node_to_push;

    size_++;
  }

  void pop_back() {
    initial_node_.prev_ = initial_node_.prev_->prev_;

    NodeAllocTraits::destroy(node_alloc_, static_cast<Node<T>*>(initial_node_.prev_->next_));
    NodeAllocTraits::deallocate(node_alloc_, static_cast<Node<T>*>(initial_node_.prev_->next_), 1);

    initial_node_.prev_->next_ = &initial_node_;

    size_--;
  }

  void pop_front() {
    initial_node_.next_ = initial_node_.next_->next_;

    NodeAllocTraits::destroy(node_alloc_, static_cast<Node<T>*>(initial_node_.next_->prev_));
    NodeAllocTraits::deallocate(node_alloc_, static_cast<Node<T>*>(initial_node_.next_->prev_), 1);

    initial_node_.next_->prev_ = &initial_node_;

    size_--;
  }

  iterator begin() { return iterator(initial_node_.next_); }

  iterator end() { return iterator(const_cast<TruncatedNode*>(&initial_node_)); }

  const_iterator begin() const { return const_iterator(initial_node_.next_); }

  const_iterator end() const { return const_iterator(const_cast<TruncatedNode*>(&initial_node_)); }

  const_iterator cbegin() const { return const_iterator(initial_node_.next_); }

  const_iterator cend() const { return const_iterator(const_cast<TruncatedNode*>(&initial_node_)); }

  reverse_iterator rbegin() { return reverse_iterator(initial_node_.prev_); }

  reverse_iterator rend() { return reverse_iterator(const_cast<TruncatedNode*>(&initial_node_)); }

  const_reverse_iterator rbegin() const { return const_reverse_iterator(initial_node_.prev_); }

  const_reverse_iterator rend() const { return const_reverse_iterator(const_cast<TruncatedNode*>(&initial_node_)); }

  const_reverse_iterator crbegin() const { return const_reverse_iterator(initial_node_.prev_); }

  const_reverse_iterator crend() const { return const_reverse_iterator(const_cast<TruncatedNode*>(&initial_node_)); }

  friend std::ostream& operator<<(std::ostream& os, const List<T, Alloc>& lst) {
    os << "i: " << &lst.initial_node_ << " || " << lst.initial_node_.next_ << " || " << lst.initial_node_.prev_ << '\n';
    auto start = (Node<T>*)lst.initial_node_.next_;
    for (int i = 0; i < lst.size(); i++) {
      os << i << ": " << start << " || " << start->get_val() << " :: " << start->get_next() << " :: " << start->get_prev() << '\n';
      start = (Node<T>*)start->next_;
    }
    return os;
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

  Iterator(TruncatedNode* node): cur_node_(static_cast<Node<T>*>(node)) {}

  Iterator& operator=(const Iterator& other) {
    cur_node_ = other.cur_node_;
  }

  ~Iterator() = default;

  Iterator& operator++() {
    if (IsReversed) {
      cur_node_ = static_cast<Node<T>*>(cur_node_->prev_);
    } else {
      cur_node_ = static_cast<Node<T>*>(cur_node_->next_);
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
      cur_node_ = static_cast<Node<T>*>(cur_node_->next_);
    } else {
      cur_node_ = static_cast<Node<T>*>(cur_node_->prev_);
    }
    return *this;
  }

  Iterator operator--(int) {
    auto temp(*this);
    --*this;
    return temp;
  }

  reference operator*() {
    return cur_node_->get_val();
  }

  pointer operator->() {
    return &(cur_node_->get_val());
  }

  bool operator==(Iterator other) {
    return cur_node_ == other.cur_node_;
  }

  bool operator!=(Iterator other) {
    return cur_node_ != other.cur_node_;
  }
};