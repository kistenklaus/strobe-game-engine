#pragma once

#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/core/memory/sync_resource.hpp"
#include "strobe/ecs/allocator.hpp"
#include <concepts>
#include <random>
#include <thread>
namespace strobe::ecs {

template <typename Job>
  requires std::invocable<Job>
class WorkerPool {
public:
  using allocator = SyncResource<MonotonicResource<strobe::ecs::allocator_ref>>;
  using allocator_traits = AllocatorTraits<allocator>;

  WorkerPool(const strobe::ecs::allocator_ref &alloc, uint32_t workerCount,
             uint32_t workerDequeCapacity = 1024) noexcept
      : m_alloc(std::in_place, alloc), m_workerCount(workerCount) {
    assert(workerCount > 0);

    m_workers = allocator_traits::template allocate<WorkerRecord>(
        m_alloc, m_workerCount);
    assert(m_workers != nullptr);

    for (uint32_t i = 0; i < m_workerCount; ++i) {
      std::construct_at(m_workers + i, i, m_alloc, workerDequeCapacity);
    }
    for (uint32_t i = 0; i < m_workerCount; ++i) {
      m_workers[i].handle = std::thread([this, i]() { this->worker_entry(i); });
    }
  }
  ~WorkerPool() {
    stop();

    if (m_workers != nullptr) {
      for (uint32_t i = 0; i < m_workerCount; ++i) {
        std::destroy_at(m_workers + i);
      }

      allocator_traits::template deallocate<WorkerRecord>(m_alloc, m_workers,
                                                          m_workerCount);

      m_workers = nullptr;
      m_workerCount = 0;
    }
  }
  
  // not thread-safe! (called from one thread!)
  void stop() {

  }

  // not thread-safe! (called from one thread!)
  void issue(Job *job) {
    
  }

private:
  struct WorkerInboxMPSC {
    struct Node {
      Node *next = nullptr;
      Job *job = nullptr;
    };

    WorkerInboxMPSC() = default;
    WorkerInboxMPSC(const WorkerInboxMPSC &) = delete;
    WorkerInboxMPSC &operator=(const WorkerInboxMPSC &) = delete;

    // multi-producer
    void push(Node *node, Job *record) noexcept {
      assert(node != nullptr);
      assert(record != nullptr);
      node->record = record;
      node->next = nullptr;
      push_list(node, node);
    }

    // multi-producer
    void push_list(Node *first, Node *last) noexcept {
      assert(first != nullptr);
      assert(last != nullptr);
      assert(last->next == nullptr);
      Node *head = m_head.load(std::memory_order_relaxed);
      do {
        last->next = head;
      } while (!m_head.compare_exchange_weak(
          head, first, std::memory_order_release, std::memory_order_relaxed));
    }

    // single-consumer.
    Node *take_all() noexcept {
      return m_head.exchange(nullptr, std::memory_order_acquire);
    }

  private:
    alignas(std::hardware_destructive_interference_size)
        std::atomic<Node *> m_head = nullptr;
  };
  struct WorkerDequeSPMC {
    explicit WorkerDequeSPMC(allocator &alloc, uint32_t capacity) noexcept {
      assert(capacity > 0);

      capacity = std::bit_ceil(capacity);
      assert(capacity >= 2);

      m_capacity = capacity;
      m_mask = capacity - 1;

      m_ringbuffer =
          allocator_traits::template allocate<Job *>(alloc, m_capacity);

      assert(m_ringbuffer != nullptr);

      for (uint32_t i = 0; i < m_capacity; ++i) {
        std::construct_at(m_ringbuffer + i, nullptr);
      }

      m_top.store(0, std::memory_order_relaxed);
      m_bottom.store(0, std::memory_order_relaxed);
    }

    WorkerDequeSPMC(const WorkerDequeSPMC &) = delete;
    WorkerDequeSPMC &operator=(const WorkerDequeSPMC &) = delete;
    WorkerDequeSPMC(WorkerDequeSPMC &&) = delete;
    WorkerDequeSPMC &operator=(WorkerDequeSPMC &&) = delete;

