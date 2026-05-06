#pragma once

#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/Mallocator.hpp"

#include <concepts>
#include <mutex>
#include <type_traits>
#include <utility>

namespace strobe {

template <Allocator Upstream> class SyncResource {
private:
  using upstream_type = Upstream;
  using upstream_traits = AllocatorTraits<upstream_type>;

public:
  SyncResource()
    requires std::default_initializable<upstream_type>
  = default;

  explicit SyncResource(upstream_type upstream)
      : m_resource(std::move(upstream)) {}

  template <typename... Args>
  explicit SyncResource(std::in_place_t, Args &&...args)
      : m_resource(std::forward<Args>(args)...) {}

  template <typename... Args>
    requires std::constructible_from<upstream_type, Args &&...>
  explicit SyncResource(std::in_place_t, Args &&...args)
      : m_resource(std::forward<Args>(args)...) {}

  SyncResource(const SyncResource &) = delete;
  SyncResource &operator=(const SyncResource &) = delete;

  SyncResource(SyncResource &&) = delete;
  SyncResource &operator=(SyncResource &&) = delete;

  void *allocate(std::size_t size, std::size_t align) {
    std::lock_guard lock(m_mutex);
    return upstream_traits::allocate(m_resource, size, align);
  }

  void deallocate(void *ptr, std::size_t size, std::size_t align) {
    std::lock_guard lock(m_mutex);
    upstream_traits::deallocate(m_resource, ptr, size, align);
  }

  void *reallocate(void *ptr, std::size_t old_size, std::size_t new_size,
                   std::size_t align)
    requires ReAllocator<upstream_type>
  {
    std::lock_guard lock(m_mutex);
    return m_resource.reallocate(ptr, old_size, new_size, align);
  }

  std::pair<void *, std::size_t> allocate_at_least(std::size_t size,
                                                   std::size_t align)
    requires OverAllocator<upstream_type>
  {
    std::lock_guard lock(m_mutex);
    return m_resource.allocate_at_least(size, align);
  }

  void deallocate(void *ptr)
    requires SizeIndepdententAllocator<upstream_type>
  {
    std::lock_guard lock(m_mutex);
    m_resource.deallocate(ptr);
  }

  bool owns(const void *ptr) const
    requires OwningAllocator<upstream_type>
  {
    std::lock_guard lock(m_mutex);
    return m_resource.owns(ptr);
  }

  template <typename Fn> decltype(auto) locked(Fn &&fn) {
    std::lock_guard lock(m_mutex);
    return std::forward<Fn>(fn)(m_resource);
  }

  template <typename Fn> decltype(auto) locked(Fn &&fn) const {
    std::lock_guard lock(m_mutex);
    return std::forward<Fn>(fn)(m_resource);
  }

  upstream_type &unsafe_resource() noexcept { return m_resource; }

  const upstream_type &unsafe_resource() const noexcept { return m_resource; }

private:
  upstream_type m_resource;
  mutable std::mutex m_mutex;
};

static_assert(Allocator<SyncResource<Mallocator>>);

} // namespace strobe
