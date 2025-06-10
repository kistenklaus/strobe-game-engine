#include "./PageAllocator.hpp"
#include <algorithm>

#if defined(_WIN32)
#include <windows.h>
#else
#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <exception>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cassert>

#include "strobe/core/memory/align.hpp"
#include "strobe/core/memory/pages.hpp"

namespace strobe {

#ifdef NDEBUG
static constexpr bool USE_GUARD_PAGES = false;
#else
static constexpr bool USE_GUARD_PAGES = false;
#endif

void* PageAllocator::allocate(std::size_t size, std::size_t alignment) {
  if (size == 0) {
    return nullptr;
  }
  const std::size_t page = strobe::page_size();
  size = memory::align_up(size, page);
  assert(page % alignment == 0);

  std::size_t totalSize = size;
  if (USE_GUARD_PAGES) {
    totalSize += 2 * page;
  }

#if defined(_WIN32)
  void* raw = VirtualAlloc(nullptr, totalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!raw) {
    return nullptr;
  }

  if (USE_GUARD_PAGES) {
    DWORD oldProtect;
    // First page
    VirtualProtect(raw, page, PAGE_NOACCESS, &oldProtect);
    // Last page
    VirtualProtect(static_cast<char*>(raw) + page + size, page, PAGE_NOACCESS, &oldProtect);
    return static_cast<char*>(raw) + page;
  }

  return raw;
#else
  void* raw = mmap(nullptr, totalSize, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (raw == MAP_FAILED)
    return nullptr;

  if (USE_GUARD_PAGES) {
    mprotect(raw, page, PROT_NONE);
    mprotect(static_cast<char*>(raw) + page + size, page, PROT_NONE);
    return static_cast<char*>(raw) + page;
  }

  return raw;
#endif
}

void PageAllocator::deallocate(void* ptr, std::size_t size, std::size_t alignment) {
  if (!ptr)
    return;

  const std::size_t page = strobe::page_size();
  alignment = ::std::max<std::size_t>(alignment, page);
  size = strobe::memory::align_up(size, page);

  void* raw = ptr;
  std::size_t totalSize = size;

  if (USE_GUARD_PAGES) {
    raw = static_cast<char*>(ptr) - page;
    totalSize += 2 * page;
  }

#if defined(_WIN32)
  BOOL result = VirtualFree(raw, 0, MEM_RELEASE);
  if (!result) {
    std::terminate();
  }
#else
  int result = munmap(raw, totalSize);
  if (result != 0) {
    std::terminate();
  }
#endif
}

}  // namespace strobe
