#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) ||               \
    defined(_M_IX86)
#include <emmintrin.h>
#endif

#include <atomic>
#include <strobe/core/memory/ReferenceCounter.hpp>
#include <thread>
#include <utility>

#include "strobe/core/memory/align.hpp"
#include "strobe/core/sync/lockfree/lock_free_flexible_ms_queue.hpp"
#include <strobe/core/memory/AllocatorTraits.hpp>

namespace strobe::mpsc {

namespace details {

template <typename T> class ChannelStateControlBlock {
  using Rc = memory::ReferenceCounter<std::size_t>;
  typedef void (*Deleter)(void *self);
  using Queue = sync::LockFreeFlexibleMSQueue<T>;

public:
  template <Allocator A>
  static ChannelStateControlBlock<T> *make(const A &allocator,
                                           std::size_t capacity) {
    std::size_t flexibleBufferSize = capacity + 1;
    std::size_t flexibleBufferOffset = sizeof(ChannelStateControlBlock<T>);

    // NOTE should be a noop
    using FreelistNode = Queue::FreelistNode;

    flexibleBufferOffset =
        memory::align_up(flexibleBufferOffset, alignof(FreelistNode));

    std::size_t allocatorOffset =
        flexibleBufferOffset + (flexibleBufferSize) * sizeof(FreelistNode);

    allocatorOffset = memory::align_up(allocatorOffset, alignof(A));

    std::size_t byteSize = allocatorOffset + sizeof(A);

    using ATraits = AllocatorTraits<A>;
    A alloc = allocator;
    auto ptr = ATraits::allocate(alloc, byteSize,
                                 alignof(ChannelStateControlBlock<T>));
    auto controlBlock = reinterpret_cast<ChannelStateControlBlock<T> *>(ptr);
    auto raw = reinterpret_cast<std::byte *>(ptr);
    auto allocPtr = reinterpret_cast<A *>(raw + allocatorOffset);
    auto bufferPtr =
        reinterpret_cast<FreelistNode *>(raw + flexibleBufferOffset);
    new (allocPtr) A(std::move(alloc));

    new (controlBlock)
        ChannelStateControlBlock(flexibleBufferSize, bufferPtr, [](void *self) {
          auto controlBlock =
              reinterpret_cast<ChannelStateControlBlock *>(self);
          // destruct queue (i.e. destruct all pending messages)
          std::size_t flexMPSCBufferSize = controlBlock->m_flexBufferSize;
          std::size_t offset = sizeof(ChannelStateControlBlock<T>);
          offset =
              memory::align_up(offset, alignof(ChannelStateControlBlock<T>));
          offset = memory::align_up(offset, alignof(FreelistNode));
          offset = offset + sizeof(FreelistNode) * flexMPSCBufferSize;
          offset = memory::align_up(offset, alignof(A));
          std::size_t byteSize = offset + sizeof(A);
          std::byte *raw = reinterpret_cast<std::byte *>(self);
          A *allocPtr = reinterpret_cast<A *>(raw + offset);
          A alloc = *allocPtr; // copy allocator out of control buffer.
          std::destroy_at(&controlBlock);
          std::destroy_at(allocPtr);
          ATraits::deallocate(alloc, self, byteSize,
                              alignof(ChannelStateControlBlock<T>));
        });
    return controlBlock;
  }

  static void free(ChannelStateControlBlock<T> *controlBlock) {

    controlBlock->m_deleter(controlBlock);
  }

  bool incRefCount() { return m_referenceCounter.inc(); }

  bool decRefCount() { return m_referenceCounter.dec(); }

  auto &queue() { return m_flexMPSC; }

  auto waitUntilSend() {
    while (!m_sendFlag.exchange(false, std::memory_order_acquire)) {
      m_sendFlag.wait(false, std::memory_order_acquire);
    }
  }

  auto notifySend() {
    m_sendFlag.store(true, std::memory_order_release);
    m_sendFlag.notify_one();
  }

  auto waitUntilRecv() {
    while (!m_recvFlag.exchange(false, std::memory_order_acquire)) {
      m_recvFlag.wait(false, std::memory_order_acquire);
    }
  }

  auto notifyRecv() {
    m_recvFlag.store(true, std::memory_order_release);
    m_recvFlag.notify_one();
  }

  ChannelStateControlBlock(const ChannelStateControlBlock &o) = delete;
  ChannelStateControlBlock &
  operator=(const ChannelStateControlBlock &o) = delete;
  ChannelStateControlBlock(ChannelStateControlBlock &&o) = delete;
  ChannelStateControlBlock &operator=(ChannelStateControlBlock &&o) = delete;

private:
  ~ChannelStateControlBlock() = delete;
  ChannelStateControlBlock(std::size_t flexibleBufferSize,
                           Queue::FreelistNode *buffer, Deleter deleter)
      : m_referenceCounter(), m_deleter(deleter),
        m_flexBufferSize(flexibleBufferSize), m_recvFlag(true),
        m_sendFlag(false), m_flexMPSC(flexibleBufferSize, buffer) {
    // reinterpret_cast<std::size_t>(this));
  }

  Rc m_referenceCounter;
  Deleter m_deleter;
  std::size_t m_flexBufferSize;
  std::atomic<bool> m_recvFlag;
  std::atomic<bool> m_sendFlag;
  Queue m_flexMPSC; // flexible container
  // <- A type erased allocator is stored after the flexible container.
};

template <typename T> struct SharedChannelState {
  template <Allocator A>
  SharedChannelState(const A &alloc, std::size_t capacity)
      : m_controlBlock(ChannelStateControlBlock<T>::make(alloc, capacity)) {
    assert(m_controlBlock != nullptr);
  }

  SharedChannelState(const SharedChannelState &o)
      : m_controlBlock(o.m_controlBlock) {
    if (m_controlBlock != nullptr && !m_controlBlock->incRefCount()) {
      m_controlBlock = nullptr; // Failed to increment ref counter.
    }
  }

  SharedChannelState &operator=(const SharedChannelState &o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    if (m_controlBlock != nullptr && m_controlBlock->decRefCount()) {
      ChannelStateControlBlock<T>::free(m_controlBlock);
    }
    m_controlBlock = o.m_controlBlock;
    if (m_controlBlock != nullptr && !m_controlBlock->incRefCount()) {
      m_controlBlock = nullptr; // Failed to increment ref counter.
    }
    return *this;
  }

  SharedChannelState(SharedChannelState &&o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}

  SharedChannelState &operator=(SharedChannelState &&o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    if (m_controlBlock != nullptr && m_controlBlock->decRefCount()) {
      ChannelStateControlBlock<T>::free(m_controlBlock);
    }
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
    return *this;
  }

  ~SharedChannelState() {
    // reinterpret_cast<std::size_t>(m_controlBlock));
    if (m_controlBlock != nullptr && m_controlBlock->decRefCount()) {
      ChannelStateControlBlock<T>::free(m_controlBlock);
      m_controlBlock = nullptr;
    }
  }

  bool enqueue(const T &v) const {
    assert(m_controlBlock != nullptr);
    auto x = m_controlBlock->queue().enqueue(v);
    if (x) {
      m_controlBlock->notifySend();
    }
    return x;
  }

  bool enqueue(T &&v) const {
    assert(m_controlBlock != nullptr);
    auto x = m_controlBlock->queue().enqueue(std::move(v));
    if (x) {
      m_controlBlock->notifySend();
    }
    return x;
  }
  static inline void cpu_relax() noexcept {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) ||               \
    defined(_M_IX86)
    _mm_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
    // Use yield on ARM64
    __asm__ volatile("yield" ::: "memory");
#elif defined(__arm__) || defined(_M_ARM)
    __asm__ volatile("yield" ::: "memory");
#else
    // Fallback
    std::this_thread::yield();
#endif
  }

