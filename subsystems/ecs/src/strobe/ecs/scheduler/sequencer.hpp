#pragma once

#include "strobe/core/containers/bitset.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/scheduler/allocator.hpp"
#include "strobe/ecs/scheduler/config.hpp"
#include "strobe/ecs/scheduler/job_scheduler.hpp"
#include "strobe/ecs/scheduler/location.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <numeric>
#include <semaphore>
#include <type_traits>
#include <utility>

namespace strobe::ecs {

/// Sequencer memory model
/// ======================
/// Location:
///   A location identifies one abstract memory location.
/// Operation:
///   An operation is one submitted unit of execution.
///   Each operation has an associated memory scope.
///   The memory scope contains the memory access declared
///   for that operation.
/// Memory Access:
///   A memory access is an acquire access, a release access, or an
///   acquire-release access to a location. An acquire-release access is both
///   an acquire and a release access.
/// Sequenced-Before:
///   The relation sequenced-before is an explicit relation between operations.
///   If operation A is sequenced-before operation B, then A is ordered before B
///   in the abstract execution of the schedule.
///   NOTE: sequenced-before is a semantic relation; it does not by itself
///   require A to execute before B.
/// Synchronizes-With:
///   For two operations A and B, A synchronizes with B if A is sequenced before
///   B and one of the following holds for the same location L.
///     - A performs an acquire access to L and B performs a release access to
///     L.
///     - A performs a release access to L and B performs an acquire access to
///     L.
///   NOTE: acq_rel access participates in both rules.
/// Happens-Before:
///   The happens-before relation is the transitive closure of the
///   synchronizes-with relation.
///   Consequently, for two operations A and B where A is sequenced before B
///   and both access the same location:
///     - acquire followed by acquire does not synchronize.
///     - release followed by release does not synchronize.
///     - release followed by acquire synchronizes.
///     - acquire followed by release synchronizes.
///     - acquire-release synchronizes with any later acquire, release, or
///       acquire-release access to the same location.
///     - any earlier acquire, release, or acquire-release access to the same
///       location synchronizes with a later acquire-release access.
///
/// The Sequencer implements this memory model with:
///   submitted-before => sequenced-before
///
/// alloc(), submit(), and fence() are owner-thread only.
/// free() is thread-safe and only retires the location. The actual reset and
/// recycling happen on the owner thread during alloc().
///
/// fence() is owner-thread-only and non-reentrant. The ScopeFn passed to
/// fence() may only declare accesses on the FenceScope. It must not call other
/// Sequencer APIs.
///
/// After free(loc) has been called, submitting another operation that uses loc
/// is a contract violation. Double-free is a contract violation.
///
/// Sequencer destruction requires that no thread can concurrently call free().
class Sequencer {
private:
  static constexpr uint32_t LOCATION_POOL_CHUNK_SIZE = 48;
  using slot_bitset = StaticBitset<schedule::SLOT_COUNT>;

  using allocator = strobe::ecs::scheduler::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

  struct LocationState {
    // Intrusive next pointer. (freelist)
    LocationState *next = nullptr;
    uint64_t observedReuseEpoch = 0;
    slot_bitset release_frontier;
    slot_bitset acquire_frontier;
  };

  class LocationPool {
  public:
    struct Chunk {
      LocationState *states = nullptr;
      uint32_t used = 0;
      Chunk *next = nullptr;
    };

    explicit LocationPool(allocator alloc) noexcept : m_alloc(alloc) {}

    LocationPool(const LocationPool &) = delete;
    LocationPool &operator=(const LocationPool &) = delete;
    LocationPool(LocationPool &&) = delete;
    LocationPool &operator=(LocationPool &&) = delete;

