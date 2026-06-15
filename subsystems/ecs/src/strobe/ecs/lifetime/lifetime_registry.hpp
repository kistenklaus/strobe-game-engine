#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/lifetime/lifetime_hook.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe::ecs {

/// LifetimeRegistry:
///
/// A lifetime has a lifetime_hook and an immutable set of required lifetimes.
///
/// Two requirement kinds exist:
///
///   require(id)
///     The required lifetime must be entered before this lifetime may enter,
///     and it must remain entered while this lifetime is entered.
///     If the required lifetime exits, this lifetime exits first.
///
///   require_enter(id)
///     The required lifetime must be entered before this lifetime may enter,
///     but it is not required while this lifetime remains entered.
///     If the required lifetime exits, this lifetime remains entered, but
///     cannot re-enter until the requirement enters again.
///
/// Interface:
///   alloc(hook, depFn)  - registers a non-requested / non-entered lifetime.
///   construct(id)       - requests enter; enters once enter requirements hold.
///   destruct(id)        - clears request; exits if currently entered.
///   free(id)            - unregisters a non-requested / non-entered lifetime.
///   Scope::require(id)        - active requirement.
///   Scope::require_enter(id)  - enter-only requirement.
///
/// Guarantees:
///   Requirements enter before dependents.
///   Active dependents exit before active requirements exit.
///
/// Contract:
///   Owner-thread only.
///   Calling any registry interface from lifetime_hook::enter / exit is UB.
class LifetimeRegistry;

struct null_lifetime_id_t {};
static constexpr null_lifetime_id_t null_lifetime_id = {};

struct lifetime_id {
  friend class LifetimeRegistry;

public:
  lifetime_id(null_lifetime_id_t) noexcept : m_ptr(nullptr) {}
  lifetime_id() noexcept : m_ptr(nullptr) {}

  friend inline bool operator==(const lifetime_id &id,
                                null_lifetime_id_t) noexcept {
    return id.m_ptr == nullptr;
  }

  friend inline bool operator==(null_lifetime_id_t,
                                const lifetime_id &id) noexcept {
    return id.m_ptr == nullptr;
  }

  friend inline bool operator!=(const lifetime_id &id,
                                null_lifetime_id_t) noexcept {
    return id.m_ptr != nullptr;
  }

  friend inline bool operator!=(null_lifetime_id_t,
                                const lifetime_id &id) noexcept {
    return id.m_ptr != nullptr;
  }

private:
  explicit lifetime_id(void *ptr) noexcept : m_ptr(ptr) {}

  void *m_ptr;
};

enum class lifetime_requirement : uint8_t { entry, persistent };

class LifetimeRegistry {
public:
  using allocator = strobe::ecs::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

  explicit LifetimeRegistry(const allocator &alloc)
      : m_recordPool(alloc), m_listPool(alloc) {}

  LifetimeRegistry(const LifetimeRegistry &) = delete;
  LifetimeRegistry &operator=(const LifetimeRegistry &) = delete;
  LifetimeRegistry(LifetimeRegistry &&) = delete;
  LifetimeRegistry &operator=(LifetimeRegistry &&) = delete;

  ~LifetimeRegistry() = default;

private:
  struct Record;

  struct FwdList {
    static constexpr std::uintptr_t active_requirement_bit = std::uintptr_t{1};
    static constexpr std::uintptr_t pointer_mask = ~active_requirement_bit;

    FwdList *next;
    std::uintptr_t item_and_flags;

    void set(Record *record, bool active_requirement) noexcept {
      const auto ptr = reinterpret_cast<std::uintptr_t>(record);
      assert((ptr & active_requirement_bit) == 0);
      item_and_flags = ptr | (active_requirement ? active_requirement_bit
                                                 : std::uintptr_t{0});
    }

    Record *item() const noexcept {
      return reinterpret_cast<Record *>(item_and_flags & pointer_mask);
    }

    bool active_requirement() const noexcept {
      return (item_and_flags & active_requirement_bit) != 0;
    }
  };

  using FwdListPool =
      MonotonicPoolResource<sizeof(FwdList), alignof(FwdList), allocator>;

  struct Record {
    lifetime_hook *hook = nullptr;
    FwdList *dependent = nullptr;
    uint32_t blocked : 30 = 0;
    uint32_t requested : 1 = false;
    uint32_t entered : 1 = false;
    uint32_t refcount : 31 = 0;
    uint32_t freed : 1 = false;
  };

  static_assert(
      alignof(Record) >= 2,
      "LifetimeRegistry requires at least one free low bit in Record*");

  using RecordPool =
      MonotonicPoolResource<sizeof(Record), alignof(Record), allocator>;

public:
  class DependencyScope {
    friend class LifetimeRegistry;

  public:
    void require(
        lifetime_id id,
        lifetime_requirement req = lifetime_requirement::persistent) noexcept {
      Record *required = static_cast<Record *>(id.m_ptr);
      assert(required != nullptr);
      assert(required != m_record);
      assert(!required->freed);
      FwdList *node = static_cast<FwdList *>(m_pool->allocate());
      std::construct_at(node);
      node->next = required->dependent;
      node->set(m_record, req != lifetime_requirement::entry);
      required->dependent = node;
      if (!required->entered) {
        m_record->blocked += 1;
      }
      m_record->refcount += 1;
    }

  private:
    DependencyScope(Record *record, FwdListPool *pool)
        : m_record(record), m_pool(pool) {}