  void blocking_enqueue(const T &v) const {

    std::size_t it = 0;
#ifdef _MSC_VER
    while (!m_controlBlock->queue().enqueue(v) && ++it < 100) {
      if (it < 10) {
        // Tight spin
        cpu_relax();
      } else {
        // Cooperative yield
        std::this_thread::yield();
      }
    }
    if (it >= 100) {
      while (!m_controlBlock->queue().enqueue(v)) {
        m_controlBlock->waitUntilRecv();
      }
    }
#else
    while (!m_controlBlock->queue().enqueue(v)) {
      m_controlBlock->waitUntilRecv();
    }
#endif

    m_controlBlock->notifySend();
  }

  T blocking_dequeue() const {
    std::optional<T> v;

#ifdef _MSC_VER
    std::size_t it = 0;
    while (!(v = m_controlBlock->queue().dequeue()).has_value() && ++it < 100) {
      if (it < 10) {
        // Tight spin
        cpu_relax();
      } else {
        // Cooperative yield
        std::this_thread::yield();
      }
    }

    if (!v.has_value()) {
      while (!(v = m_controlBlock->queue().dequeue()).has_value()) {
        m_controlBlock->waitUntilSend();
      }
    }
#else
    while (!(v = m_controlBlock->queue().dequeue()).has_value()) {
      m_controlBlock->waitUntilSend();
    }
#endif
    m_controlBlock->notifyRecv();
    return *v;
  }

  std::optional<T> dequeue() const {
    auto x = m_controlBlock->queue().dequeue();
    if (x) {
      m_controlBlock->notifyRecv();
    }
    return x;
  }

private:
  ChannelStateControlBlock<T> *m_controlBlock;
};

}; // namespace details

template <typename T> class Sender {
public:
  explicit Sender(details::SharedChannelState<T> state)
      : m_state(std::move(state)) {}

  bool send(const T &v) const { return m_state.enqueue(v); }
  bool send(T &&v) const { return m_state.enqueue(std::move(v)); }
  void blocking_send(const T &v) const { m_state.blocking_enqueue(v); }

private:
  details::SharedChannelState<T> m_state;
};

template <typename T> class Receiver {
public:
  explicit Receiver(details::SharedChannelState<T> state)
      : m_state(std::move(state)) {}
  Receiver() = default;

  Receiver(const Receiver &) = delete;
  Receiver &operator=(const Receiver &) = delete;
  Receiver(Receiver &&) = default;
  Receiver &operator=(Receiver &&) = default;

  std::optional<T> recv() const { return m_state.dequeue(); }
  T blocking_recv() const { return m_state.blocking_dequeue(); }

private:
  details::SharedChannelState<T> m_state;
};

template <typename T, std::size_t Capacity, Allocator A>
static std::pair<Sender<T>, Receiver<T>> channel(const A &allocator = {}) {
  auto state = details::SharedChannelState<T>(allocator, Capacity);
  auto tx = Sender(state);
  return std::pair{std::move(tx), Receiver(std::move(state))};
}

} // namespace strobe::mpsc
