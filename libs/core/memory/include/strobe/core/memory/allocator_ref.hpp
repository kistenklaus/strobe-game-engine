#pragma once

#include <cassert>
#include <type_traits>
#include <variant>

#include "strobe/core/memory/allocator_traits.hpp"

namespace strobe {

/**
 * \ingroup core
 * \brief Allocator adapter that refers to stateful or stateless allocators.
 * \code{.cpp}
 * template<Allocator Resource>
 * class AllocatorReference;
 * \endcode
 *
 * Stateful resources are referenced through a pointer. Stateless resources are
 * represented without storing a pointer and are instantiated when an
 * operation is performed.
 *
 * \attention 1. A stateful resource must outlive this reference and every
 * operation performed through it.
 */
template <Allocator Resource> class AllocatorReference {
public:
  /**
   * \brief Constructs an allocator reference.
   * \code{.cpp}
   * explicit AllocatorReference(Resource* resource)
   *   requires(!StatelessAllocator<Resource>);
   * AllocatorReference() noexcept
   *   requires StatelessAllocator<Resource>;
   * AllocatorReference(Resource* resource) noexcept
   *   requires StatelessAllocator<Resource>;
   * \endcode
   *
   * The default constructor and the pointer constructor without storage are
   * available for StatelessAllocator resources. The pointer is ignored for a
   * stateless resource.
   *
   * \param resource Resource to reference for a stateful allocator.
   * \attention 1. For a stateful resource, \p resource must not be null.
   */
  AllocatorReference(Resource *resource)
    requires(!StatelessAllocator<Resource>)
      : m_resource(resource) {
    assert(resource != nullptr);
  }

  AllocatorReference() noexcept
    requires StatelessAllocator<Resource>
      : m_resource{} {}

  AllocatorReference([[maybe_unused]] Resource *) noexcept
    requires StatelessAllocator<Resource>
      : m_resource{} {}

  /**
   * \brief Allocates raw storage through the referenced resource.
   * \code{.cpp}
   * void* allocate(std::size_t size, std::size_t align);
   * \endcode
   *
   * \param size Number of bytes to allocate.
   * \param align Required alignment.
   * \return Pointer to allocated storage, or the resource's failure result.
   */
  void *allocate(std::size_t size, std::size_t align) {
    if constexpr (StatelessAllocator<Resource>) {
      Resource resource{};
      return Traits::allocate(resource, size, align);
    } else {
      return Traits::allocate(*m_resource, size, align);
    }
  }

  /**
   * \brief Releases storage through the referenced resource.
   * \code{.cpp}
   * void deallocate(void* ptr, std::size_t size, std::size_t align);
   * \endcode
   *
   * \param ptr Storage returned by allocate.
   * \param size Number of bytes originally allocated.
   * \param align Alignment used for the allocation.
   * \attention 1. The pointer, size, and alignment must describe a prior
   * allocation from this reference.
   */
  void deallocate(void *ptr, std::size_t size, std::size_t align) {
    if constexpr (StatelessAllocator<Resource>) {
      Resource resource{};
      return Traits::deallocate(resource, ptr, size, align);
    } else {
      return Traits::deallocate(*m_resource, ptr, size, align);
    }
  }

  /**
   * \brief Releases storage without requiring its size.
   * \code{.cpp}
   * void deallocate(void* ptr);
   * \endcode
   *
   * \param ptr Storage to release.
   * \attention 1. Resource must satisfy SizeIndependentAllocator.
   */
  void deallocate(void *ptr)
    requires(SizeIndependentAllocator<Resource>)
  {
    if constexpr (StatelessAllocator<Resource>) {
      Resource resource{};
      return Traits::size_independent_deallocate(resource, ptr);
    } else {
      return Traits::size_independent_deallocate(*m_resource, ptr);
    }
  }

  /**
   * \brief Reports whether the resource owns a pointer.
   * \code{.cpp}
   * bool owns(void* ptr);
   * \endcode
   *
   * \param ptr Pointer to test.
   * \return Whether the referenced resource owns \p ptr.
   * \attention 1. Resource must satisfy OwningAllocator.
   */
  bool owns(void *ptr)
    requires OwningAllocator<Resource>
  {
    if constexpr (StatelessAllocator<Resource>) {
      Resource resource{};
      return Traits::owns(resource, ptr);
    } else {
      return Traits::owns(*m_resource, ptr);
    }
  }

private:
  using Traits = AllocatorTraits<Resource>;
  using storage_type =
      std::conditional_t<StatelessAllocator<Resource>, std::monostate,
                         Resource *>;

  storage_type m_resource;
};

} // namespace strobe