    ~LocationPool() noexcept {
      Chunk *chunk = m_head;
      while (chunk != nullptr) {
        Chunk *next = chunk->next;
        for (uint32_t i = 0; i < chunk->used; ++i) {
          std::destroy_at(chunk->states + i);
        }
        allocator_traits::template deallocate<LocationState>(
            m_alloc, chunk->states, LOCATION_POOL_CHUNK_SIZE);
        std::destroy_at(chunk);
        allocator_traits::template deallocate<Chunk>(m_alloc, chunk, 1);
        chunk = next;
      }
      m_head = nullptr;
    }

    LocationState *allocate() noexcept {
      if (m_head == nullptr || m_head->used == LOCATION_POOL_CHUNK_SIZE) {
        Chunk *chunk = allocator_traits::template allocate<Chunk>(m_alloc, 1);
        assert(chunk != nullptr);
        std::construct_at(chunk);
        chunk->states = allocator_traits::template allocate<LocationState>(
            m_alloc, LOCATION_POOL_CHUNK_SIZE);
        assert(chunk->states != nullptr);
        chunk->used = 0;
        chunk->next = m_head;
        m_head = chunk;
      }
      LocationState *state = m_head->states + m_head->used++;
      std::construct_at(state);
      return state;
    }

  private:
    allocator m_alloc;
    Chunk *m_head = nullptr;
  };

public:
  class FenceScope {
    friend class Sequencer;

  public:
    void acquire(location loc) noexcept {
      assert(m_sequencer != nullptr);
      m_sequencer->fence_acquire(*this, loc);
    }

    void release(location loc) noexcept {
      assert(m_sequencer != nullptr);
      m_sequencer->fence_release(*this, loc);
    }

    void acq_rel(location loc) noexcept {
      assert(m_sequencer != nullptr);
      m_sequencer->fence_acq_rel(*this, loc);
    }

    FenceScope(const FenceScope &) = delete;
    FenceScope &operator=(const FenceScope &) = delete;

    FenceScope(FenceScope &&) = delete;
    FenceScope &operator=(FenceScope &&) = delete;

  private:
    explicit FenceScope(Sequencer *sequencer) noexcept
        : m_sequencer(sequencer) {
      assert(m_sequencer != nullptr);
    }

    Sequencer *m_sequencer;
    slot_bitset m_dependencies;
  };

  class MemoryScope {
    friend class Sequencer;

  public:
    void acquire(location loc) noexcept {
      assert(m_sequencer != nullptr);
      m_sequencer->acquire(*this, loc);
    }

    void release(location loc) noexcept {
      assert(m_sequencer != nullptr);
      m_sequencer->release(*this, loc);
    }

    void acq_rel(location loc) noexcept {
      assert(m_sequencer != nullptr);
      m_sequencer->acq_rel(*this, loc);
    }

    MemoryScope(const MemoryScope &) = delete;
    MemoryScope &operator=(const MemoryScope &) = delete;

    MemoryScope(MemoryScope &&) = delete;
    MemoryScope &operator=(MemoryScope &&) = delete;

  private:
    explicit MemoryScope(Sequencer *sequencer, job_id job) noexcept
        : m_sequencer(sequencer), m_job(job) {}

    Sequencer *m_sequencer;
    job_id m_job;
    slot_bitset m_dependencies;
  };

public:
  explicit Sequencer(const allocator &alloc, uint32_t maxThreadCount) noexcept
      : m_scheduler(alloc, std::min(maxThreadCount, schedule::SLOT_COUNT)),
        m_locationPool(alloc) {
    assert(maxThreadCount > 1);
  }

  Sequencer(const Sequencer &) = delete;
  Sequencer &operator=(const Sequencer &) = delete;

  Sequencer(Sequencer &&) = delete;
  Sequencer &operator=(Sequencer &&) = delete;

  ~Sequencer() noexcept {
    // Stop worker execution before LocationState storage is destroyed.
    // This is intentionally explicit because m_scheduler is declared before
    // m_locationPool and would otherwise be destroyed after it.
    m_scheduler.stop();
    // Move any retired locations into the owner freelist before destruction.
    // This is not required for deallocation, but leaves the intrusive state
    // non-concurrent and well-formed under the destruction contract.
    drain_retired();
  }

