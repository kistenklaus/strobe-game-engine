#pragma once
#include "strb/stl.hpp"
#include <cassert>
#include <cstddef>
#include <memory>

namespace strb::allocation::pool {

template <typename T> union Chunk {
  T value;
  Chunk<T> *next;

  Chunk() {}
  ~Chunk() {}
};

template <typename T, typename Allocator = std::allocator<Chunk<T>>>
class PoolAllocator {
private:
  static const size_t DEFAULT_SIZE = 1024;
  const size_t m_size;
  Chunk<T> *m_chunks;
  Chunk<T> *m_head;
  Allocator *m_baseAllocator;
  const strb::boolean m_requireAllocatorDeletion;
  size_t m_open_allocations;

public:
  explicit PoolAllocator(size_t size = DEFAULT_SIZE,
                         Allocator *baseAllocator = nullptr)
      : m_size(size), m_baseAllocator(baseAllocator == nullptr) {
    assert(size >= 16);
    if (!m_baseAllocator) {
      m_baseAllocator = new Allocator();
    }

    m_chunks = m_baseAllocator->allocate(size);
    m_head = m_chunks;
    for (size_t i = 0; i < size - 1; i++) {
      m_chunks[i].next = std::addressof(m_chunks[i + 1]);
    }
    m_chunks[size - 1].next = nullptr;
  }
  ~PoolAllocator() {
    m_baseAllocator->deallocate(m_chunks, m_size);
    delete m_chunks;
    if (m_requireAllocatorDeletion) {
      delete m_baseAllocator;
    }
    if (m_open_allocations > 0) {
      strb::error("WARNING from PoolAllocator: ");
      strb::errorln(typeid(T).name());
      strb::error(m_open_allocations);
      strb::errorln(" elements where not deallocated after alloations");
    }
    m_chunks = nullptr;
    m_head = nullptr;
  }
  PoolAllocator(const PoolAllocator &) = delete;
  PoolAllocator(const PoolAllocator &&) = delete;
  PoolAllocator &operator=(const PoolAllocator &) = delete;
  PoolAllocator &operator=(const PoolAllocator &&) = delete;

  template <typename... Args> T *allocate(Args &&...args) {
    assert(m_head != nullptr);
    if (m_head == nullptr) {
      return nullptr;
    }
    Chunk<T> *allocatedChunk = m_head;
    m_head = m_head->next;
    T *newChunk = new (std::addressof(allocatedChunk->value))
        T(std::forward<Args>(args)...);
    m_open_allocations++;
    return newChunk;
  }

  void deallocate(T *data) {
    assert(data >= m_chunks &&
           data < m_chunks + m_size); // assert that data is inside of m_chunks
    data->~T();
    Chunk<T> *chunk = reinterpret_cast<Chunk<T> *>(data);
    chunk->next = m_head;
    m_head = chunk;
    m_open_allocations--;
  }
};

} // namespace strb::allocation::pool
