#pragma once

#include "strobe/core/memory/align.hpp"
#include "strobe/core/memory/cache_line.hpp"
#include "strobe/ecs/scheduler/allocator.hpp"
#include "strobe/ecs/scheduler/job_id.hpp"
#include "strobe/ecs/scheduler/job_scoreboard.hpp"
#include "strobe/ecs/scheduler/worker_pool.hpp"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <ranges>
#include <semaphore>
#include <type_traits>
#include <utility>

namespace strobe::ecs {

static constexpr size_t MAX_JOB_FN_SIZE = 16;

template <typename Fn>
concept job_fn =
    std::is_object_v<std::remove_cvref_t<Fn>> &&
    (sizeof(std::remove_cvref_t<Fn>) <= MAX_JOB_FN_SIZE) &&
    (alignof(std::remove_cvref_t<Fn>) <= alignof(std::max_align_t)) &&
    (std::is_nothrow_invocable_v<std::remove_cvref_t<Fn> &> ||
     std::is_nothrow_invocable_v<std::remove_cvref_t<Fn> &, uint32_t>);

class JobScheduler {
public:
  using allocator = strobe::ecs::scheduler::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

  static constexpr uint32_t SLOT_COUNT = strobe::ecs::schedule::SLOT_COUNT;

private:
  struct JobFn {
    friend class JobScheduler;

    JobFn() noexcept = default;

    JobFn(const JobFn &) = delete;
    JobFn(JobFn &&) = delete;
    JobFn &operator=(const JobFn &) = delete;
    JobFn &operator=(JobFn &&) = delete;

    ~JobFn() noexcept { release(); }

    template <job_fn Fn> void emplace(Fn &&fn) noexcept {
      assert(m_execute == nullptr);
      assert(m_destroy == nullptr);

      using T = std::remove_cvref_t<Fn>;
      static_assert(job_fn<T>);

      std::construct_at(reinterpret_cast<T *>(m_storage), std::forward<Fn>(fn));

      m_execute = [](void *self,
                     uint32_t subInvocationId) noexcept -> uint32_t {
        T *typed = reinterpret_cast<T *>(self);

        if constexpr (std::is_nothrow_invocable_v<T &, uint32_t>) {
          if constexpr (std::is_void_v<std::invoke_result_t<T &, uint32_t>>) {
            (*typed)(subInvocationId);
            return 0;
          } else {
            return static_cast<uint32_t>((*typed)(subInvocationId));
          }
        } else {
          if constexpr (std::is_void_v<std::invoke_result_t<T &>>) {
            (*typed)();
            return 0;
          } else {
            return static_cast<uint32_t>((*typed)());
          }
        }
      };

      m_destroy = [](void *self) noexcept {
        T *typed = reinterpret_cast<T *>(self);
        std::destroy_at(typed);
      };
    }

    bool empty() noexcept {
      assert((m_destroy == nullptr) == (m_execute == nullptr));
      return m_destroy == nullptr;
    }

    void release() noexcept {
      if (m_destroy != nullptr) {
        m_destroy(m_storage);
        m_destroy = nullptr;
        m_execute = nullptr;
      }
    }

    void operator()(WorkerPool<JobFn>::Context *ctx) noexcept {
      JobSlot *slot = JobSlot::from_fn_ptr(this);

      const uint32_t invocationId =
          m_invocationCounter.fetch_add(1, std::memory_order_relaxed);

      assert(m_execute != nullptr);

      const uint32_t children =
          m_execute(static_cast<void *>(m_storage), invocationId);

      if (children != 0) {
        m_liveInvocations.fetch_add(children, std::memory_order_relaxed);

        for (uint32_t i = 0; i < children; ++i) {
          ctx->submit_local(this);
        }
      }

      const uint32_t old =
          m_liveInvocations.fetch_sub(1, std::memory_order_acq_rel);

      assert(old > 0);

      if (old != 1) {
        // There are still outstanding child invocations.
        return;
      }

      // Destroy Fn handle.
      release();

      // Issue ready tasks.
      for (JobScoreboard *consumer : slot->scoreboard.complete()) {
        JobSlot *readySlot = JobSlot::from_scoreboard_ptr(consumer);
        JobFn &fn = readySlot->fn;

        fn.m_invocationCounter.store(0, std::memory_order_relaxed);

        [[maybe_unused]] const uint32_t old =
            fn.m_liveInvocations.fetch_add(1, std::memory_order_relaxed);

        assert(old == 0);

        ctx->submit(&fn);
      }

      // Release slot.
      JobScheduler *scheduler = slot->scheduler;

      JobSlot *head = scheduler->m_freeSlots.load(std::memory_order_relaxed);

      do {
        slot->next = head;
      } while (!scheduler->m_freeSlots.compare_exchange_weak(
          head, slot, std::memory_order_release, std::memory_order_relaxed));

      scheduler->m_freeSlotCount.release();
    }

