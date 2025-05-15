#pragma once

#include <strobe/core/memory/ReferenceCounter.hpp>
#include <utility>

#include "strobe/core/memory/align.hpp"
#include "strobe/core/memory/smart_pointers/SharedPtr.hpp"
#include "strobe/core/memory/smart_pointers/SharedResource.hpp"
#include "strobe/core/sync/lockfree/spsc_ring_buffer.hpp"

namespace strobe::spsc {

namespace details {

template <typename T>
class ChannelStateControlBlock {
  using Rc = memory::ReferenceCounter<std::size_t>;
  typedef void (*Deleter)(void* self);

 public:
  template <Allocator A>
  static ChannelStateControlBlock<T>* make(const A& allocator,
                                           std::size_t capacity) {
    std::size_t flexibleBufferSize = capacity + 1;
    std::size_t flexibleBufferOffset = sizeof(ChannelStateControlBlock<T>);
    flexibleBufferOffset = memory::align_up(
        flexibleBufferOffset, alignof(ChannelStateControlBlock<T>));
    std::size_t allocatorOffset =
        flexibleBufferOffset + flexibleBufferSize * sizeof(T);
    allocatorOffset = memory::align_up(allocatorOffset, alignof(A));

    std::size_t byteSize = allocatorOffset + sizeof(A);

    using ATraits = AllocatorTraits<A>;
    A alloc = allocator;
    auto ptr = ATraits::allocate(alloc, byteSize,
                                 alignof(ChannelStateControlBlock<T>));
    auto controlBlock = reinterpret_cast<ChannelStateControlBlock<T>*>(ptr);
    auto raw = reinterpret_cast<std::byte*>(ptr);
    auto allocPtr = reinterpret_cast<A*>(raw + allocatorOffset);
    new (allocPtr) A(std::move(alloc));

    new (controlBlock)
        ChannelStateControlBlock(flexibleBufferSize, [](void* self) {
          auto controlBlock = reinterpret_cast<ChannelStateControlBlock*>(self);
          // destruct queue (i.e. destruct all pending messages)
          std::size_t flexMPSCBufferSize =
              controlBlock->m_flexMPSC.bufferSize();
          controlBlock->m_flexMPSC.~FlexibleLockFreeSPSCRingBuffer();
          std::size_t offset = sizeof(ChannelStateControlBlock<T>);
          offset =
              memory::align_up(offset, alignof(ChannelStateControlBlock<T>));
          offset = offset + sizeof(T) * flexMPSCBufferSize;
          offset = memory::align_up(offset, alignof(A));
          std::size_t byteSize = offset + sizeof(A);
          std::byte* raw = reinterpret_cast<std::byte*>(self);
          A* allocPtr = reinterpret_cast<A*>(raw + offset);
          A alloc = *allocPtr;  // copy allocator out of control buffer.
          ATraits::deallocate(alloc, self, byteSize,
                              alignof(ChannelStateControlBlock<T>));
        });
    return controlBlock;
  }

  static void free(ChannelStateControlBlock<T>* controlBlock) {
    // std::println("Free control block");

    controlBlock->m_deleter(controlBlock);
  }

  bool close() {
    bool expected = false;
    return !m_closed.compare_exchange_strong(expected, true);
  }

  auto& queue() { return m_flexMPSC; }

  ChannelStateControlBlock(const ChannelStateControlBlock& o) = delete;
  ChannelStateControlBlock& operator=(const ChannelStateControlBlock& o) =
      delete;
  ChannelStateControlBlock(ChannelStateControlBlock&& o) = delete;
  ChannelStateControlBlock& operator=(ChannelStateControlBlock&& o) = delete;

 private:
  ~ChannelStateControlBlock() = delete;
  ChannelStateControlBlock(std::size_t flexibleBufferSize, Deleter deleter)
      : m_closed(false), m_deleter(deleter), m_flexMPSC(flexibleBufferSize) {
    // std::println("Created control block 0x{:X}",
    // reinterpret_cast<std::size_t>(this));
  }

  std::atomic<bool> m_closed;
  Deleter m_deleter;
  sync::FlexibleLockFreeSPSCRingBuffer<T> m_flexMPSC;  // flexible container
  // <- A type erased allocator is stored after the flexible container.
};

};  // namespace details

template <typename T>
class Sender {
 public:
  explicit Sender(details::ChannelStateControlBlock<T>* state)
      : m_controlBlock(state) {}

  ~Sender() {
    if (m_controlBlock != nullptr && m_controlBlock->close()) {
      details::ChannelStateControlBlock<T>::free(m_controlBlock);
    }
  }

  Sender(const Sender&) = delete;
  Sender& operator=(const Sender&) = delete;
  Sender(Sender&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}
  Sender& operator=(Sender&& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    if (m_controlBlock != nullptr) {
      if (m_controlBlock->close()) {
        details::ChannelStateControlBlock<T>::free(m_controlBlock);
      }
    }
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
  }

  bool send(const T& v) const { return m_controlBlock->queue().enqueue(v); }
  bool send(T&& v) const {
    return m_controlBlock->queue().enqueue(std::move(v));
  }

 private:
  details::ChannelStateControlBlock<T>* m_controlBlock;
};

template <typename T>
class SharedReceiver;

template <typename T>
class Receiver {
 public:
  friend class SharedReceiver<T>;
  explicit Receiver(details::ChannelStateControlBlock<T>* state)
      : m_controlBlock(std::move(state)) {}

  ~Receiver() {
    if (m_controlBlock != nullptr && m_controlBlock->close()) {
      details::ChannelStateControlBlock<T>::free(m_controlBlock);
    }
  }

  Receiver(const Receiver&) = delete;
  Receiver& operator=(const Receiver&) = delete;
  Receiver(Receiver&& o)
      : m_controlBlock(std::exchange(o.m_controlBlock, nullptr)) {}
  Receiver& operator=(Receiver&& o) {
    if (m_controlBlock == o.m_controlBlock) {
      return *this;
    }
    if (m_controlBlock != nullptr) {
      if (m_controlBlock->close()) {
        details::ChannelStateControlBlock<T>::free(m_controlBlock);
      }
    }
    m_controlBlock = std::exchange(o.m_controlBlock, nullptr);
  }

  std::optional<T> recv() const { return m_controlBlock->queue().dequeue(); }

 private:
  details::ChannelStateControlBlock<T>* m_controlBlock;
};

template <typename T, std::size_t Capacity, Allocator A>
static std::pair<Sender<T>, Receiver<T>> channel(std::size_t capacity,
                                                 const A& allocator = {}) {
  auto state = details::ChannelStateControlBlock<T>::make(allocator, capacity);
  return std::make_pair(Sender(state), Receiver(state));
}

template <typename T>
class SharedReceiver {
 private:
  class SharedReceiverDestructor {
    void operator()(details::ChannelStateControlBlock<T>*& controlBlock) {
      if (controlBlock != nullptr && controlBlock->close()) {
        details::ChannelStateControlBlock<T>::free(controlBlock);
      }
    }
  };

 public:
  template <Allocator A>
  SharedReceiver(Receiver<T> receiver, A alloc = {}) {
    details::ChannelStateControlBlock<T>* controlBlock =
        std::exchange(receiver.m_controlBlock, nullptr);
    m_controlBlock = makeSharedResource<T, A, SharedReceiverDestructor>(
        controlBlock, std::move(alloc));
  }

  std::optional<T> recv() const { 
    return m_controlBlock->queue().dequeue(); 
  }

 private:
  SharedResource<details::ChannelStateControlBlock<T>> m_controlBlock;
};

}  // namespace strobe::spsc
