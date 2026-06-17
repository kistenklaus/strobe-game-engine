#include <tracy/Tracy.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <new>

#include <fmt/printf.h>

namespace {

[[nodiscard]]
void *traced_allocate(std::size_t size) {
  size = std::max<std::size_t>(size, 1);

  while (true) {
    if (void *ptr = std::malloc(size)) {
      TracySecureAlloc(ptr, size);

      return ptr;
    }

    const std::new_handler handler = std::get_new_handler();

    if (handler == nullptr) {
      throw std::bad_alloc{};
    }

    handler();
  }
}

[[nodiscard]]
void *traced_allocate_aligned(std::size_t size, std::size_t alignment) {
  size = std::max<std::size_t>(size, 1);

  alignment = std::max(alignment, sizeof(void *));

  while (true) {
    void *ptr = nullptr;

    if (posix_memalign(&ptr, alignment, size) == 0) {
      TracySecureAlloc(ptr, size);

      return ptr;
    }

    const std::new_handler handler = std::get_new_handler();

    if (handler == nullptr) {
      throw std::bad_alloc{};
    }

    handler();
  }
}

void traced_deallocate(void *ptr) noexcept {
  if (ptr == nullptr) {
    return;
  }

  TracySecureFree(ptr);

  std::free(ptr);
}

} // namespace

void *operator new(std::size_t size) { return traced_allocate(size); }

void *operator new[](std::size_t size) { return traced_allocate(size); }

void operator delete(void *ptr) noexcept { traced_deallocate(ptr); }

void operator delete[](void *ptr) noexcept { traced_deallocate(ptr); }

void operator delete(void *ptr, std::size_t) noexcept {
  traced_deallocate(ptr);
}

void operator delete[](void *ptr, std::size_t) noexcept {
  traced_deallocate(ptr);
}

void *operator new(std::size_t size, std::align_val_t alignment) {
  return traced_allocate_aligned(size, static_cast<std::size_t>(alignment));
}

void *operator new[](std::size_t size, std::align_val_t alignment) {
  return traced_allocate_aligned(size, static_cast<std::size_t>(alignment));
}

void operator delete(void *ptr, std::align_val_t) noexcept {
  traced_deallocate(ptr);
}

void operator delete[](void *ptr, std::align_val_t) noexcept {
  traced_deallocate(ptr);
}

void operator delete(void *ptr, std::size_t, std::align_val_t) noexcept {
  traced_deallocate(ptr);
}

void operator delete[](void *ptr, std::size_t, std::align_val_t) noexcept {
  traced_deallocate(ptr);
}