  // Owner-thread only.
  location alloc() noexcept {
    drain_retired();
    LocationState *state = nullptr;
    if (m_freelist != nullptr) {
      state = m_freelist;
      m_freelist = state->next;
    } else {
      state = m_locationPool.allocate();
    }
    assert(state != nullptr);
    assert(state->release_frontier.none());
    assert(state->acquire_frontier.none());
    state->next = nullptr;
    state->observedReuseEpoch = m_reuseEpoch;
    return location{static_cast<void *>(state)};
  }

  // Thread-safe.
  //
  // This only retires the location. The owner thread performs the actual reset
  // and recycling during alloc().
  void free(location loc) noexcept {
    LocationState *state = static_cast<LocationState *>(loc.m_ptr);
    assert(state != nullptr);
    LocationState *head = m_retired.load(std::memory_order_relaxed);
    do {
      state->next = head;
    } while (!m_retired.compare_exchange_weak(
        head, state, std::memory_order_release, std::memory_order_relaxed));
  }

  // Owner-thread only.
  template <typename ScopeFn, job_fn Fn>
    requires(std::is_nothrow_invocable_v<ScopeFn &&, MemoryScope &>)
  void submit(ScopeFn &&scopeFn, Fn &&fn) noexcept {
    ZoneScopedN("sequencer::submit");
    job_id job;
    {
      ZoneScopedN("sequencer::acquire-submit-job");
      job = m_scheduler.acquire();
    }
    assert(job.m_index < schedule::SLOT_COUNT);
    if (m_slotGeneration[job.m_index] != job.m_gen) {
      assert(m_reuseEpoch != std::numeric_limits<uint64_t>::max());
      m_slotGeneration[job.m_index] = job.m_gen;
      ++m_reuseEpoch;
      m_slotReuseEpoch[job.m_index] = m_reuseEpoch;
    }
    MemoryScope scope{this, job};
    {
      ZoneScopedN("sequencer::build-job-scope");
      std::forward<ScopeFn>(scopeFn)(scope);
    }
    scope.m_dependencies.reset(job.m_index);
    scope.m_dependencies.for_each_set_bit([&](std::size_t index) noexcept {
      assert(index < schedule::SLOT_COUNT);
      job_id dependency;
      dependency.m_index = static_cast<uint32_t>(index);
      dependency.m_gen = m_slotGeneration[index];
      m_scheduler.addDependency(job, dependency);
    });
    m_scheduler.submit(job, std::forward<Fn>(fn));
  }

  // Owner-thread only.
  //
  // Blocking host fence. The fence scope declares host-side memory accesses.
  // If the scope induces dependencies, this submits a marker job and waits
  // until it completes. If there are no dependencies, no scheduler job is
  // submitted.
  template <typename ScopeFn>
    requires(std::is_nothrow_invocable_v<ScopeFn &&, FenceScope &>)
  void fence(ScopeFn &&scopeFn) noexcept {
    ZoneScopedN("sequencer::fence");
    FenceScope scope{this};
    {
      ZoneScopedN("sequencer::build-fence-scope");
      std::forward<ScopeFn>(scopeFn)(scope);
    }
    if (scope.m_dependencies.none()) {
      return;
    }
    job_id job;
    {
      ZoneScopedN("sequencer::acquire-fence-job");
      job = m_scheduler.acquire();
    }
    assert(job.m_index < schedule::SLOT_COUNT);
    if (m_slotGeneration[job.m_index] != job.m_gen) {
      assert(m_reuseEpoch != std::numeric_limits<uint64_t>::max());
      m_slotGeneration[job.m_index] = job.m_gen;
      ++m_reuseEpoch;
      m_slotReuseEpoch[job.m_index] = m_reuseEpoch;
    }
    scope.m_dependencies.reset(job.m_index);
    scope.m_dependencies.for_each_set_bit([&](std::size_t index) noexcept {
      assert(index < schedule::SLOT_COUNT);
      job_id dependency;
      dependency.m_index = static_cast<uint32_t>(index);
      dependency.m_gen = m_slotGeneration[index];
      m_scheduler.addDependency(job, dependency);
    });
    std::binary_semaphore signal{0};
    m_scheduler.submit(job, [&signal]() noexcept {
      ZoneScopedN("sequencer::fence-job");
      signal.release();
    });
    {
      ZoneScopedN("sequencer::wait-on-fence");
      signal.acquire();
    }
  }

