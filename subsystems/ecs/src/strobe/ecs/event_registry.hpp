#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/containers/vector_storage.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/event_queue.hpp"
#include "strobe/ecs/events.hpp"

#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace strobe::ecs {

class EventRegistry;

using event_type_id = uint32_t;

namespace details {

struct EventRegistryBank {
  EventRegistryBank *next = nullptr;
  VectorStorage<void *> queues;
};

} // namespace details

struct event_epoch {
  friend class EventRegistry;

public:
private:
  explicit event_epoch(details::EventRegistryBank *bank) : m_bank(bank) {}

  details::EventRegistryBank *m_bank = nullptr;
};

class EventRegistry {
public:
  using allocator = strobe::ecs::allocator_ref;
  static_assert(std::same_as<allocator, event_queue_allocator>);

  using allocator_traits = AllocatorTraits<allocator>;

  EventRegistry(const allocator &alloc) : m_alloc(alloc), m_queuePool(alloc) {}

  EventRegistry(const EventRegistry &) = delete;
  EventRegistry &operator=(const EventRegistry &) = delete;

  EventRegistry(EventRegistry &&) = delete;
  EventRegistry &operator=(EventRegistry &&) = delete;

  ~EventRegistry() {
    assert(m_activeEpochs.load(std::memory_order_relaxed) == 0);

    std::shared_ptr<const EventQueueTypeInfos> type_infos =
        s_typeInfos.load(std::memory_order_acquire);

    details::EventRegistryBank *bank =
        m_freelist.exchange(nullptr, std::memory_order_acquire);

    while (bank != nullptr) {
      details::EventRegistryBank *next = bank->next;

      assert(bank->queues.size() <= type_infos->size());

      for (event_type_id id = 0; id < bank->queues.size(); ++id) {
        void *queue = bank->queues[id];

        assert(queue != nullptr);

        const EventQueueTypeInfo &info = (*type_infos)[id];

        assert(info.destroy != nullptr);

        info.destroy(queue);
        bank->queues[id] = nullptr;
      }

      std::destroy_at(bank);
      allocator_traits::template deallocate<details::EventRegistryBank>(m_alloc,
                                                                        bank);

      bank = next;
    }
  }

  template <typename E>
    requires(!std::is_reference_v<E>)
  static event_type_id get_event_type_id() {
    using event_type = std::remove_cvref_t<E>;

    static const event_type_id id = []() {
      EventQueueTypeInfo info{
          .reset =
              [](void *ptr) {
                static_cast<EventQueue<event_type> *>(ptr)->reset();
              },
          .destroy =
              [](void *ptr) {
                std::destroy_at(static_cast<EventQueue<event_type> *>(ptr));
              },
          .construct =
              [](void *ptr, const event_queue_allocator &alloc) {
                std::construct_at(static_cast<EventQueue<event_type> *>(ptr),
                                  alloc);
              },
      };

      while (true) {
        std::shared_ptr<const EventQueueTypeInfos> old_infos =
            s_typeInfos.load(std::memory_order_acquire);

        auto new_infos = std::make_shared<EventQueueTypeInfos>(*old_infos);

        const event_type_id id = static_cast<event_type_id>(new_infos->size());

        new_infos->push_back(info);

        std::shared_ptr<const EventQueueTypeInfos> desired{new_infos};
        std::shared_ptr<const EventQueueTypeInfos> expected = old_infos;

        if (s_typeInfos.compare_exchange_weak(expected, desired,
                                              std::memory_order_release,
                                              std::memory_order_acquire)) {
          return id;
        }
      }
    }();

    return id;
  }