    Record *m_record;
    FwdListPool *m_pool;
  };

public:
  template <typename DepScopeFn>
    requires(std::is_nothrow_invocable_v<DepScopeFn, DependencyScope &>)
  lifetime_id alloc(DepScopeFn &&fn) noexcept {
    Record *record = static_cast<Record *>(m_recordPool.allocate());
    std::construct_at(record);
    record->hook = nullptr;
    record->dependent = nullptr;
    record->blocked = 0;
    record->requested = false;
    record->entered = false;
    record->refcount = 1;
    record->freed = false;
    DependencyScope scope{record, &m_listPool};
    std::forward<DepScopeFn>(fn)(scope);
    return lifetime_id{record};
  }

  void install(lifetime_id id, lifetime_hook *hook) {
    Record *record = static_cast<Record *>(id.m_ptr);
    record->hook = hook;
  }

  void free(lifetime_id id) noexcept {
    Record *record = static_cast<Record *>(id.m_ptr);
    assert(record != nullptr);
    assert(!record->freed);
    assert(record->refcount > 0);
    assert(!record->requested);
    assert(!record->entered);
    record->freed = true;
    record->refcount -= 1;
    if (record->refcount == 0) {
      destroy_record(record);
    }
  }

  void construct(Universe *universe, lifetime_id id) noexcept {
    Record *record = static_cast<Record *>(id.m_ptr);
    assert(record != nullptr);
    assert(!record->freed);
    assert(record->refcount > 0);
    assert(!record->requested);
    record->requested = true;
    try_enter(universe, record);
  }

  void destruct(Universe *universe, lifetime_id id) noexcept {
    Record *record = static_cast<Record *>(id.m_ptr);
    assert(record != nullptr);
    assert(!record->freed);
    assert(record->refcount > 0);
    assert(record->requested);
    record->requested = false;
    if (record->entered) {
      exit_entered(universe, record);
    }
  }

  bool is_live(lifetime_id id) const noexcept {
    const Record *record = static_cast<const Record *>(id.m_ptr);
    assert(record != nullptr);
    assert(!record->freed);
    assert(record->refcount > 0);
    return record->entered;
  }

  bool is_constructed(lifetime_id id) const noexcept {
    const Record *record = static_cast<const Record *>(id.m_ptr);
    assert(record != nullptr);
    assert(!record->freed);
    assert(record->refcount > 0);
    return record->requested;
  }

private:
  void try_enter(Universe *universe, Record *record) noexcept {
    assert(record != nullptr);
    assert(!record->freed);
    if (!record->requested) {
      return;
    }
    if (record->entered) {
      return;
    }
    if (record->blocked != 0) {
      return;
    }
    record->entered = true;
    if (record->hook != nullptr) {
      record->hook->enter(universe);
    }
    notify_available(universe, record->dependent);
  }

  void exit_entered(Universe *universe, Record *record) noexcept {
    assert(record != nullptr);
    assert(!record->freed);
    assert(record->entered);
    force_exit_active_dependents(universe, record->dependent);
    record->entered = false;
    notify_unavailable(record->dependent);
    if (record->hook != nullptr) {
      record->hook->exit(universe);
    }
  }

  void force_exit_active_dependents(Universe *universe,
                                    FwdList *&list) noexcept {
    FwdList **link = &list;
    while (*link != nullptr) {
      FwdList *curr = *link;
      if (remove_if_freed(link)) {
        continue;
      }
      Record *dependent = curr->item();
      if (curr->active_requirement() && dependent->entered) {
        exit_entered(universe, dependent);
      }
      link = &curr->next;
    }
  }

  void notify_available(Universe *universe, FwdList *&list) noexcept {
    FwdList **link = &list;
    while (*link != nullptr) {
      FwdList *curr = *link;
      if (remove_if_freed(link)) {
        continue;
      }
      Record *dependent = curr->item();
      assert(dependent->blocked > 0);
      dependent->blocked -= 1;
      try_enter(universe, dependent);
      link = &curr->next;
    }
  }

  void notify_unavailable(FwdList *&list) noexcept {
    FwdList **link = &list;
    while (*link != nullptr) {
      FwdList *curr = *link;
      if (remove_if_freed(link)) {
        continue;
      }
      Record *dependent = curr->item();
      dependent->blocked += 1;
      link = &curr->next;
    }
  }

  bool remove_if_freed(FwdList **link) noexcept {
    assert(link != nullptr);
    assert(*link != nullptr);
    FwdList *node = *link;
    Record *dependent = node->item();
    if (!dependent->freed) {
      return false;
    }
    *link = node->next;
    release_dependency_edge(node);
    return true;
  }

  void release_dependency_edge(FwdList *node) noexcept {
    assert(node != nullptr);
    Record *dependent = node->item();
    assert(dependent != nullptr);
    assert(dependent->freed);
    assert(dependent->refcount > 0);
    dependent->refcount -= 1;
    if (dependent->refcount == 0) {
      destroy_record(dependent);
    }
    std::destroy_at(node);
    m_listPool.deallocate(node);
  }

  void destroy_record(Record *record) noexcept {
    assert(record != nullptr);
    assert(record->freed);
    assert(record->refcount == 0);
    assert(!record->requested);
    assert(!record->entered);
    while (record->dependent != nullptr) {
      FwdList *node = record->dependent;
      record->dependent = node->next;
      Record *dependent = node->item();
      assert(dependent != nullptr);
      assert(dependent->refcount > 0);
      dependent->refcount -= 1;
      if (dependent->freed && dependent->refcount == 0) {
        destroy_record(dependent);
      }
      std::destroy_at(node);
      m_listPool.deallocate(node);
    }
    std::destroy_at(record);
    m_recordPool.deallocate(record);
  }

private:
  RecordPool m_recordPool;
  FwdListPool m_listPool;
};

} // namespace strobe::ecs
