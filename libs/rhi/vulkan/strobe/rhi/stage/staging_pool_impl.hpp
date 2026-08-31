#pragma once

#include "strobe/core/type_traits/unroll.hpp"
#include "strobe/rhi/buf/buf.hpp"
#include "strobe/rhi/memory/memory_pool.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/stage/staging_page.hpp"
#include <atomic>
#include <memory>
#include <mutex>

namespace strobe::rhi {

struct StagingPoolImpl {
public:
  static constexpr std::array<VkDeviceSize, 5> STAGING_CLASSES{
      1 << 14, // 16KiB
      1 << 16, // 64KiB
      1 << 18, // 256KiB
      1 << 20, // 1MiB
      1 << 22, // 4MiB
               // larger (is not held persistantly)
  };
  static constexpr BufferUsage BUFFER_USAGE =
      BufferUsage::transfer_src | BufferUsage::shader_device_address;
  static constexpr MemoryUsage MEMORY_USAGE =
      MemoryUsage::mapped_write_sequential;

  explicit StagingPoolImpl(MemoryPool memoryPool,
                           buf::handle_allocators *bufAllocators,
                           strobe::rhi::allocator_ref alloc) noexcept 
      : memoryPool(std::move(memoryPool)), m_bufAllocators(bufAllocators),
        m_pageAlloc(alloc) {
    for (uint32_t i = 0; i < STAGING_CLASSES.size(); ++i) {
      m_readyPages[i] = nullptr;
    }
  }
  StagingPoolImpl(const StagingPoolImpl &) = delete;
  StagingPoolImpl(StagingPoolImpl &&) = delete;
  StagingPoolImpl &operator=(const StagingPoolImpl &) = delete;
  StagingPoolImpl &operator=(StagingPoolImpl &&) = delete;
  ~StagingPoolImpl() noexcept {
    destroy_pages(m_returnedPages.load(std::memory_order_relaxed));
    for (StagingPage *pages : m_readyPages) {
      destroy_pages(pages);
    }
  }

  // multiple producers (internally synchronized)
  StagingPage *alloc_page(VkDeviceSize minSize) {
    ZoneScopedN("stage/alloc-page");
    size_t classIndex = STAGING_CLASSES.size();
    STROBE_UNROLL(STAGING_CLASSES.size())
    for (size_t i = 0; i < STAGING_CLASSES.size(); ++i) {
      if (minSize <= STAGING_CLASSES[i]) {
        classIndex = i;
        break;
      }
    }

    std::lock_guard lck{m_mutex};

    // Oversized allocations are never cached.
    if (classIndex == STAGING_CLASSES.size()) {
      return create_page(minSize);
    }

    if (StagingPage *page = m_readyPages[classIndex]) {
      m_readyPages[classIndex] = page->next;
      page->next = nullptr;
      return page;
    }

    StagingPage *returned =
        m_returnedPages.exchange(nullptr, std::memory_order_acquire);
    while (returned != nullptr) {
      StagingPage *next = returned->next;
      returned->next = nullptr;
      size_t returnedClass = STAGING_CLASSES.size();
      STROBE_UNROLL(STAGING_CLASSES.size())
      for (size_t i = 0; i < STAGING_CLASSES.size(); ++i) {
        if (returned->buffer.size() <= STAGING_CLASSES[i]) {
          returnedClass = i;
          break;
        }
      }
      if (returnedClass == STAGING_CLASSES.size()) {
        // Oversized staging buffers are not retained.
        destroy_pages(returned);
      } else {
        returned->next = m_readyPages[returnedClass];
        m_readyPages[returnedClass] = returned;
      }
      returned = next;
    }
    if (StagingPage *page = m_readyPages[classIndex]) {
      m_readyPages[classIndex] = page->next;
      page->next = nullptr;
      return page;
    }
    return create_page(STAGING_CLASSES[classIndex]);
  }

  // multiple producers (lockfree)
  void recycle_page(StagingPage *begin, StagingPage *end) noexcept {
    if (begin == nullptr) {
      assert(end == nullptr);
      return;
    }
    assert(end != nullptr);
    StagingPage *head = m_returnedPages.load(std::memory_order_relaxed);
    do {
      end->next = head;
    } while (!m_returnedPages.compare_exchange_weak(
        head, begin, std::memory_order_release, std::memory_order_relaxed));
  }

private:
  // single consumer (externally synchronized)
  StagingPage *create_page(VkDeviceSize size) {
    auto *page = static_cast<StagingPage *>(m_pageAlloc.allocate());
    std::construct_at(page);
    page->buffer = buf::create_buffer(memoryPool,
                                      {
                                          .size = size,
                                          .bufferUsage = BUFFER_USAGE,
                                          .memoryUsage = MEMORY_USAGE,
                                      },
                                      {}, m_bufAllocators);
    return page;
  }
  // multiple producer
  void destroy_pages(StagingPage *page) {
    while (page != nullptr) {
      StagingPage *next = page->next;
      std::destroy_at(page);
      m_pageAlloc.deallocate(page);
      page = next;
    }
  }

private:
  MemoryPool memoryPool;
  buf::handle_allocators *m_bufAllocators;

  MPSCMonotonicPoolResource<sizeof(StagingPage), alignof(StagingPage),
                            strobe::rhi::allocator_ref>
      m_pageAlloc;
  std::mutex m_mutex{};

  std::array<StagingPage *, STAGING_CLASSES.size()> m_readyPages;
  std::atomic<StagingPage *> m_returnedPages = nullptr;
};

} // namespace strobe::rhi
