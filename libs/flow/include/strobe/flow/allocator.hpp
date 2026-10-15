#pragma once

#include <strobe/core/memory/allocator_ref.hpp>
#include <strobe/core/memory/mallocator.hpp>

namespace strobe::flow {

/**
 * \brief Allocator used by Flow-owned data.
 * \code{.cpp}
 * using allocator = Mallocator;
 * \endcode
 */
using allocator = Mallocator;

/**
 * \brief Non-owning reference to the allocator used by Flow.
 * \code{.cpp}
 * using allocator_ref = AllocatorReference<allocator>;
 * \endcode
 */
using allocator_ref = AllocatorReference<allocator>;

} // namespace strobe::flow
