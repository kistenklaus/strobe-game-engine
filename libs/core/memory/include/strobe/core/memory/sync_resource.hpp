#pragma once

#include "strobe/core/memory/allocator_traits.hpp"
#include "strobe/core/memory/mallocator.hpp"

#include <concepts>
#include <cstddef>
#include <mutex>
#include <utility>

namespace strobe {

/**
 * \ingroup core
 * \brief Mutex-synchronized wrapper for an allocator.
 * \code{.cpp}
 * template<Allocator Upstream>
 * class SyncResource;
 * \endcode
 *
 * Every forwarded allocator operation acquires the resource mutex. The
 * unsafe_resource() accessors and locked() callbacks are the exceptions: they
 * expose controlled access to the underlying allocator and must follow the
 * caller's synchronization requirements.
 *
 * \tparam Upstream Allocator to wrap.
 */
template <Allocator Upstream> class SyncResource {
private:
  using upstream_type = Upstream;
  using upstream_traits = AllocatorTraits<upstream_type>;

public:
  /**
   * \brief Constructs a resource with a default-constructed upstream.
   * \code{.cpp}
   * SyncResource()
   *   requires std::default_initializable<Upstream>;
   * \endcode
   */
  SyncResource()
    requires std::default_initializable<upstream_type>
  = default;

  /**
   * \brief Constructs a resource from an upstream allocator.
   * \code{.cpp}
   * explicit SyncResource(Upstream upstream);
   * \endcode
   *
   * \param upstream Allocator to wrap.
   */
  explicit SyncResource(upstream_type upstream)
      : m_resource(std::move(upstream)) {}

  /**
   * \brief Constructs the upstream allocator in place.
   * \code{.cpp}
   * template<typename... Args>
   * explicit SyncResource(std::in_place_t, Args&&... args);
   * \endcode
   *
   * \param args Arguments forwarded to the upstream constructor.
   */
  template <typename... Args>
    requires std::constructible_from<upstream_type, Args &&...>
  explicit SyncResource(std::in_place_t, Args &&...args)
      : m_resource(std::forward<Args>(args)...) {}

  /**
   * \brief Constructs and assigns resource state.
   * \code{.cpp}
   * SyncResource(const SyncResource&) = delete;
   * SyncResource& operator=(const SyncResource&) = delete;
   * SyncResource(SyncResource&&) = delete;
   * SyncResource& operator=(SyncResource&&) = delete;
   * \endcode
   */
  SyncResource(const SyncResource &) = delete;
  SyncResource &operator=(const SyncResource &) = delete;
  SyncResource(SyncResource &&) = delete;
  SyncResource &operator=(SyncResource &&) = delete;

  /**
   * \brief Allocates storage through the upstream allocator.
   * \code{.cpp}
   * [[nodiscard]] void* allocate(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Number of bytes requested.
   * \param align Required alignment.
   * \return Pointer to allocated storage.
   */
  [[nodiscard]] void *allocate(std::size_t size, std::size_t align) {
    std::lock_guard lock{m_mutex};
    return upstream_traits::allocate(m_resource, size, align);
  }

  /**
   * \brief Releases storage through the upstream allocator.
   * \code{.cpp}
   * void deallocate(void* pointer, std::size_t size,
   *                 std::size_t align);
   * \endcode
   *
   * \param pointer Storage returned by allocate().
   * \param size Original allocation size.
   * \param align Original allocation alignment.
   */
  void deallocate(void *pointer, std::size_t size, std::size_t align) {
    std::lock_guard lock{m_mutex};
    upstream_traits::deallocate(m_resource, pointer, size, align);
  }

  /**
   * \brief Resizes storage through the upstream allocator.
   * \code{.cpp}
   * [[nodiscard]] void* reallocate(void* pointer, std::size_t old_size,
   *                                std::size_t new_size, std::size_t align)
   *   requires ReAllocator<Upstream>;
   * \endcode
   *
   * \param pointer Storage to resize.
   * \param old_size Size of the existing allocation.
   * \param new_size Requested new size.
   * \param align Allocation alignment.
   * \return Pointer to resized storage.
   */
  [[nodiscard]] void *reallocate(void *pointer, std::size_t old_size,
                                 std::size_t new_size, std::size_t align)
    requires ReAllocator<upstream_type>
  {
    std::lock_guard lock{m_mutex};
    return m_resource.reallocate(pointer, old_size, new_size, align);
  }

  /**
   * \brief Allocates storage and reports the available size.
   * \code{.cpp}
   * [[nodiscard]] std::pair<void*, std::size_t>
   * allocate_at_least(std::size_t size, std::size_t align)
   *   requires OverAllocator<Upstream>;
   * \endcode
   *
   * \param size Minimum number of bytes requested.
   * \param align Required alignment.
   * \return Pointer and number of bytes available.
   */
  [[nodiscard]] std::pair<void *, std::size_t>
  allocate_at_least(std::size_t size, std::size_t align)
    requires OverAllocator<upstream_type>
  {
    std::lock_guard lock{m_mutex};
    return m_resource.allocate_at_least(size, align);
  }

  /**
   * \brief Releases storage without size metadata.
   * \code{.cpp}
   * void deallocate(void* pointer)
   *   requires SizeIndependentAllocator<Upstream>;
   * \endcode
   *
   * \param pointer Storage returned by the upstream allocator.
   */
  void deallocate(void *pointer)
    requires SizeIndependentAllocator<upstream_type>
  {
    std::lock_guard lock{m_mutex};
    m_resource.deallocate(pointer);
  }

  /**
   * \brief Tests whether the upstream allocator owns a pointer.
   * \code{.cpp}
   * bool owns(const void* pointer) const
   *   requires OwningAllocator<Upstream>;
   * \endcode
   *
   * \param pointer Pointer to test.
   * \return Whether the pointer belongs to the upstream allocator.
   */
  bool owns(const void *pointer) const
    requires OwningAllocator<upstream_type>
  {
    std::lock_guard lock{m_mutex};
    return m_resource.owns(pointer);
  }

  /**
   * \brief Runs a callback while holding the resource mutex.
   * \code{.cpp}
   * template<typename Fn>
   * decltype(auto) locked(Fn&& function);
   * template<typename Fn>
   * decltype(auto) locked(Fn&& function) const;
   * \endcode
   *
   * \param function Callback receiving the underlying allocator.
   * \return The callback's result.
   * \attention 1. The callback must not attempt to lock this resource again.
   */
  template <typename Fn> decltype(auto) locked(Fn &&function) {
    std::lock_guard lock{m_mutex};
    return std::forward<Fn>(function)(m_resource);
  }

  template <typename Fn> decltype(auto) locked(Fn &&function) const {
    std::lock_guard lock{m_mutex};
    return std::forward<Fn>(function)(m_resource);
  }

  /**
   * \brief Accesses the upstream allocator without locking.
   * \code{.cpp}
   * Upstream& unsafe_resource() noexcept;
   * const Upstream& unsafe_resource() const noexcept;
   * \endcode
   *
   * \return Reference to the wrapped allocator.
   * \attention 1. The caller is responsible for synchronization.
   */
  upstream_type &unsafe_resource() noexcept { return m_resource; }
  const upstream_type &unsafe_resource() const noexcept { return m_resource; }

private:
  upstream_type m_resource;
  mutable std::mutex m_mutex;
};

static_assert(Allocator<SyncResource<Mallocator>>);

} // namespace strobe
