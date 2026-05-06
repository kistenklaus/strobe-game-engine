#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/ecs/allocator.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe::ecs {

template <typename E>
  requires(!std::is_reference_v<E>)
class EventQueue {
public:
  using value_type = E;
  using allocator = strobe::ecs::event_queue_allocator;
  using allocator_traits = AllocatorTraits<allocator>;

private:
  static constexpr std::size_t SHRINK_MIN_CAPACITY = 4;
  static constexpr float SHRINK_EMA_ALPHA = 0.1f;
  static constexpr float SHRINK_THRESHOLD = 0.1f;
  static constexpr std::uint32_t SHRINK_DELAY = 64;

public:
  explicit EventQueue(const allocator& alloc) : m_alloc(alloc) {}

  EventQueue(const EventQueue&) = delete;
  EventQueue& operator=(const EventQueue&) = delete;

  EventQueue(EventQueue&&) = delete;
  EventQueue& operator=(EventQueue&&) = delete;

  ~EventQueue() {
    if constexpr (!std::is_trivially_destructible_v<value_type>) {
      for (std::size_t i = 0; i < m_size; ++i) {
        std::destroy_at(m_data + i);
      }
    }

    if (m_data != nullptr) {
      allocator_traits::deallocate(
          m_alloc,
          m_data,
          sizeof(value_type) * m_capacity,
          alignof(value_type));
    }
  }

  template <typename... Args>
    requires std::constructible_from<value_type, Args&&...>
  value_type& emplace(Args&&... args) {
    if (m_size == m_capacity) {
      const std::size_t new_capacity =
          m_capacity == 0 ? SHRINK_MIN_CAPACITY : m_capacity * 2;

      assert(new_capacity > m_capacity);

      void* raw = allocator_traits::allocate(
          m_alloc,
          sizeof(value_type) * new_capacity,
          alignof(value_type));

      value_type* new_data = static_cast<value_type*>(raw);

      std::size_t constructed = 0;

      try {
        for (; constructed < m_size; ++constructed) {
          std::construct_at(
              new_data + constructed,
              std::move_if_noexcept(m_data[constructed]));
        }
      } catch (...) {
        if constexpr (!std::is_trivially_destructible_v<value_type>) {
          for (std::size_t i = 0; i < constructed; ++i) {
            std::destroy_at(new_data + i);
          }
        }

        allocator_traits::deallocate(
            m_alloc,
            new_data,
            sizeof(value_type) * new_capacity,
            alignof(value_type));

        throw;
      }

      if constexpr (!std::is_trivially_destructible_v<value_type>) {
        for (std::size_t i = 0; i < m_size; ++i) {
          std::destroy_at(m_data + i);
        }
      }

      if (m_data != nullptr) {
        allocator_traits::deallocate(
            m_alloc,
            m_data,
            sizeof(value_type) * m_capacity,
            alignof(value_type));
      }

      m_data = new_data;
      m_capacity = new_capacity;

      // A queue that had to grow should not be considered persistently
      // underused immediately afterwards.
      m_underused_resets = 0;
    }

    value_type* ptr = m_data + m_size;
    std::construct_at(ptr, std::forward<Args>(args)...);
    ++m_size;

    return *ptr;
  }

  const value_type* begin() const noexcept { return m_data; }

  const value_type* end() const noexcept { return m_data + m_size; }

  strobe::span<const value_type> span() const noexcept {
    return strobe::span<const value_type>{m_data, m_size};
  }

  std::size_t size() const noexcept { return m_size; }

  std::size_t capacity() const noexcept { return m_capacity; }

  bool empty() const noexcept { return m_size == 0; }

  void clear() noexcept {
    if (m_size == 0) {
      return;
    }

    if constexpr (!std::is_trivially_destructible_v<value_type>) {
      for (std::size_t i = 0; i < m_size; ++i) {
        std::destroy_at(m_data + i);
      }
    }

    m_size = 0;
  }

  void reset() {
    if (m_size == 0) {
      if (m_capacity <= SHRINK_MIN_CAPACITY) {
        return;
      }

      m_ema_occupancy =
          (1.0f - SHRINK_EMA_ALPHA) * m_ema_occupancy;
    } else {
      assert(m_capacity > 0);

      const float occupancy =
          static_cast<float>(m_size) / static_cast<float>(m_capacity);

      m_ema_occupancy =
          SHRINK_EMA_ALPHA * occupancy +
          (1.0f - SHRINK_EMA_ALPHA) * m_ema_occupancy;

      if constexpr (!std::is_trivially_destructible_v<value_type>) {
        for (std::size_t i = 0; i < m_size; ++i) {
          std::destroy_at(m_data + i);
        }
      }

      m_size = 0;
    }

    if (m_capacity <= SHRINK_MIN_CAPACITY) {
      m_underused_resets = 0;
      return;
    }

    if (m_ema_occupancy >= SHRINK_THRESHOLD) {
      m_underused_resets = 0;
      return;
    }

    ++m_underused_resets;

    if (m_underused_resets < SHRINK_DELAY) {
      return;
    }

    m_underused_resets = 0;

    assert(m_size == 0);

    const std::size_t new_capacity =
        m_capacity / 2 < SHRINK_MIN_CAPACITY
            ? SHRINK_MIN_CAPACITY
            : m_capacity / 2;

    if (new_capacity >= m_capacity) {
      return;
    }

    void* raw = allocator_traits::allocate(
        m_alloc,
        sizeof(value_type) * new_capacity,
        alignof(value_type));

    value_type* new_data = static_cast<value_type*>(raw);

    allocator_traits::deallocate(
        m_alloc,
        m_data,
        sizeof(value_type) * m_capacity,
        alignof(value_type));

    m_data = new_data;
    m_capacity = new_capacity;
  }

private:
  allocator m_alloc;

  value_type* m_data = nullptr;
  std::size_t m_size = 0;
  std::size_t m_capacity = 0;

  float m_ema_occupancy = 1.0f;
  std::uint32_t m_underused_resets = 0;
};

} // namespace strobe::ecs
