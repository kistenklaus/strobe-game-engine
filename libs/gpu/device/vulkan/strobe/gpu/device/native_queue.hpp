#pragma once

#include "strobe/gpu/device/queue_submission.hpp"
#include "strobe/gpu/vulkan/timeline_semaphore.hpp"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <limits>
#include <mutex>

namespace strobe::gpu {

struct NativeQueue {
  static constexpr size_t SUBMISSION_CHUNK_SIZE = 16;

  ~NativeQueue() {
    strobe::gpu::allocator alloc{};

    SubmissionChunk *chunk = submissionChunks;

    while (chunk != nullptr) {
      SubmissionChunk *next = chunk->next;

      std::destroy_at(chunk);
      alloc.deallocate(chunk, sizeof(SubmissionChunk),
                       alignof(SubmissionChunk));

      chunk = next;
    }
  }

  static constexpr uint32_t EXCLUSIVE = std::numeric_limits<uint32_t>::max();
  [[nodiscard]] bool try_acquire(bool exclusive) {
    if (exclusive) {
      uint32_t expected = 0;
      return users.compare_exchange_strong(expected, EXCLUSIVE,
                                           std::memory_order_relaxed,
                                           std::memory_order_relaxed);
    }
    uint32_t current = users.load(std::memory_order_relaxed);
    while (true) {
      if (current == EXCLUSIVE) {
        return false;
      }
      if (current == EXCLUSIVE - 1) {
        return false;
      }
      if (users.compare_exchange_weak(current, current + 1,
                                      std::memory_order_relaxed,
                                      std::memory_order_relaxed)) {
        return true;
      }
    }
  }

  void release(bool exclusive) noexcept {
    if (exclusive) {
      uint32_t expected = EXCLUSIVE;
      [[maybe_unused]] const bool success = users.compare_exchange_strong(
          expected, 0, std::memory_order_relaxed, std::memory_order_relaxed);
      assert(success);
      return;
    }
    [[maybe_unused]] const uint32_t previous =
        users.fetch_sub(1, std::memory_order_relaxed);
    assert(previous != 0);
    assert(previous != EXCLUSIVE);
  }

  [[nodiscard]] uint32_t user_count() const {
    const uint32_t value = users.load(std::memory_order_relaxed);

    return value == EXCLUSIVE ? 1 : value;
  }

  [[nodiscard]] bool exclusively_owned() const {
    return users.load(std::memory_order_relaxed) == EXCLUSIVE;
  }

  [[nodiscard]] bool available() const { return static_cast<bool>(queue); }

  QueueSubmission *acquire_submission() noexcept {
    std::lock_guard lock{submissionPoolMutex};

    if (freeSubmissions == nullptr) {
      strobe::gpu::allocator alloc{};

      void *ptr =
          alloc.allocate(sizeof(SubmissionChunk), alignof(SubmissionChunk));

      auto *chunk = std::construct_at(static_cast<SubmissionChunk *>(ptr));

      chunk->next = submissionChunks;
      submissionChunks = chunk;

      for (size_t i = 0; i < SUBMISSION_CHUNK_SIZE; ++i) {
        chunk->nodes[i].next = freeSubmissions;
        freeSubmissions = &chunk->nodes[i];
      }
    }

    DequeNode *node = freeSubmissions;
    freeSubmissions = node->next;

    node->next = nullptr;
    return node;
  }

  void release_submission(QueueSubmission *submission) noexcept {
    submission->commandBuffers.clear();
    submission->binarySemaphores.clear();
    submission->timelineSemaphores.clear();
    submission->fence = {};

    DequeNode *node = node_from_submission(submission);

    std::lock_guard lock{submissionPoolMutex};

    node->next = freeSubmissions;
    freeSubmissions = node;
  }

  void commit_submission(QueueSubmission *submission) noexcept {
    // NativeQueue::mutex must already be held.
    DequeNode *node = node_from_submission(submission);
    assert(node->next == nullptr);

    node->next = nullptr;

    if (submissionTail != nullptr) {
      submissionTail->next = node;
    } else {
      submissionHead = node;
    }

    submissionTail = node;
  }

  void collect_submissions(uint64_t completed) noexcept {
    DequeNode *retiredHead = nullptr;
    DequeNode *retiredTail = nullptr;
    {
      std::lock_guard lock{mutex};
      if (submissionHead == nullptr ||
          submissionHead->timelineValue > completed) {
        return;
      }
      retiredHead = submissionHead;
      retiredTail = submissionHead;
      while (retiredTail->next != nullptr &&
             retiredTail->next->timelineValue <= completed) {
        retiredTail = retiredTail->next;
      }
      submissionHead = retiredTail->next;
      retiredTail->next = nullptr;
      if (submissionHead == nullptr) {
        submissionTail = nullptr;
      }
    }

    // Drop all retained references outside both mutexes.
    for (DequeNode *node = retiredHead; node != nullptr; node = node->next) {
      node->commandBuffers.clear();
      node->binarySemaphores.clear();
      node->timelineSemaphores.clear();
      node->fence = {};
    }

    // Return the complete retired chain to the freelist with one lock.
    {
      std::lock_guard lock{submissionPoolMutex};
      retiredTail->next = freeSubmissions;
      freeSubmissions = retiredHead;
    }
  }

  vulkan::Queue queue;
  std::mutex mutex;

  vulkan::TimelineSemaphore timeline;

  uint64_t nextTimelineValue{1};

private:
  struct DequeNode : QueueSubmission {
    DequeNode *next{nullptr};
  };

  static DequeNode *node_from_submission(QueueSubmission *submission) noexcept {
    return static_cast<DequeNode *>(submission);
  }

  // Protected by `mutex`.
  DequeNode *submissionHead{nullptr};
  DequeNode *submissionTail{nullptr};

  // Protected independently.
  std::mutex submissionPoolMutex;
  DequeNode *freeSubmissions{nullptr};

  struct SubmissionChunk {
    SubmissionChunk *next{nullptr};
    DequeNode nodes[SUBMISSION_CHUNK_SIZE];
  };
  SubmissionChunk *submissionChunks{nullptr};

  // 0         -> unused
  // 1..N      -> shared by N logical queues
  // EXCLUSIVE -> exclusively owned by one logical queue
  std::atomic<uint32_t> users{0};
};

} // namespace strobe::gpu
