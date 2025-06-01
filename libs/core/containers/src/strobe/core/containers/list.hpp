#pragma once

#include <concepts>

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"

namespace strobe {

template <typename T, Allocator A>
class List {
 private:
  using Self = List<T, A>;
  struct Node {
    T value;
    Node* next;
    Node* prev;
  };

  using Resource = MonotonicPoolResource<sizeof(Node), alignof(Node), A>;
  using ResourceTraits = AllocatorTraits<Resource>;

 public:
  using value_type = T;
  using allocator_type = A;
  using allocator_traits = A;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;

  class const_iterator;
  class iterator {
   public:
    using value_type = T;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

   private:
    friend Self;
    friend const_iterator;
    iterator(Node* node) : m_node(node) {}

   public:
    iterator() : m_node(nullptr) {}

    T& operator*() const {
      assert(m_node != nullptr);
      return m_node->value;
    }
    T* operator->() const {
      assert(m_node != nullptr);
      return &m_node->value;
    }

    iterator& operator++() {
      assert(m_node != nullptr);
      assert(m_node != m_node->next);
      Node* next = m_node->next;
      m_node = m_node->next;


      return *this;
    }
    iterator operator++(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    iterator& operator--() {
      assert(m_node != nullptr);
      assert(m_node != m_node->prev);
      m_node = m_node->prev;
      return *this;
    }
    iterator operator--(int) {
      iterator temp = *this;
      --(*this);
      return temp;
    }

    bool operator==(const iterator& other) const {
      return m_node == other.m_node;
    }
    bool operator!=(const iterator& other) const {
      return m_node != other.m_node;
    }

   private:
    Node* m_node;
  };

  class const_iterator {
   public:
    using value_type = T;
    using reference = const T&;
    using difference_type = std::ptrdiff_t;

   private:
    friend Self;
    const Node* m_node = nullptr;
    explicit const_iterator(const Node* node) : m_node(node) {}

   public:
    const_iterator(iterator it) : m_node(it.m_node) {}
    const_iterator() = default;
    const T& operator*() const { return m_node->value; }
    const T* operator->() const { return &m_node->value; }

    const_iterator& operator++() {
      m_node = m_node->next;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator temp = *this;
      ++(*this);
      return temp;
    }

    const_iterator& operator--() {
      m_node = m_node->prev;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator temp = *this;
      --(*this);
      return temp;
    }

    bool operator==(const const_iterator& other) const {
      return m_node == other.m_node;
    }
    bool operator!=(const const_iterator& other) const {
      return m_node != other.m_node;
    }
  };
  class const_reverse_iterator;
  class reverse_iterator {
   public:
    using value_type = T;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

   private:
    friend Self;
    friend const_reverse_iterator;
    reverse_iterator(Node* node) : m_node(node) {}

   public:
    reverse_iterator() : m_node(nullptr) {}

    T& operator*() const {
      assert(m_node != nullptr);
      return m_node->value;
    }
    T* operator->() const {
      assert(m_node != nullptr);
      return &m_node->value;
    }

    reverse_iterator& operator--() {
      assert(m_node != nullptr);
      assert(m_node != m_node->next);
      m_node = m_node->next;
      return *this;
    }
    reverse_iterator operator--(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    reverse_iterator& operator++() {
      assert(m_node != nullptr);
      assert(m_node != m_node->prev);
      m_node = m_node->prev;
      return *this;
    }
    reverse_iterator operator++(int) {
      iterator temp = *this;
      --(*this);
      return temp;
    }

    bool operator==(const reverse_iterator& other) const {
      return m_node == other.m_node;
    }
    bool operator!=(const reverse_iterator& other) const {
      return m_node != other.m_node;
    }

   private:
    Node* m_node;
  };
  class const_reverse_iterator {
   public:
    using value_type = T;
    using reference = T&;
    using difference_type = std::ptrdiff_t;

   private:
    friend Self;
    const_reverse_iterator(Node* node) : m_node(node) {}

   public:
    const_reverse_iterator() : m_node(nullptr) {}
    const_reverse_iterator(reverse_iterator it) : m_node(it.m_node) {}

    T& operator*() const {
      assert(m_node != nullptr);
      return m_node->value;
    }
    T* operator->() const {
      assert(m_node != nullptr);
      return &m_node->value;
    }

    const_reverse_iterator& operator--() {
      assert(m_node != nullptr);
      assert(m_node != m_node->next);
      m_node = m_node->next;
      return *this;
    }
    const_reverse_iterator operator--(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    const_reverse_iterator& operator++() {
      assert(m_node != nullptr);
      assert(m_node != m_node->prev);
      m_node = m_node->prev;
      return *this;
    }
    const_reverse_iterator operator++(int) {
      iterator temp = *this;
      --(*this);
      return temp;
    }

    bool operator==(const const_reverse_iterator& other) const {
      return m_node == other.m_node;
    }
    bool operator!=(const const_reverse_iterator& other) const {
      return m_node != other.m_node;
    }

   private:
    Node* m_node;
  };

  explicit List(const A& alloc = {})
      : m_resource(alloc), m_head(nullptr), m_tail(nullptr) {}

  List(const List& o)
      : m_resource(
            Resource::select_on_container_copy_construction(o.m_resource)),
        m_head(nullptr),
        m_tail(nullptr) {
    for (auto& x : o) {
      push_front(x);
    }
  }
  List& operator=(const List& o) {
    if (this == &o) {
      return *this;
    }
    clear();
    // Hacky only works because we know what resource is
    m_resource = std::move(
        ResourceTraits::select_on_container_copy_construction(o.m_resource));
    static_assert(
        std::same_as<Resource,
                     MonotonicPoolResource<sizeof(T), alignof(T), A>>);
    m_head = nullptr;
    m_tail = nullptr;
    for (auto& x : o) {
      push_front(x);
    }
    return *this;
  }
  List(List&& o)
      : m_resource(std::move(o.m_resource)),
        m_head(std::exchange(o.m_head, nullptr)),
        m_tail(std::exchange(o.m_tail, nullptr)) {}

