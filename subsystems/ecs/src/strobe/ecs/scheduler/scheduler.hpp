#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/scheduler/allocator.hpp"
#include "strobe/ecs/scheduler/operation.hpp"
#include "strobe/ecs/scheduler/sequencer.hpp"
#include <memory>
namespace strobe::ecs {

class Scheduler {
public:
  using allocator = strobe::ecs::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

private: // allocation estimates.
  static constexpr size_t static_allocation_size(uint32_t threadCount) {
    return Sequencer::static_allocation_size(threadCount) + sizeof(Sequencer) +
           Sequencer::dynamic_allocation_size();
  }

public:
  explicit Scheduler(const allocator &alloc, uint32_t threadCount)
      : m_alloc(std::in_place, alloc, static_allocation_size(threadCount),
                Sequencer::dynamic_allocation_size()) {
    m_sequencer =
        strobe::ecs::scheduler::allocator_traits::template allocate<Sequencer>(
            m_alloc);
    std::construct_at(m_sequencer, &m_alloc, threadCount);
  }
  Scheduler(const Scheduler &) = delete;
  Scheduler &operator=(const Scheduler &) = delete;
  Scheduler(Scheduler &&) = delete;
  Scheduler &operator=(Scheduler &&) = delete;
  ~Scheduler() {
    if (m_sequencer != nullptr) {
      std::destroy_at(m_sequencer);
      strobe::ecs::scheduler::allocator_traits::template deallocate<Sequencer>(
          m_alloc, m_sequencer);
      m_sequencer = nullptr;
    }
  }

  // owner-thread only.
  inline location alloc() noexcept { return m_sequencer->alloc(); }

  // thread-safe.
  //
  // This only retires the location. The owner thread performs the actual reset
  // and recycling during alloc().
  inline void free(location loc) noexcept { return m_sequencer->free(loc); }

  // owner-thread only.
  template <typename ScopeFn, job_fn Fn>
    requires(std::is_nothrow_invocable_v<ScopeFn &&, Sequencer::MemoryScope &>)
  inline void submit(ScopeFn &&scopeFn, Fn &&fn) noexcept {
    return m_sequencer->submit(std::forward<ScopeFn>(scopeFn),
                               std::forward<Fn>(fn));
  }

  inline void submit(const operation *op) noexcept {
    return m_sequencer->submit(
        [op](Sequencer::MemoryScope &scope) noexcept -> void {
          for (const location *it = op->acquire; it->m_ptr; ++it) {
            scope.acquire(*it);
          }
          for (const location *it = op->release; it->m_ptr; ++it) {
            scope.release(*it);
          }
          for (const location *it = op->acq_rel; it->m_ptr; ++it) {
            scope.acq_rel(*it);
          }
        },
        [op](uint32_t invoc) noexcept -> uint32_t {
          return op->execute(op->object, invoc);
        });
  }

  template <typename ScopeDesc, job_fn Fn>
    requires(is_operation_scope_desc_v<ScopeDesc>)
  inline void submit(ScopeDesc &&scopeDesc, Fn &&fn) noexcept {
    return m_sequencer->submit(
        [&scopeDesc](Sequencer::MemoryScope &scope) noexcept {
          scopeDesc.apply(scope);
        },
        std::forward<Fn>(fn));
  }

  // owner-thread only.
  //
  // Blocking host fence. The fence scope declares host-side memory accesses.
  // If the scope induces dependencies, this submits a marker job and waits
  // until it completes. If there are no dependencies, no scheduler job is
  // submitted.
  template <typename ScopeFn>
    requires(std::is_nothrow_invocable_v<ScopeFn &&, Sequencer::FenceScope &>)
  inline void fence(ScopeFn &&scopeFn) noexcept {
    return m_sequencer->fence(std::forward<ScopeFn>(scopeFn));
  }

  template <typename ScopeDesc>
    requires(is_operation_scope_desc_v<ScopeDesc>)
  inline void fence(ScopeDesc &&scopeDesc) {
    m_sequencer->fence([&scopeDesc](Sequencer::FenceScope &scope) noexcept {
      scopeDesc.apply(scope);
    });
  }

  inline void request_stop() noexcept { m_sequencer->request_stop(); }
  inline void join() noexcept { m_sequencer->join(); }
  inline void stop() noexcept { m_sequencer->stop(); }

private:
  strobe::ecs::scheduler::allocator m_alloc;
  Sequencer *m_sequencer;
};

} // namespace strobe::ecs
