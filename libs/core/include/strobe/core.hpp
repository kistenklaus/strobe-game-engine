#pragma once

// precompile header
#include "strobe/core/containers.hpp"
#include "strobe/core/events/event_dispatcher.hpp"

/**
 * \defgroup core core-template-library
 * \brief Core foundation libraries
 *
 * \subsection core_containers Containers:
 * - \ref strobe::Bitset -- Fixed-size compile-time bitset.
 * - \ref strobe::BitVector -- Dynamically sized packed bit vector.
 * - \ref strobe::BoxedArray -- Heap-backed fixed-size array.
 * - \ref strobe::BoxedBitset -- Heap-backed fixed-size bitset.
 * - \ref strobe::BoxedStr -- Heap-backed fixed-capacity string.
 * - \ref strobe::HashMap -- Hash table with open addressing.
 * - \ref strobe::InplaceBitVector -- Fixed-capacity packed bit vector.
 * - \ref strobe::InplaceLinearMap -- Fixed-capacity linear map.
 * - \ref strobe::InplaceLinearSet -- Fixed-capacity linear set.
 * - \ref strobe::InplaceString -- Fixed-capacity string with inline storage.
 * - \ref strobe::InplaceVector -- Fixed-capacity vector with inline storage.
 * - \ref strobe::InplaceVectorDeque -- Fixed-capacity double-ended vector.
 * - \ref strobe::LinearMap -- Dynamically sized map with linear lookup.
 * - \ref strobe::LinearSet -- Dynamically sized set with linear lookup.
 * - \ref strobe::SmallBitVector -- Bit vector with small-object optimization.
 * - \ref strobe::SmallLinearMap -- Linear map with inline storage.
 * - \ref strobe::SmallLinearSet -- Linear set with inline storage.
 * - \ref strobe::SmallString -- String with small-object optimization.
 * - \ref strobe::SmallVector -- Vector with small-object optimization.
 * - \ref strobe::SmallVectorDeque -- Double-ended vector with inline storage.
 * - \ref strobe::span -- Non-owning view over contiguous objects.
 * - \ref strobe::Str -- Fixed-size null-terminated string literal wrapper.
 * - \ref strobe::String -- Dynamically sized null-terminated string.
 * - \ref strobe::SwizzHashMap -- SIMD-accelerated open-addressing hash map.
 * - \ref strobe::Vector -- Dynamically sized contiguous sequence.
 * - \ref strobe::VectorDeque -- Dynamically sized double-ended vector.

 * \subsection core_memory Memory:
 * - \ref strobe::Allocator -- Basic allocation and deallocation concept.
 * - \ref strobe::AllocatorTraits -- Common allocator operation adapters.
 * - \ref strobe::ComparableAllocator -- Equality-comparable allocator concept.
 * - \ref strobe::OverAllocator -- Allocator that reports available size.
 * - \ref strobe::ReAllocator -- Allocator that resizes allocations.
 * - \ref strobe::OwningAllocator -- Allocator that tests pointer ownership.
 * - \ref strobe::SizeIndependentAllocator -- Allocator with size-free deallocation.
 * - \ref strobe::StatelessAllocator -- Allocator with no per-instance state.
 * - \ref strobe::AllocatorReference -- Reference adapter for allocators.
 * - \ref strobe::NamedAllocator -- Compile-time named allocator adapter.
 * - \ref strobe::Mallocator -- General-purpose aligned allocator.
 * - \ref strobe::PageAllocator -- Page-aligned virtual memory allocator.
 * - \ref strobe::NullAllocator -- Allocator that never provides storage.
 * - \ref strobe::PolyMemoryResource -- Type-erased byte allocator interface.
 * - \ref strobe::MemoryResource -- Adapter from an allocator to the erased interface.
 * - \ref strobe::PolyResourceReference -- Non-owning reference to an erased resource.
 * - \ref strobe::StlAllocator -- Standard allocator adapter.
 * - \ref strobe::Box -- Nullable move-only ownership of one object.
 * - \ref strobe::Rc -- Single-threaded reference-counted ownership.
 * - \ref strobe::Arc -- Atomically reference-counted ownership.
 * - \ref strobe::MonotonicResource -- Single-threaded variable-size resource.
 * - \ref strobe::SmallMonotonicResource -- Monotonic resource with inline storage.
 * - \ref strobe::SyncMonotonicResource -- Concurrent monotonic allocation resource.
 * - \ref strobe::SyncResource -- Mutex-synchronized allocator wrapper.
 * - \ref strobe::MonotonicPoolResource -- Single-threaded fixed-size pool resource.
 * - \ref strobe::MPSCMonotonicPoolResource -- MPSC fixed-size pool resource.
 * - \ref strobe::MPMCMonotonicPoolResource -- MPMC fixed-size pool resource.
 * - \ref strobe::SyncMonotonicPoolResource -- Mutex-synchronized MPSC pool.
 * - \ref strobe::RankedMPSCResource -- Ranked collection of MPSC pools.
 * - \ref strobe::RankedMPMCResource -- Ranked collection of MPMC pools.
 * - \ref strobe::RankedSyncResource -- Ranked collection of synchronized pools.
 * - \ref strobe::BuddyResource -- Power-of-two allocator over a backing region.
 * - \ref strobe::memory::cache_line -- Cache-line size constant.
 * - \ref strobe::page_size -- Returns the system page size.
 *
 * \subsection core_events Events:
 * - \ref strobe::BasicEvent -- Simple event carrying a payload.
 * - \ref strobe::EventListenerRef -- Non-owning reference to an event callback.
 * - \ref strobe::EventListenerHandle -- Handle that controls listener lifetime.
 * - \ref strobe::EventDispatcher -- Dispatches events to registered listeners.
 *
 *
 */
