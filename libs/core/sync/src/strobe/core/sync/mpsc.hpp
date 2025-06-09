#pragma once

#include <atomic>
#include <strobe/core/memory/ReferenceCounter.hpp>
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
    std::size_t flexibleBufferSize = capacity;
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
    auto bufferPtr = reinterpret_cast<FreelistNode*>(raw + flexibleBufferOffset);
    new (allocPtr) A(std::move(alloc));

    new (controlBlock)
        ChannelStateControlBlock(flexibleBufferSize, bufferPtr, [](void *self) {
          auto controlBlock =
              reinterpret_cast<ChannelStateControlBlock *>(self);
          // destruct queue (i.e. destruct all pending messages)
          std::size_t flexMPSCBufferSize = controlBlock->m_flexBufferSize;
          std::size_t offset = sizeof(ChannelStateControlBlock<T>);
          offset = memory::align_up(offset, alignof(ChannelStateControlBlock<T>));
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
    // std::println("Free control block");

    controlBlock->m_deleter(controlBlock);
  }

  bool incRefCount() { return m_referenceCounter.inc(); }

  bool decRefCount() { return m_referenceCounter.dec(); }

  auto &queue() { return m_flexMPSC; }

  auto waitUntilSend() {
    m_sendFlag.store(false, std::memory_order_relaxed);
    m_sendFlag.wait(false, std::memory_order_acquire);
  }

  auto notifySend() {
    m_sendFlag.store(true, std::memory_order_release);
    m_sendFlag.notify_one();
  }

  auto waitUntilRecv() {
    m_recvFlag.store(false, std::memory_order_relaxed);
    m_recvFlag.wait(false, std::memory_order_acquire);
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
    // std::println("Created control block 0x{:X}",
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
    // std::println("COPY construct");
    if (m_controlBlock != nullptr && !m_controlBlock->incRefCount()) {
      m_controlBlock = nullptr; // Failed to increment ref counter.
    }
  }

  SharedChannelState &operator=(const SharedChannelState &o) {
    // std::println("COPY assign");
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
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {
    // std::println("MOVE construct");
  }

  SharedChannelState &operator=(SharedChannelState &&o) {
    // std::println("MOVE assign");
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
    // std::println("Destruct Shared {}",
    // reinterpret_cast<std::size_t>(m_controlBlock));
    if (m_controlBlock != nullptr && m_controlBlock->decRefCount()) {
      ChannelStateControlBlock<T>::free(m_controlBlock);
      m_controlBlock = nullptr;
    }
  }

  bool enqueue(const T &v) const { return m_controlBlock->queue().enqueue(v); }

  bool enqueue(T &&v) const {
    assert(m_controlBlock != nullptr);
    return m_controlBlock->queue().enqueue(std::move(v));
  }

  void blocking_enqueue(const T &v) const {
    while (!m_controlBlock->queue().enqueue(v)) {
      m_controlBlock->waitUntilRecv();
    }
    m_controlBlock->notifySend();
  }

  void blocking_enqueue(T &&v) const {
    while (!m_controlBlock->queue().enqueue(std::move(v))) {
      m_controlBlock->waitUntilRecv();
    }
    m_controlBlock->notifySend();
  }

  T blocking_dequeue() const {
    std::optional<T> v;
    while (!(v = m_controlBlock->queue().dequeue()).has_value()) {
      m_controlBlock->waitUntilSend();
    }
    m_controlBlock->notifyRecv();
    return *v;
  }

  std::optional<T> dequeue() const { return m_controlBlock->queue().dequeue(); }

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
  void blocking_send(T &&v) const { m_state.blocking_enqueue(std::move(v)); }

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
