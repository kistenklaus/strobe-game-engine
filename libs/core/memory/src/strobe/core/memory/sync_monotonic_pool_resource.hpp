#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/mpsc_monotonic_pool_resource.hpp"
#include <cstddef>
#include <mutex>
#include <ratio>

namespace strobe {

template <std::size_t BlockSize, std::size_t BlockAlign, Allocator A,
          typename GrowthFactor = std::ratio<2, 1>>
class SyncMonotonicPoolResource {
public:
  using Self =
      SyncMonotonicPoolResource<BlockSize, BlockAlign, A, GrowthFactor>;

  using upstream_allocator = A;
  using upstream_traits = AllocatorTraits<upstream_allocator>;

  using mpsc_pool =
      MPSCMonotonicPoolResource<BlockSize, BlockAlign, upstream_allocator, GrowthFactor>;

  explicit SyncMonotonicPoolResource(const A &upstream) : m_mpsc(upstream) {}

  void *allocate(std::size_t size, std::size_t align) {
    std::lock_guard lck{m_mutex};
    return m_mpsc.allocate(size, align);
  }

  void *allocate() {
    std::lock_guard lck{m_mutex};
    return m_mpsc.allocate();
  }

  void deallocate(void *ptr, std::size_t size, std::size_t align) noexcept {
    return m_mpsc.deallocate(ptr, size, align);
  }

  void deallocate(void *ptr) noexcept { return m_mpsc.deallocate(ptr); }

  SyncMonotonicPoolResource select_on_container_copy_construction() const {
    return SyncMonotonicPoolResource(m_mpsc);
  }

  bool operator==(const SyncMonotonicPoolResource &o) const noexcept {
    return this == &o;
  }

  bool operator!=(const SyncMonotonicPoolResource &o) const noexcept = default;

private:
  std::mutex m_mutex;
  mpsc_pool m_mpsc;
};

} // namespace strobe::rhi