  event_epoch beginEpoch() noexcept {
    details::EventRegistryBank *bank =
        m_freelist.load(std::memory_order_acquire);

    while (bank != nullptr) {
      details::EventRegistryBank *next = bank->next;

      if (m_freelist.compare_exchange_weak(bank, next,
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed)) {
        bank->next = nullptr;
        break;
      }
    }

    if (bank == nullptr) {
      bank = allocator_traits::template allocate<details::EventRegistryBank>(
          m_alloc);
      std::construct_at(bank);
    }

    std::shared_ptr<const EventQueueTypeInfos> type_infos =
        s_typeInfos.load(std::memory_order_acquire);

    const std::size_t type_count = type_infos->size();

    while (bank->queues.size() < type_count) {
      const event_type_id id = static_cast<event_type_id>(bank->queues.size());

      const EventQueueTypeInfo &info = (*type_infos)[id];

      void *ptr = m_queuePool.allocate();
      info.construct(ptr, m_alloc);

      bank->queues.push_back(m_alloc, ptr);
    }

    m_activeEpochs.fetch_add(1, std::memory_order_relaxed);

    return event_epoch(bank);
  }

  void endEpoch(event_epoch epoch) noexcept {
    details::EventRegistryBank *bank = epoch.m_bank;
    assert(bank != nullptr);

    std::shared_ptr<const EventQueueTypeInfos> type_infos =
        s_typeInfos.load(std::memory_order_acquire);

    assert(bank->queues.size() <= type_infos->size());

    for (event_type_id id = 0; id < bank->queues.size(); ++id) {
      void *queue = bank->queues[id];

      assert(queue != nullptr);

      const EventQueueTypeInfo &info = (*type_infos)[id];

      assert(info.reset != nullptr);

      info.reset(queue);
    }

    [[maybe_unused]] const std::uint32_t previous_active =
        m_activeEpochs.fetch_sub(1, std::memory_order_relaxed);

    assert(previous_active > 0);

    details::EventRegistryBank *head =
        m_freelist.load(std::memory_order_relaxed);

    do {
      bank->next = head;
    } while (!m_freelist.compare_exchange_weak(
        head, bank, std::memory_order_release, std::memory_order_relaxed));
  }

  template <typename E>
    requires(!std::is_reference_v<E> &&
             !std::is_const_v<std::remove_reference_t<E>>)
  Events<E> getWriter(event_epoch epoch) {
    using event_type = std::remove_cvref_t<E>;

    const event_type_id id = get_event_type_id<event_type>();

    assert(epoch.m_bank != nullptr);
    assert(id < epoch.m_bank->queues.size());
    assert(epoch.m_bank->queues[id] != nullptr);

    return Events<E>(
        static_cast<EventQueue<event_type> *>(epoch.m_bank->queues[id]));
  }

  template <typename E>
    requires(!std::is_reference_v<E>)
  Events<const std::remove_cv_t<E>> getReader(event_epoch epoch) {
    using event_type = std::remove_cvref_t<E>;

    const event_type_id id = get_event_type_id<event_type>();

    assert(epoch.m_bank != nullptr);
    assert(id < epoch.m_bank->queues.size());
    assert(epoch.m_bank->queues[id] != nullptr);

    return Events<const event_type>(
        static_cast<EventQueue<event_type> *>(epoch.m_bank->queues[id]));
  }

private:
  struct EventQueueTypeInfo {
    void (*reset)(void *ptr) = nullptr;
    void (*destroy)(void *ptr) = nullptr;
    void (*construct)(void *ptr, const event_queue_allocator &alloc) = nullptr;
  };

  using EventQueueTypeInfos = Vector<EventQueueTypeInfo, strobe::Mallocator>;

private:
  allocator m_alloc;

  MonotonicPoolResource<sizeof(EventQueue<int>), alignof(EventQueue<int>),
                        allocator>
      m_queuePool;

  std::atomic<details::EventRegistryBank *> m_freelist = nullptr;
  std::atomic<std::uint32_t> m_activeEpochs = 0;

  static_assert(sizeof(EventQueue<uint8_t>) == sizeof(EventQueue<uint64_t>));
  static_assert(alignof(EventQueue<uint8_t>) == alignof(EventQueue<uint64_t>));

  inline static std::atomic<std::shared_ptr<const EventQueueTypeInfos>>
      s_typeInfos{std::make_shared<const EventQueueTypeInfos>()};
};

} // namespace strobe::ecs
