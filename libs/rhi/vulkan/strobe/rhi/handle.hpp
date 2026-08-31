#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/align.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include "strobe/rhi/allocator.hpp"
#include "strobe/rhi/objects/object.hpp"
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <type_traits>

namespace strobe::rhi {

template <typename T> struct handle_control_block;

template <typename T> struct handle_allocator {
  friend struct handle_control_block<T>;
  using type = std::remove_cvref_t<T>;

public:
  static constexpr uint16_t handle_alignment = std::max(
      alignof(std::atomic<uint64_t>), std::max(alignof(type), alignof(void *)));
  static constexpr size_t handle_size = memory::align_up(
      memory::align_up(
          memory::align_up(sizeof(std::atomic<uint64_t>), alignof(type)) +
              sizeof(type),
          alignof(void *)) +
          sizeof(void *),
      handle_alignment);

  using upstream_alloc = strobe::rhi::allocator_ref;
  using upstream_traits = AllocatorTraits<upstream_alloc>;
  using allocator =
      MPSCMonotonicPoolResource<handle_size, handle_alignment, upstream_alloc>;
  using allocator_traits = AllocatorTraits<allocator>;

  handle_allocator(strobe::rhi::allocator_ref alloc)
      : storage(upstream_traits::template allocate<Storage>(alloc)) {
    std::construct_at(storage, std::move(alloc));
  }
  handle_allocator(const handle_allocator &o) noexcept : storage(o.storage) {
    if (storage != nullptr) {
      storage->refCount.fetch_add(1, std::memory_order_acq_rel);
    }
  }
  handle_allocator(handle_allocator &&o) noexcept
      : storage(std::exchange(o.storage, nullptr)) {}
  handle_allocator &operator=(const handle_allocator &o) noexcept {
    if (this == &o) {
      return *this;
    }
    if (o.storage != nullptr) {
      o.storage->refCount.fetch_add(1, std::memory_order_relaxed);
    }
    { // unpin
      if (storage->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        upstream_alloc tmp = std::move(storage->upstream);
        std::destroy_at(storage);
        upstream_traits::template deallocate<Storage>(tmp, storage);
      }
    }
    storage = o.storage;
    return *this;
  }
  handle_allocator &operator=(handle_allocator &&o) noexcept {
    if (this == &o) {
      return *this;
    }
    { // unpin
      if (storage->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        upstream_alloc tmp = std::move(storage->upstream);
        std::destroy_at(storage);
        upstream_traits::template deallocate<Storage>(tmp, storage);
      }
    }
    storage = std::exchange(o.storage, nullptr);
    return *this;
  }
  ~handle_allocator() noexcept {
    if (storage == nullptr) {
      return;
    }
    if (storage->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      upstream_alloc tmp = std::move(storage->upstream);
      std::destroy_at(storage);
      upstream_traits::template deallocate<Storage>(tmp, storage);
    }
  }

  handle_control_block<T> *allocate() {
    assert(storage != nullptr);
    void *ptr = allocator_traits::allocate(storage->alloc, handle_size,
                                           handle_alignment);
    return static_cast<handle_control_block<T> *>(ptr);
  }

  void deallocate(handle_control_block<T> *ptr) {
    assert(storage != nullptr);
    allocator_traits::deallocate(storage->alloc, ptr, handle_size,
                                 handle_alignment);
  }

  struct Storage {
    explicit Storage(upstream_alloc up)
        : upstream(std::move(up)), alloc(upstream) {}
    std::atomic<uint64_t> refCount{1};
    upstream_alloc upstream;
    allocator alloc;
  };
  Storage *storage;
};

template <typename T>
using handle_allocator_ref = handle_allocator<std::remove_cvref_t<T>> *;

template <typename T> struct handle_control_block {
  using type = std::remove_cvref_t<T>;
  using allocator = handle_allocator<T>;
  using allocator_storage = allocator::Storage;

  template <typename... Args>
  explicit handle_control_block(handle_allocator<T> *alloc, Args &&...args)
      : refCount{1}, value(std::forward<Args>(args)...),
        allocatorStorage(alloc->storage) {
    static_assert(allocator::handle_size >= sizeof(decltype(*this)));
    static_assert(allocator::handle_alignment >= alignof(decltype(*this)));
    assert(allocatorStorage != 0);
  }

  std::atomic<uint64_t> refCount{1};
  type value;
  allocator_storage *allocatorStorage;
};

template <typename T>
using handle = handle_control_block<std::remove_cvref_t<T>> *;

template <typename T, typename... Args>
handle<T> make_handle(handle_allocator<T> *alloc, Args &&...args) {
  handle<T> h = alloc->allocate();
  try {
    std::construct_at(h, alloc, std::forward<Args>(args)...);
  } catch (...) {
    alloc->deallocate(h);
    throw;
  }
  return h;
}

template <typename T> void pin_handle(handle<T> h) noexcept {
  assert(h != nullptr);
  h->refCount.fetch_add(1, std::memory_order_relaxed);
}

template <typename T> void unpin_handle(handle<T> h) noexcept {

  using handle_alloc = handle_allocator<T>;
  using mpsc_alloc = handle_alloc::allocator;
  using mpsc_traits = AllocatorTraits<mpsc_alloc>;
  using upstream_alloc = handle_alloc::upstream_alloc;
  using upstream_traits = AllocatorTraits<upstream_alloc>;
  using cb = handle_control_block<T>;
  using alloc_storage = handle_alloc::Storage;

  if (h == nullptr) {
    return;
  }
  if (h->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }
  // pin allocator storage
  alloc_storage *storage = h->allocatorStorage;
  assert(storage != nullptr);
  storage->refCount.fetch_add(1, std::memory_order_relaxed);
  // destroy control block
  std::destroy_at(h);
  // deallocate control block
  mpsc_traits::template deallocate<cb>(storage->alloc, h);
  // unpin allocator
  if (storage->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }
  strobe::rhi::allocator_ref upstream = std::move(storage->upstream);
  std::destroy_at(storage);
  upstream_traits::template deallocate<alloc_storage>(upstream, storage);
}

template <typename T> void pin_void_handle(void *h) noexcept {
  pin_handle<T>(static_cast<handle<T>>(h));
}
template <typename T> void unpin_void_handle(void *h) noexcept {
  unpin_handle<T>(static_cast<handle<T>>(h));
}

template <typename T> T *handle_ptr(handle<T> h) noexcept {
  assert(h != nullptr);
  return &h->value;
}

template <typename T>
const T *handle_ptr(const handle_control_block<T> *h) noexcept {
  assert(h != nullptr);
  return &h->value;
}

template <typename T, typename... Args>
void *make_void_handle(handle_allocator_ref<T> alloc, Args &&...args) {
  return make_handle<T, Args...>(alloc, std::forward<Args>(args)...);
}

template <typename T>
[[nodiscard]]
T *void_handle_ptr(void *h) noexcept {
  return handle_ptr<T>(static_cast<handle<T>>(h));
}

template <typename T>
[[nodiscard]]
const T *void_handle_ptr(const void *h) noexcept {
  return handle_ptr<T>(static_cast<const handle_control_block<T> *>(h));
}

template <typename T, typename Obj>
  requires std::is_base_of_v<Object<Obj>, Obj>
T *object_handle_ptr(const Obj &obj) {
  handle<T> h;
  std::memcpy(&h, &obj, sizeof(void *));
  assert(h);
  return &h->value;
}

template <typename T, typename Obj>
  requires std::is_base_of_v<Object<Obj>, Obj>
void *object_handle(const Obj &obj) {
  handle<T> h;
  std::memcpy(&h, &obj, sizeof(void *));
  return h;
}

} // namespace strobe::rhi
