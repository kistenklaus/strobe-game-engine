#pragma once

#include <cstddef>

#include "strobe/core/memory/allocator_ref.hpp"
#include "strobe/core/memory/allocator_traits.hpp"

namespace strobe {

/**
 * \ingroup core
 * \brief Type-erased interface for a byte allocator.
 * \code{.cpp}
 * class PolyMemoryResource;
 * \endcode
 *
 * Concrete allocators can be exposed through this interface when callers do
 * not need to know the allocator's concrete type.
 */
class PolyMemoryResource {
 public:
  /**
   * \brief Destroys the polymorphic resource.
   * \code{.cpp}
   * virtual ~PolyMemoryResource();
   * \endcode
   */
  virtual ~PolyMemoryResource() = default;

  /**
   * \brief Allocates raw storage through the resource.
   * \code{.cpp}
   * virtual void* allocate(std::size_t size, std::size_t alignment) = 0;
   * \endcode
   *
   * \param size Number of bytes requested.
   * \param alignment Required alignment.
   * \return Allocated storage, or \c nullptr when the resource cannot satisfy
   * the request.
   */
  virtual void* allocate(std::size_t size, std::size_t alignment) = 0;

  /**
   * \brief Releases storage previously returned by allocate.
   * \code{.cpp}
   * virtual void deallocate(void* pointer, std::size_t size,
   *                         std::size_t alignment) = 0;
   * \endcode
   *
   * \param pointer Storage to release.
   * \param size Number of bytes originally requested.
   * \param alignment Alignment supplied for the allocation.
   * \attention 1. The arguments must describe a live allocation from this
   * resource.
   */
  virtual void deallocate(void* pointer, std::size_t size,
                          std::size_t alignment) = 0;
};

/**
 * \ingroup core
 * \brief Adapts a concrete allocator to PolyMemoryResource.
 * \code{.cpp}
 * template<Allocator A>
 * class MemoryResource;
 * \endcode
 *
 * \tparam A Concrete allocator type to erase.
 */
template <Allocator A>
class MemoryResource : public PolyMemoryResource {
  using ATraits = AllocatorTraits<A>;

 public:
  /**
   * \brief Constructs a resource by copying an allocator.
   * \code{.cpp}
   * explicit MemoryResource(const A& allocator);
   * \endcode
   *
   * \param allocator Allocator instance to retain.
   */
  MemoryResource(const A& alloc) : m_allocator(alloc) {}

  /**
   * \brief Allocates raw storage through the wrapped allocator.
   * \code{.cpp}
   * void* allocate(std::size_t size, std::size_t alignment) override;
   * \endcode
   *
   * \param size Number of bytes requested.
   * \param alignment Required alignment.
   * \return Allocated storage, or \c nullptr when the wrapped allocator
   * cannot satisfy the request.
   */
  void* allocate(std::size_t size, std::size_t alignment) final override {
    return ATraits::allocate(m_allocator, size, alignment);
  }

  /**
   * \brief Releases storage through the wrapped allocator.
   * \code{.cpp}
   * void deallocate(void* pointer, std::size_t size,
   *                 std::size_t alignment) override;
   * \endcode
   *
   * \param pointer Storage to release.
   * \param size Number of bytes originally requested.
   * \param alignment Alignment supplied for the allocation.
   * \attention 1. The arguments must describe a live allocation from this
   * resource.
   */
  void deallocate(void* pointer, std::size_t size,
                  std::size_t alignment) final override {
    ATraits::deallocate(m_allocator, pointer, size, alignment);
  }

  /**
   * \brief Accesses the wrapped allocator.
   * \code{.cpp}
   * const A& operator*() const;
   * A& operator*();
   * \endcode
   *
   * \return Reference to the wrapped allocator.
   */
  const A& operator*() const { return m_allocator; }
  A& operator*() { return m_allocator; }

 private:
  [[no_unique_address]] A m_allocator;
};

/**
 * \ingroup core
 * \brief Non-owning reference to a polymorphic memory resource.
 * \code{.cpp}
 * using PolyResourceReference = AllocatorReference<PolyMemoryResource>;
 * \endcode
 *
 * \attention 1. The referenced resource must outlive the reference.
 */
using PolyResourceReference = AllocatorReference<PolyMemoryResource>;

}  // namespace strobe