    // If backing allocator is monotonic, this does not actually release memory,
    // but it keeps object lifetime correct.
    ~WorkerDequeSPMC() {
      if (m_ringbuffer != nullptr) {
        for (uint32_t i = 0; i < m_capacity; ++i) {
          std::destroy_at(m_ringbuffer + i);
        }

        // Optional with monotonic resource, but interface-correct if allocator
        // later changes.
        // allocator_traits::template deallocate<JobRecord *>(
        //     alloc, m_ringbuffer, m_capacity);
      }
    }

    // Owner only.
    bool push(Job *job) noexcept {
      assert(job != nullptr);

      const uint64_t bottom = m_bottom.load(std::memory_order_relaxed);
      const uint64_t top = m_top.load(std::memory_order_acquire);

      if (bottom - top >= m_capacity) {
        return false;
      }

      m_ringbuffer[bottom & m_mask] = job;
      m_bottom.store(bottom + 1, std::memory_order_release);

      return true;
    }

    // Owner only.
    bool pop(Job *&out) noexcept {
      uint64_t bottom = m_bottom.load(std::memory_order_relaxed);
      uint64_t top = m_top.load(std::memory_order_relaxed);

      if (bottom <= top) {
        return false;
      }

      bottom -= 1;
      m_bottom.store(bottom, std::memory_order_relaxed);

      std::atomic_thread_fence(std::memory_order_seq_cst);

      top = m_top.load(std::memory_order_relaxed);

      const int64_t size =
          static_cast<int64_t>(bottom) - static_cast<int64_t>(top);

      if (size < 0) {
        m_bottom.store(top, std::memory_order_relaxed);
        return false;
      }

      Job *job = m_ringbuffer[bottom & m_mask];

      if (size > 0) {
        out = job;
        return true;
      }

      if (!m_top.compare_exchange_strong(top, top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed)) {
        m_bottom.store(top + 1, std::memory_order_relaxed);
        return false;
      }

      m_bottom.store(top + 1, std::memory_order_relaxed);
      out = job;
      return true;
    }

    // Thieves only.
    bool steal(Job *&out) noexcept {
      uint64_t top = m_top.load(std::memory_order_acquire);

      std::atomic_thread_fence(std::memory_order_seq_cst);

      const uint64_t bottom = m_bottom.load(std::memory_order_acquire);

      const int64_t size =
          static_cast<int64_t>(bottom) - static_cast<int64_t>(top);

      if (size <= 0) {
        return false;
      }

      Job *job = m_ringbuffer[top & m_mask];

      if (!m_top.compare_exchange_strong(top, top + 1,
                                         std::memory_order_seq_cst,
                                         std::memory_order_relaxed)) {
        return false;
      }

      out = job;
      return true;
    }

    uint32_t capacity() const noexcept { return m_capacity; }

  private:
    uint32_t m_capacity = 0;
    uint32_t m_mask = 0;

    alignas(std::hardware_destructive_interference_size)
        std::atomic<uint64_t> m_top{0};

    alignas(std::hardware_destructive_interference_size)
        std::atomic<uint64_t> m_bottom{0};

    Job **m_ringbuffer = nullptr;
  };

  struct WorkerQueue {
  public:
    using Node = WorkerInboxMPSC::Node;

    explicit WorkerQueue(allocator &alloc, uint32_t deque_capacity)
        : m_inbox(), m_deque(alloc, deque_capacity) {}

    WorkerQueue(const WorkerQueue &) = delete;
    WorkerQueue &operator=(const WorkerQueue &) = delete;

    WorkerQueue(WorkerQueue &&) = delete;
    WorkerQueue &operator=(WorkerQueue &&) = delete;

    // Remote/external producer.
    void push(Node *node, Job *job) noexcept {
      assert(node != nullptr);
      assert(job != nullptr);

      m_inbox.push(node, job);
    }

    // Owner only.
    Node *drain_inbox() noexcept {
      Node *list = m_inbox.take_all();
      Node *recycled = nullptr;
      while (list != nullptr) {
        Node *node = list;
        list = list->next;
        node->next = nullptr;
        if (m_deque.push(node->record)) {
          node->record = nullptr;
          node->next = recycled;
          recycled = node;
          continue;
        }
        node->next = list;
        Node *last = node;
        while (last->next != nullptr) {
          last = last->next;
        }
        m_inbox.push_list(node, last);
        break;
      }
      return recycled;
    }

    // Owner only.
    Job *pop() noexcept {
      Job *out = nullptr;
      return m_deque.pop(out) ? out : nullptr;
    }

