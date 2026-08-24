#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe::gpu {

template <typename T, typename Alloc = strobe::Mallocator>
struct handle_control_block {
  using allocator = Alloc;
  using allocator_traits = AllocatorTraits<allocator>;

  template <typename... Args>
  explicit handle_control_block(allocator alloc, Args &&...args)
      : refCount(1), value(std::forward<Args>(args)...),
        alloc(std::move(alloc)) {}

  std::atomic<uint64_t> refCount{1};
  T value;

  [[no_unique_address]]
  allocator alloc;
};

template <typename T, typename Alloc = strobe::Mallocator>
using handle = handle_control_block<T, Alloc> *;

// -----------------------------------------------------------------------------
// allocation
// -----------------------------------------------------------------------------

template <typename T, typename Alloc = strobe::Mallocator, typename... Args>
handle<T, Alloc> alloc_handle(Alloc alloc, Args &&...args) {

  using cb = handle_control_block<T, Alloc>;
  using traits = typename cb::allocator_traits;

  cb *h = traits::template allocate<cb>(alloc);

  try {
    std::construct_at(h, alloc, std::forward<Args>(args)...);
  } catch (...) {
    traits::template deallocate<cb>(alloc, h);
    throw;
  }

  return h;
}

template <typename T, typename... Args> handle<T> make_handle(Args &&...args) {
  return alloc_handle<T>(Mallocator{}, std::forward<Args>(args)...);
}

// -----------------------------------------------------------------------------
// pinning
// -----------------------------------------------------------------------------

template <typename T, typename Alloc = strobe::Mallocator>
void pin_handle(handle<T, Alloc> h) noexcept {

  assert(h != nullptr);

  h->refCount.fetch_add(1, std::memory_order_relaxed);
}

template <typename T, typename Alloc = strobe::Mallocator>
void pin_void_handle(void *h) noexcept {

  pin_handle<T, Alloc>(static_cast<handle<T, Alloc>>(h));
}

// -----------------------------------------------------------------------------
// unpinning
// -----------------------------------------------------------------------------

template <typename T, typename Alloc = strobe::Mallocator>
void unpin_handle(handle<T, Alloc> h) noexcept {

  if (h == nullptr) {
    return;
  }

  if (h->refCount.fetch_sub(1, std::memory_order_acq_rel) != 1) {
    return;
  }

  using cb = handle_control_block<T, Alloc>;
  using traits = typename cb::allocator_traits;

  static_assert(std::is_nothrow_move_constructible_v<Alloc>,
                "Handle allocator must be nothrow move constructible");

  /*
   * The allocator is part of the control block, so copy/move
   * it out before destroying the block.
   */
  Alloc alloc = std::move(h->alloc);

  std::destroy_at(h);

  traits::template deallocate<cb>(alloc, h);
}

template <typename T, typename Alloc = strobe::Mallocator>
void unpin_void_handle(void *h) noexcept {

  unpin_handle<T, Alloc>(static_cast<handle<T, Alloc>>(h));
}

// -----------------------------------------------------------------------------
// void handle allocation
// -----------------------------------------------------------------------------

template <typename T, typename... Args> void *make_void_handle(Args &&...args) {

  return static_cast<void *>(make_handle<T>(std::forward<Args>(args)...));
}

template <typename T, typename Alloc, typename... Args>
void *alloc_void_handle(Alloc alloc, Args &&...args) {

  return static_cast<void *>(
      alloc_handle<T, Alloc>(std::move(alloc), std::forward<Args>(args)...));
}

// -----------------------------------------------------------------------------
// typed access
// -----------------------------------------------------------------------------

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
T *handle_ptr(handle<T, Alloc> h) noexcept {
  return h != nullptr ? &h->value : nullptr;
}

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
const T *handle_ptr(const handle_control_block<T, Alloc> *h) noexcept {

  return h != nullptr ? &h->value : nullptr;
}

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
T &handle_ref(handle<T, Alloc> h) noexcept {

  assert(h != nullptr);
  return h->value;
}

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
const T &handle_ref(const handle_control_block<T, Alloc> *h) noexcept {

  assert(h != nullptr);
  return h->value;
}

// -----------------------------------------------------------------------------
// void handle access
// -----------------------------------------------------------------------------

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
T *void_handle_ptr(void *h) noexcept {
#if defined(__GNUC__) && !defined(__clang__)
  if (h == nullptr) [[unlikely]]
    __builtin_unreachable();

  auto *typedHandle = static_cast<handle<T, Alloc>>(h);

  typedHandle = static_cast<handle<T, Alloc>>(__builtin_assume_aligned(
      typedHandle, alignof(handle_control_block<T, Alloc>)));

  auto *ptr = handle_ptr<T, Alloc>(typedHandle);

  if (ptr == nullptr) [[unlikely]]
    __builtin_unreachable();

  ptr = static_cast<T *>(__builtin_assume_aligned(ptr, alignof(T)));

  return __builtin_launder(ptr);
#else
  return handle_ptr<T, Alloc>(static_cast<handle<T, Alloc>>(h));
#endif
}

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
const T *void_handle_ptr(const void *h) noexcept {

  return handle_ptr<T, Alloc>(
      static_cast<const handle_control_block<T, Alloc> *>(h));
}

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
T &void_handle_ref(void *h) noexcept {

  return handle_ref<T, Alloc>(static_cast<handle<T, Alloc>>(h));
}

template <typename T, typename Alloc = strobe::Mallocator>
[[nodiscard]]
const T &void_handle_ref(const void *h) noexcept {

  return handle_ref<T, Alloc>(
      static_cast<const handle_control_block<T, Alloc> *>(h));
}

} // namespace strobe::gpu