  inline void request_stop() noexcept { m_scheduler.request_stop(); }
  inline void join() noexcept { m_scheduler.join(); }
  inline void stop() noexcept { m_scheduler.stop(); }

private:
  // Owner-thread only.
  void drain_retired() noexcept {
    LocationState *list =
        m_retired.exchange(nullptr, std::memory_order_acquire);
    while (list != nullptr) {
      LocationState *state = list;
      list = state->next;
      state->release_frontier.reset_all();
      state->acquire_frontier.reset_all();
      state->observedReuseEpoch = m_reuseEpoch;
      state->next = m_freelist;
      m_freelist = state;
    }
  }

  void acquire(MemoryScope &scope, location loc) noexcept {
    LocationState *statePtr = static_cast<LocationState *>(loc.m_ptr);
    assert(statePtr != nullptr);
    LocationState &state = *statePtr;
    normalize(state);
    scope.m_dependencies |= state.release_frontier;
    state.acquire_frontier.set(scope.m_job.m_index);
  }

  void release(MemoryScope &scope, location loc) noexcept {
    LocationState *statePtr = static_cast<LocationState *>(loc.m_ptr);
    assert(statePtr != nullptr);
    LocationState &state = *statePtr;
    normalize(state);
    scope.m_dependencies |= state.acquire_frontier;
    state.release_frontier.set(scope.m_job.m_index);
  }

  void acq_rel(MemoryScope &scope, location loc) noexcept {
    LocationState *statePtr = static_cast<LocationState *>(loc.m_ptr);
    assert(statePtr != nullptr);
    LocationState &state = *statePtr;
    normalize(state);
    scope.m_dependencies |= state.acquire_frontier;
    scope.m_dependencies |= state.release_frontier;
    state.acquire_frontier.reset_all();
    state.release_frontier.reset_all();
    state.acquire_frontier.set(scope.m_job.m_index);
    state.release_frontier.set(scope.m_job.m_index);
  }

  void fence_acquire(FenceScope &scope, location loc) noexcept {
    LocationState *statePtr = static_cast<LocationState *>(loc.m_ptr);
    assert(statePtr != nullptr);
    LocationState &state = *statePtr;
    normalize(state);
    scope.m_dependencies |= state.release_frontier;
    // The host acquire waits for all prior releases. Since fence() is
    // owner-thread-only and non-reentrant, no later Sequencer operation can
    // observe this frontier update until the fence has completed.
    state.release_frontier.reset_all();
  }

  void fence_release(FenceScope &scope, location loc) noexcept {
    LocationState *statePtr = static_cast<LocationState *>(loc.m_ptr);
    assert(statePtr != nullptr);
    LocationState &state = *statePtr;
    normalize(state);
    scope.m_dependencies |= state.acquire_frontier;
    // The host release waits for all prior acquires. Since fence() is
    // owner-thread-only and non-reentrant, no later Sequencer operation can
    // observe this frontier update until the fence has completed.
    state.acquire_frontier.reset_all();
  }