    // Thieves only.
    Job *steal() noexcept {
      Job *out = nullptr;
      return m_deque.steal(out) ? out : nullptr;
    }

  private:
    WorkerInboxMPSC m_inbox;
    WorkerDequeSPMC m_deque;
  };

  struct WorkerNodePool {
    static constexpr std::size_t CHUNK_SIZE = 128;
    using Node = WorkerInboxMPSC::Node;

    WorkerNodePool() = default;

    WorkerNodePool(const WorkerNodePool &) = delete;
    WorkerNodePool &operator=(const WorkerNodePool &) = delete;

    WorkerNodePool(WorkerNodePool &&) = delete;
    WorkerNodePool &operator=(WorkerNodePool &&) = delete;

    // Owner only.
    Node *pop(allocator &alloc) {
      if (m_freeNodes == nullptr) {
        Node *chunk =
            allocator_traits::template allocate<Node>(alloc, CHUNK_SIZE);

        assert(chunk != nullptr);

        for (std::size_t i = 0; i < CHUNK_SIZE; ++i) {
          std::construct_at(chunk + i);

          chunk[i].record = nullptr;
          chunk[i].next = (i + 1 < CHUNK_SIZE) ? (chunk + i + 1) : nullptr;
        }

        m_freeNodes = chunk;
      }

      Node *node = m_freeNodes;
      assert(node != nullptr);

      m_freeNodes = node->next;

      node->next = nullptr;
      node->record = nullptr;

      return node;
    }

    // Owner only.
    void push(Node *nodes) noexcept {
      if (nodes == nullptr) {
        return;
      }

      Node *last = nodes;

      while (last->next != nullptr) {
        last->record = nullptr;
        last = last->next;
      }

      last->record = nullptr;
      last->next = m_freeNodes;
      m_freeNodes = nodes;
    }

  private:
    Node *m_freeNodes = nullptr;
  };

  struct alignas(std::hardware_destructive_interference_size) WorkerRecord {
    using Node = WorkerInboxMPSC::Node;

    WorkerRecord(uint32_t id, allocator &alloc, uint32_t deque_capacity)
        : thread_id(id), handle(), queue(alloc, deque_capacity), nodes() {}

    WorkerRecord(const WorkerRecord &) = delete;
    WorkerRecord &operator=(const WorkerRecord &) = delete;

    WorkerRecord(WorkerRecord &&) = delete;
    WorkerRecord &operator=(WorkerRecord &&) = delete;

    uint32_t thread_id = 0;
    std::thread handle;

    WorkerQueue queue;
    WorkerNodePool nodes;
  };

  Job *steal_job(uint32_t self_id, std::mt19937 &rng) noexcept {
    if (m_workerCount <= 1) {
      return nullptr;
    }

    std::uniform_int_distribution<uint32_t> dist(1, m_workerCount - 1);

    const uint32_t start_offset = dist(rng);

    for (uint32_t i = 0; i < m_workerCount - 1; ++i) {
      const uint32_t victim = (self_id + start_offset + i) % m_workerCount;

      assert(victim != self_id);

      if (Job *job = m_workers[victim].queue.steal()) {
        return job;
      }
    }

    return nullptr;
  }

  void exec_job(Job *job, WorkerRecord *worker) {
    (*job)();
    // NOTE/TODO: Do we need to do anything else here, like returning
    // memory?? once the job is done?
  }

  void worker_entry(uint32_t workerId) {
    WorkerRecord &self = m_workers[workerId];

    std::mt19937 prng(workerId);

    while (true) {
      if (Job *job = self.queue.pop()) {
        exec_job(job, self);
        continue;
      }

      self.nodes.push(self.queue.drain_inbox());

      if (Job *job = self.queue.pop()) {
        exec_job(job, self);
        continue;
      }

      if (Job *job = steal_job(workerId, prng)) {
        exec_job(job, self);
        continue;
      }

      if (m_stopRequested.load(std::memory_order_acquire) &&
          m_liveJobs.load(std::memory_order_acquire) == 0) {
        break;
      }

      std::this_thread::yield();
    }
  }

private:
  allocator m_alloc;
  uint32_t m_workerCount;
  WorkerRecord *m_workers;

  std::atomic<bool> m_stopRequested = false;
  std::atomic<uint32_t> m_liveJobs = 0;
};

} // namespace strobe::ecs