  List& operator=(List&& o) {
    if (this == &o) {
      return *this;
    }
    static_assert(
        ResourceTraits::propagate_on_container_move_construction,
        "Just not implemented yet. MAybe this fails if A is non propagating");
    clear();
    m_resource = std::move(o.m_resource);
    m_head = o.m_head;
    m_tail = o.m_tail;
    return *this;
  }


  void push_back(const T& value) {
    Node* node = newNode(value);
    node->next = nullptr;
    if (!m_tail) {  // Empty list
      node->prev = nullptr;
      m_head = m_tail = node;
    } else {
      node->prev = m_tail;
      m_tail->next = node;
      m_tail = node;
    }
  }

  void push_back(T&& value) {
    Node* node = newNode(std::forward<T>(value));
    node->next = nullptr;
    if (empty()) {  // Empty list
      node->prev = nullptr;
      m_head = node;
      m_tail = node;
    } else {
      node->prev = m_tail;
      m_tail->next = node;
      m_tail = node;
    }
  }

  void push_front(T&& value) {
    Node* node = newNode(std::forward<T>(value));
    if (empty()) {  // Empty list
      node->next = nullptr;
      node->prev = nullptr;
      m_head = m_tail = node;
    } else {
      node->next = m_head;
      node->prev = nullptr;
      m_head->prev = node;
      m_head = node;
    }
  }

  void push_front(const T& value) {
    Node* node = newNode(value);
    if (empty()) {  // Empty list
      node->next = nullptr;
      node->prev = nullptr;
      m_head = m_tail = node;
    } else {
      node->next = m_head;
      node->prev = nullptr;
      m_head->prev = node;
      m_head = node;
    }
  }

  void pop_back() {
    if (!m_tail) return;
    Node* to_remove = m_tail;
    m_tail = m_tail->prev;
    if (m_tail)
      m_tail->next = nullptr;
    else
      m_head = nullptr;
    deleteNode(to_remove);
  }

  void pop_front() {
    if (!m_head) return;
    Node* to_remove = m_head;
    m_head = m_head->next;
    if (m_head)
      m_head->prev = nullptr;
    else
      m_tail = nullptr;
    deleteNode(to_remove);
  }

  iterator insert(const_iterator pos, T&& value) {
    Node* node = newNode(std::forward<T>(value));
    node->next = nullptr;
    node->prev = nullptr;

    if (pos.m_node == nullptr) {  // Insert at end
      if (empty()) {
        m_head = m_tail = node;
      } else {
        node->prev = m_tail;
        m_tail->next = node;
        m_tail = node;
      }
      return iterator(node);
    }

    Node* next = const_cast<Node*>(pos.m_node);
    assert(next != nullptr);
    Node* prev = next->prev;

    node->next = next;
    node->prev = prev;

    if (next != nullptr) {
      next->prev = node;
    }

    if (prev != nullptr) {
      prev->next = node;
    } else {
      m_head = node;  // Inserting at the front
    }

    return iterator(node);
  }

  iterator erase(iterator pos) {
    Node* to_remove = pos.m_node;
    assert(to_remove);  // Safety check

    Node* next = to_remove->next;
    Node* prev = to_remove->prev;

    if (prev) {
      prev->next = next;
    } else {
      m_head = next;
    }

    if (next) {
      next->prev = prev;
    } else {
      m_tail = prev;
    }

    deleteNode(to_remove);
    return iterator(next);
  }

  iterator begin() { return iterator(m_head); }
  iterator end() { return iterator(nullptr); }
  const_iterator begin() const { return const_iterator(m_head); }
  const_iterator end() const { return const_iterator(nullptr); }
  const_iterator cbegin() const { return const_iterator(m_head); }
  const_iterator cend() const { return const_iterator(nullptr); }

  // Reverse iterators
  reverse_iterator rbegin() { return reverse_iterator(m_tail); }
  reverse_iterator rend() { return reverse_iterator(nullptr); }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(const_reverse_iterator(m_tail));
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(const_reverse_iterator(nullptr));
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(m_tail);
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(nullptr);
  }

  reference front() {
    assert(m_head);
    return m_head->value;
  }

  const_reference front() const {
    assert(m_head);
    return m_head->value;
  }

  reference back() {
    assert(m_tail);
    return m_tail->value;
  }

  const_reference back() const {
    assert(m_tail);
    return m_tail->value;
  }

  bool empty() const noexcept {
    assert((m_head != nullptr) == (m_tail != nullptr));
    return m_head == nullptr;
  }

  void clear() {
    Node* current = m_head;
    while (current) {
      Node* next = current->next;
      deleteNode(current);
      current = next;
    }
    m_head = m_tail = nullptr;
  }

 private:
  Node* newNode(T&& value) {
    Node* node =
        static_cast<Node*>(m_resource.allocate(sizeof(Node), alignof(Node)));
    new (&node->value) T(std::forward<T>(value));
    return node;
  }

  Node* newNode(const T& value) {
    Node* node =
        static_cast<Node*>(m_resource.allocate(sizeof(Node), alignof(Node)));
    new (&node->value) T(value);
    return node;
  }

  void deleteNode(Node* node) {
    node->value.~T();
    m_resource.deallocate(node, sizeof(Node), alignof(Node));
  }

 private:
  Resource m_resource;
  Node* m_head;
  Node* m_tail;
};

}  // namespace strobe
