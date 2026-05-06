#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/ecs/schedule/config.hpp"
#include "strobe/ecs/schedule/allocator.hpp"
#include "strobe/ecs/schedule/cpu_relax.hpp"
#include "strobe/ecs/schedule/worker_queue.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <new>
#include <thread>

namespace strobe::ecs {

template <typename JobFn> class WorkerPool {
public:
  static constexpr uint32_t MAX_STEAL_ATTEMPTS = 2;
  static constexpr uint32_t ACTIVE_SPIN_LIMIT = 0;
  using allocator = strobe::ecs::job_allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

private:
  using Queue = WorkerQueue;
  using Pool = Queue::Pool;
  using item = Queue::item;

  struct FastRng {
    uint32_t state;

    explicit FastRng(uint32_t seed) noexcept {
      seed ^= seed >> 16;
      seed *= 0x7feb352du;
      seed ^= seed >> 15;
      seed *= 0x846ca68bu;
      seed ^= seed >> 16;

      state = seed != 0 ? seed : 1;
    }

    uint32_t bounded(uint32_t bound) noexcept {
      assert(bound > 0);

      uint32_t x = state;
      x ^= x << 13;
      x ^= x >> 17;
      x ^= x << 5;

      state = x != 0 ? x : 1;

      return static_cast<uint32_t>((uint64_t{state} * bound) >> 32);
    }
  };

  struct WorkerRecord {
    WorkerRecord(allocator alloc, uint32_t dequeCapacity,
                 uint32_t seed) noexcept
        : queue(alloc, dequeCapacity), pool(), thread(), prng(seed) {}
    Queue queue;
    Pool pool;
    std::thread thread;
    FastRng prng;
  };

public: // public types
  class Context;

  class Context {
  public:
    friend class WorkerPool;

    void submit_local(JobFn *job) noexcept {
      assert(job != nullptr);
      Queue::job *queueJob = static_cast<Queue::job *>(job);
      m_pool->m_liveInvocations.fetch_add(1, std::memory_order_relaxed);
      if (m_worker->queue.submit_local(queueJob)) {
        return;
      }
      item *node = m_worker->pool.allocate(m_alloc);
      m_worker->queue.submit_local(node, queueJob);
    }

    void submit(JobFn *job) noexcept {
      assert(job != nullptr);
      Queue::job *queueJob = static_cast<Queue::job *>(job);
      const uint32_t workerId = m_worker->prng.bounded(m_pool->m_workerCount);
      WorkerRecord *target = m_pool->m_workers + workerId;
      m_pool->m_liveInvocations.fetch_add(1, std::memory_order_relaxed);
      if (target == m_worker) {
        if (m_worker->queue.submit_local(queueJob)) {
          return;
        }
        item *node = m_worker->pool.allocate(m_alloc);
        m_worker->queue.submit_local(node, queueJob);
        return;
      }
      item *node = m_worker->pool.allocate(m_alloc);
      target->queue.submit(node, queueJob);
    }

  private:
    explicit Context(WorkerPool *pool, allocator alloc,
                     WorkerRecord *worker) noexcept
        : m_pool(pool), m_alloc(alloc), m_worker(worker) {}
    WorkerPool *m_pool;
    allocator m_alloc;
    WorkerRecord *m_worker;
  };

public: // public functions
  /// requires alloc to be stably placed!
  WorkerPool(allocator alloc, uint32_t workerCount,
             uint32_t dequeCapacity = schedule::SLOT_COUNT) noexcept
      : m_workerCount(workerCount) {
    assert(workerCount > 1);
    m_workers =
        allocator_traits::template allocate<WorkerRecord>(alloc, workerCount);
    for (uint32_t i = 0; i < workerCount; ++i) {
      std::construct_at(m_workers + i, alloc, dequeCapacity, 0x9e3779b9u + i);
    }
    // start worker threads.
    for (uint32_t i = 0; i < workerCount; ++i) {
      m_workers[i].thread =
          std::thread([this, i, alloc]() { this->worker_main(alloc, i); });
    }
  }
  WorkerPool(const WorkerPool &) = delete;
  WorkerPool &operator=(const WorkerPool &) = delete;
  WorkerPool(WorkerPool &&) = delete;
  WorkerPool &operator=(WorkerPool &&) = delete;

  ~WorkerPool() noexcept {
    request_stop();
    join();
    for (uint32_t i = 0; i < m_workerCount; ++i) {
      std::destroy_at(m_workers + i);
    }
  }

  // not-thread-safe!
  // invalid after request_stop!
  void submit(allocator alloc, JobFn *job) noexcept {
    assert(job != nullptr);
    assert(!m_stopRequested.load(std::memory_order_acquire));

    const uint32_t workerId = m_nextSubmitWorker;
    m_nextSubmitWorker = (workerId + 1 == m_workerCount) ? 0 : workerId + 1;

    item *item = m_queuePool.allocate(alloc);

    m_liveInvocations.fetch_add(1, std::memory_order_relaxed);

    WorkerRecord *record = m_workers + workerId;
    record->queue.submit(item, static_cast<Queue::job *>(job));
  }

  void request_stop() noexcept {
    m_stopRequested.store(true, std::memory_order_release);
  }

  void join() noexcept {
    for (uint32_t i = 0; i < m_workerCount; ++i) {
      if (m_workers[i].thread.joinable()) {
        m_workers[i].thread.join();
      }
    }
  }

private:
  void exec_job(JobFn *job, Context *ctx) noexcept {
    assert(job != nullptr);
    assert(ctx != nullptr);

    (*job)(ctx);
    [[maybe_unused]] const uint32_t old =
        m_liveInvocations.fetch_sub(1, std::memory_order_relaxed);
    assert(old > 0);
  }

  JobFn *try_steal(uint32_t workerId, WorkerRecord &self) noexcept {
    const uint32_t attempts =
        std::min<uint32_t>(m_workerCount - 1, MAX_STEAL_ATTEMPTS);
    uint32_t targetId = workerId + (1 + self.prng.bounded(m_workerCount - 1));
    if (targetId >= m_workerCount) {
      targetId -= m_workerCount;
    }
    for (uint32_t i = 0; i < attempts; ++i) {
      assert(targetId != workerId);
      WorkerRecord *target = m_workers + targetId;
      if (JobFn *job = static_cast<JobFn *>(target->queue.steal())) {
        return job;
      }
      ++targetId;
      if (targetId == m_workerCount) {
        targetId = 0;
      }
      if (targetId == workerId) {
        ++targetId;
        if (targetId == m_workerCount) {
          targetId = 0;
        }
      }
    }
    return nullptr;
  }

  void worker_main(allocator alloc, uint32_t workerId) noexcept {
    assert(workerId < m_workerCount);
    WorkerRecord &self = m_workers[workerId];
    Context context{this, alloc, &self};
    uint32_t idleAttempts = 0;
    while (true) {
      if (JobFn *job = static_cast<JobFn *>(self.queue.pop())) {
        idleAttempts = 0;
        exec_job(job, &context);
        continue;
      }
      if (JobFn *job = try_steal(workerId, self)) {
        idleAttempts = 0;
        exec_job(job, &context);
        continue;
      }
      if (m_stopRequested.load(std::memory_order_acquire) &&
          m_liveInvocations.load(std::memory_order_relaxed) == 0) {
        break;
      }
      if (idleAttempts < ACTIVE_SPIN_LIMIT) {
        ++idleAttempts;
        details::cpu_relax();
      } else {
        std::this_thread::yield();
      }
    }
  }

private:
  uint32_t m_workerCount;
  WorkerRecord *m_workers;
  // submit state
  Pool m_queuePool;
  uint32_t m_nextSubmitWorker = 0;
  // shutdown state
  alignas(std::hardware_destructive_interference_size)
      std::atomic<uint32_t> m_liveInvocations = 0;
  alignas(std::hardware_destructive_interference_size)
      std::atomic<bool> m_stopRequested = false;
};

} // namespace strobe::ecs