  private:
    alignas(memory::cache_line) std::atomic<uint32_t> m_liveInvocations = 0;

    alignas(memory::cache_line) std::atomic<uint32_t> m_invocationCounter = 0;

    uint32_t (*m_execute)(void *, uint32_t) noexcept = nullptr;
    void (*m_destroy)(void *self) noexcept = nullptr;

    alignas(std::max_align_t) std::byte m_storage[MAX_JOB_FN_SIZE];
  };

  using worker_pool = WorkerPool<JobFn>;
  using semaphore = std::counting_semaphore<>;

  static_assert(SLOT_COUNT > 1);

  struct JobSlot {
    JobSlot(allocator alloc, JobScheduler *scheduler)
        : fn(), scoreboard(alloc), generation(0), scheduler(scheduler) {}

    static JobSlot *from_scoreboard_ptr(JobScoreboard *scoreboard) noexcept {
      auto *bytes = reinterpret_cast<std::byte *>(scoreboard);
      return reinterpret_cast<JobSlot *>(bytes - offsetof(JobSlot, scoreboard));
    }

    static JobSlot *from_fn_ptr(JobFn *fn) noexcept {
      auto *bytes = reinterpret_cast<std::byte *>(fn);
      return reinterpret_cast<JobSlot *>(bytes - offsetof(JobSlot, fn));
    }

    JobFn fn;
    JobScoreboard scoreboard;
    uint32_t generation;

    JobScheduler *scheduler;
    JobSlot *next;
  };

  static_assert(std::is_standard_layout_v<JobSlot>);

public:
  JobScheduler(const allocator &alloc, uint32_t workerCount) noexcept
      : m_alloc(alloc), m_workerPool(m_alloc, workerCount) {
    assert(workerCount > 1);
    assert(workerCount <= SLOT_COUNT);
    assert(static_cast<std::ptrdiff_t>(SLOT_COUNT) <= semaphore::max());

    m_slots = allocator_traits::template allocate<JobSlot>(m_alloc, SLOT_COUNT);

    assert(m_slots != nullptr);

    for (uint32_t i = 0; i < SLOT_COUNT; ++i) {
      std::construct_at(m_slots + i, m_alloc, this);
    }

    for (uint32_t i = 0; i + 1 < SLOT_COUNT; ++i) {
      m_slots[i].next = m_slots + i + 1;
    }

    m_slots[SLOT_COUNT - 1].next = nullptr;

    m_freeSlots.store(m_slots, std::memory_order_release);
    m_freeSlotCount.release(static_cast<std::ptrdiff_t>(SLOT_COUNT));
  }

  JobScheduler(const JobScheduler &) = delete;
  JobScheduler &operator=(const JobScheduler &) = delete;

  JobScheduler(JobScheduler &&) = delete;
  JobScheduler &operator=(JobScheduler &&) = delete;

  ~JobScheduler() noexcept {
    stop();

    if (m_slots != nullptr) {
      for (uint32_t i = 0; i < SLOT_COUNT; ++i) {
        std::destroy_at(m_slots + i);
      }

      allocator_traits::template deallocate<JobSlot>(m_alloc, m_slots,
                                                     SLOT_COUNT);

      m_slots = nullptr;
    }
  }

