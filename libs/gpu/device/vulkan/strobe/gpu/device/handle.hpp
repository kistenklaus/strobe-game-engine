#pragma once

#include <atomic>
#include <cassert>
#include <cstdint>
namespace strobe::gpu {

template <typename T> struct handle_control_block {
  template <typename... Args>
  explicit handle_control_block(Args &&...args)
      : refCount(1), value(std::forward<Args>(args)...) {}

  std::atomic<uint64_t> refCount{1};
  T value;
};
template <typename T> using handle = handle_control_block<T> *;

template <typename T, typename... Args> handle<T> make_handle(Args &&...args) {
  return new handle_control_block<T>(std::forward<Args>(args)...);
}

template <typename T> void pin_handle(handle<T> h) noexcept {
  assert(h != nullptr);
  h->refCount.fetch_add(1, std::memory_order_relaxed);
}

template <typename T> void pin_void_handle(void *h) noexcept {
  pin_handle(static_cast<handle<T>>(h));
}

template <typename T> void unpin_handle(handle<T> h) noexcept {
  if (h == nullptr) {
    return;
  }
  if (h->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    delete h;
  }
}

template <typename T> void unpin_void_handle(void *h) noexcept {
  unpin_handle(static_cast<handle<T>>(h));
}

template <typename T, typename... Args> void *make_void_handle(Args &&...args) {
  return static_cast<void *>(make_handle<T>(std::forward<Args>(args)...));
}

template <typename T>
[[nodiscard]]
T *handle_ptr(handle<T> h) noexcept {
  return h != nullptr ? &h->value : nullptr;
}

template <typename T>
[[nodiscard]]
const T *handle_ptr(const handle_control_block<T> *h) noexcept {
  return h != nullptr ? &h->value : nullptr;
}

template <typename T>
[[nodiscard]]
T &handle_ref(handle<T> h) noexcept {
  assert(h != nullptr);
  return h->value;
}

template <typename T>
[[nodiscard]]
const T &handle_ref(const handle_control_block<T> *h) noexcept {
  assert(h != nullptr);
  return h->value;
}

template <typename T>
[[nodiscard]]
T *void_handle_ptr(void *h) noexcept {
  return handle_ptr(static_cast<handle<T>>(h));
}

template <typename T>
[[nodiscard]]
const T *void_handle_ptr(const void *h) noexcept {
  return handle_ptr(
      static_cast<const handle_control_block<T> *>(h));
}

template <typename T>
[[nodiscard]]
T &void_handle_ref(void *h) noexcept {
  return handle_ref(static_cast<handle<T>>(h));
}

template <typename T>
[[nodiscard]]
const T &void_handle_ref(const void *h) noexcept {
  return handle_ref(
      static_cast<const handle_control_block<T> *>(h));
}

} // namespace strobe::gpu