  void fence_acq_rel(FenceScope &scope, location loc) noexcept {
    LocationState *statePtr = static_cast<LocationState *>(loc.m_ptr);
    assert(statePtr != nullptr);
    LocationState &state = *statePtr;
    normalize(state);
    scope.m_dependencies |= state.acquire_frontier;
    scope.m_dependencies |= state.release_frontier;
    // The completed host acq_rel dominates both previous frontiers. Since
    // fence() is owner-thread-only and non-reentrant, no later Sequencer
    // operation can observe this frontier update until the fence has completed.
    state.acquire_frontier.reset_all();
    state.release_frontier.reset_all();
  }

  void normalize(LocationState &state) noexcept {
    if (state.observedReuseEpoch == m_reuseEpoch) {
      return;
    }
    const uint64_t observed = state.observedReuseEpoch;
    for (uint32_t i = 0; i < schedule::SLOT_COUNT; ++i) {
      if (m_slotReuseEpoch[i] > observed) {
        state.acquire_frontier.reset(i);
        state.release_frontier.reset(i);
      }
    }
    state.observedReuseEpoch = m_reuseEpoch;
  }

private:
  JobScheduler m_scheduler;

  LocationPool m_locationPool;

  // Owner-thread freelist of fully reset, reusable locations.
  LocationState *m_freelist = nullptr;
  // MPSC stack of retired locations.
  std::atomic<LocationState *> m_retired = nullptr;

  std::array<uint32_t, schedule::SLOT_COUNT> m_slotGeneration{};
  std::array<uint64_t, schedule::SLOT_COUNT> m_slotReuseEpoch{};
  uint64_t m_reuseEpoch = 0;

public:
  static constexpr size_t static_allocation_size(uint32_t threadCount) {
    assert(threadCount > 1);

    const uint32_t workerCount = std::min(threadCount, schedule::SLOT_COUNT);
    return JobScheduler::static_allocation_size(workerCount);
  }

  static constexpr size_t static_allocation_alignment() {
    return JobScheduler::static_allocation_alignment();
  }

  static constexpr size_t dynamic_allocation_size() {
    size_t locationPoolOffset = 0;

    locationPoolOffset += (alignof(LocationPool::Chunk) -
                           locationPoolOffset % alignof(LocationPool::Chunk)) %
                          alignof(LocationPool::Chunk);

    locationPoolOffset += sizeof(LocationPool::Chunk);

    locationPoolOffset +=
        (alignof(LocationState) - locationPoolOffset % alignof(LocationState)) %
        alignof(LocationState);

    locationPoolOffset += sizeof(LocationState) * LOCATION_POOL_CHUNK_SIZE;

    const size_t locationPoolAlignment =
        std::max(alignof(LocationPool::Chunk), alignof(LocationState));

    locationPoolOffset +=
        (locationPoolAlignment - locationPoolOffset % locationPoolAlignment) %
        locationPoolAlignment;

    const size_t schedulerUnit =
        JobScheduler::dynamic_allocation_size() +
        (JobScheduler::dynamic_allocation_alignment() -
         JobScheduler::dynamic_allocation_size() %
             JobScheduler::dynamic_allocation_alignment()) %
            JobScheduler::dynamic_allocation_alignment();

    const size_t largestUnit = std::max(schedulerUnit, locationPoolOffset);

    constexpr size_t chunkGranularity = 4096;
    constexpr size_t minChunkSize = 32 * 1024;
    constexpr size_t largestUnitMultiplier = 4;

    const size_t unroundedChunkSize =
        std::max(minChunkSize, largestUnitMultiplier * largestUnit);

    const size_t result =
        unroundedChunkSize +
        (chunkGranularity - unroundedChunkSize % chunkGranularity) %
            chunkGranularity;

    return result;
  }

  static constexpr size_t dynamic_allocation_alignment() {
    return std::max(
        JobScheduler::dynamic_allocation_alignment(),
        std::max(alignof(LocationPool::Chunk), alignof(LocationState)));
  }
};

} // namespace strobe::ecs