  job_id acquire() noexcept {
    m_freeSlotCount.acquire();

    JobSlot *slot = m_freeSlots.load(std::memory_order_acquire);

    while (true) {
      assert(slot != nullptr);

      JobSlot *next = slot->next;

      if (m_freeSlots.compare_exchange_weak(slot, next,
                                            std::memory_order_acquire,
                                            std::memory_order_relaxed)) {
        break;
      }
    }

    slot->next = nullptr;

    const uint32_t slotIndex = static_cast<uint32_t>(slot - m_slots);

    assert(slotIndex < SLOT_COUNT);

    ++slot->generation;
    assert(slot->generation != 0);

    slot->fn.release();
    slot->fn.m_liveInvocations.store(0, std::memory_order_relaxed);
    slot->fn.m_invocationCounter.store(0, std::memory_order_relaxed);

    slot->scoreboard.reset();

    job_id id;
    id.m_index = slotIndex;
    id.m_gen = slot->generation;

    return id;
  }

  void addDependency(job_id id, job_id dependency) noexcept {
    assert(id.m_index < SLOT_COUNT);
    assert(dependency.m_index < SLOT_COUNT);

    JobSlot *slot = m_slots + id.m_index;

    assert(slot->generation == id.m_gen);

    JobSlot *sourceSlot = m_slots + dependency.m_index;

    if (sourceSlot->generation == dependency.m_gen) {
      slot->scoreboard.addSource(&sourceSlot->scoreboard);
    }
  }

  template <job_fn Fn> void submit(job_id id, Fn &&fn) noexcept {
    assert(id.m_index < SLOT_COUNT);

    JobSlot *slot = m_slots + id.m_index;

    assert(slot->generation == id.m_gen);

    slot->fn.emplace(std::forward<Fn>(fn));

    if (slot->scoreboard.commit()) {
      JobFn &job = slot->fn;

      job.m_invocationCounter.store(0, std::memory_order_relaxed);

      [[maybe_unused]] const uint32_t old =
          job.m_liveInvocations.fetch_add(1, std::memory_order_relaxed);
      assert(old == 0);

      m_workerPool.submit(m_alloc, &job);
    }
  }

  template <job_fn Fn> job_id submit(Fn &&fn) noexcept {
    job_id id = acquire();
    submit(id, std::forward<Fn>(fn));
    return id;
  }

  template <job_fn Fn, std::ranges::input_range Rg>
    requires std::convertible_to<std::ranges::range_reference_t<Rg>, job_id>
  job_id submit(Fn &&fn, Rg &&dependencies) noexcept {
    job_id id = acquire();

    for (job_id dep : dependencies) {
      addDependency(id, dep);
    }

    submit(id, std::forward<Fn>(fn));

    return id;
  }

  void request_stop() noexcept { m_workerPool.request_stop(); }

  void join() noexcept { m_workerPool.join(); }

  void stop() noexcept {
    m_workerPool.request_stop();
    m_workerPool.join();
  }

private:
  allocator m_alloc;
  worker_pool m_workerPool;

  JobSlot *m_slots = nullptr;

  alignas(memory::cache_line) std::atomic<JobSlot *> m_freeSlots = nullptr;

  semaphore m_freeSlotCount{0};

public: // allocation info
  static constexpr size_t static_allocation_size(uint32_t workerCount) {
    assert(workerCount > 1);
    assert(workerCount <= SLOT_COUNT);

    size_t offset = 0;

    // m_workerPool is constructed before the JobScheduler constructor body.
    offset =
        memory::align_up(offset, worker_pool::static_allocation_alignment());
    offset += worker_pool::static_allocation_size(workerCount);

    // m_slots = allocate<JobSlot>(..., SLOT_COUNT)
    offset = memory::align_up(offset, alignof(JobSlot));
    offset += sizeof(JobSlot) * SLOT_COUNT;

    // Each JobSlot constructor constructs one JobScoreboard, which allocates
    // its ConsumerNode[MAX_DEPENDENCIES] array.
    for (uint32_t i = 0; i < SLOT_COUNT; ++i) {
      offset = memory::align_up(offset,
                                JobScoreboard::static_allocation_alignment());
      offset += JobScoreboard::static_allocation_size();
    }

    return offset;
  }

  static constexpr size_t static_allocation_alignment() {
    return std::max({
        worker_pool::static_allocation_alignment(),
        alignof(JobSlot),
        JobScoreboard::static_allocation_alignment(),
    });
  }

  static constexpr size_t dynamic_allocation_size() {
    return worker_pool::dynamic_allocation_size();
  }

  static constexpr size_t dynamic_allocation_alignment() {
    return worker_pool::dynamic_allocation_alignment();
  }
};

} // namespace strobe::ecs
