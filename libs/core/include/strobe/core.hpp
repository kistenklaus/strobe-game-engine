#pragma once

// precompile header
#include "strobe/core/containers.hpp"
#include "strobe/core/events/event_dispatcher.hpp"

/**
 * \defgroup core core-template-library
 * \brief Core foundation libraries
 *
 * \subsection core_containers Containers:
 * - \ref strobe::Bitset -- \copybrief strobe::Bitset
 * - \ref strobe::BitVector -- \copybrief strobe::BitVector
 * - \ref strobe::BoxedArray -- \copybrief strobe::BoxedArray
 * - \ref strobe::BoxedBitset -- \copybrief strobe::BoxedBitset
 * - \ref strobe::BoxedStr -- \copybrief strobe::BoxedStr
 * - \ref strobe::HashMap -- \copybrief strobe::HashMap
 * - \ref strobe::InplaceBitVector -- \copybrief strobe::InplaceBitVector
 * - \ref strobe::InplaceLinearMap -- \copybrief strobe::InplaceLinearMap
 * - \ref strobe::InplaceLinearSet -- \copybrief strobe::InplaceLinearSet
 * - \ref strobe::InplaceString -- \copybrief strobe::InplaceString
 * - \ref strobe::InplaceVector -- \copybrief strobe::InplaceVector
 * - \ref strobe::InplaceVectorDeque -- \copybrief strobe::InplaceVectorDeque
 * - \ref strobe::LinearMap -- \copybrief strobe::LinearMap
 * - \ref strobe::LinearSet -- \copybrief strobe::LinearSet
 * - \ref strobe::SmallBitVector -- \copybrief strobe::SmallBitVector
 * - \ref strobe::SmallLinearMap -- \copybrief strobe::SmallLinearMap
 * - \ref strobe::SmallLinearSet -- \copybrief strobe::SmallLinearSet
 * - \ref strobe::SmallString -- \copybrief strobe::SmallString
 * - \ref strobe::SmallVector -- \copybrief strobe::SmallVector
 * - \ref strobe::SmallVectorDeque -- \copybrief strobe::SmallVectorDeque
 * - \ref strobe::span -- \copybrief strobe::span
 * - \ref strobe::Str -- \copybrief strobe::Str
 * - \ref strobe::String -- \copybrief strobe::String
 * - \ref strobe::SwizzHashMap -- \copybrief strobe::SwizzHashMap
 * - \ref strobe::Vector -- \copybrief strobe::Vector
 * - \ref strobe::VectorDeque -- \copybrief strobe::VectorDeque

 * \subsection core_fs Filesystem:
 * - \ref strobe::PathView -- \copybrief strobe::PathView
 * - \ref strobe::Path -- \copybrief strobe::Path
 * - \ref strobe::fs::File -- \copybrief strobe::fs::File
 * - \ref strobe::fs::Directory -- \copybrief strobe::fs::Directory
 * - \ref strobe::fs::Directory::EntryView -- \copybrief strobe::fs::Directory::EntryView
 * - \ref strobe::fs::Directory::iterator -- \copybrief strobe::fs::Directory::iterator
 * - \ref strobe::fs::Stat -- \copybrief strobe::fs::Stat
 * - \ref strobe::fs::Type -- \copybrief strobe::fs::Type
 * - \ref strobe::fs::FileAccess -- \copybrief strobe::fs::FileAccess
 * - \ref strobe::fs::FileSeekFlags -- \copybrief strobe::fs::FileSeekFlags
 * - \ref strobe::fs::CpFlags -- \copybrief strobe::fs::CpFlags
 * - \ref strobe::fs::MkdirFlags -- \copybrief strobe::fs::MkdirFlags
 * - \ref strobe::fs::MvFlags -- \copybrief strobe::fs::MvFlags
 * - \ref strobe::fs::RmFlags -- \copybrief strobe::fs::RmFlags
 * - \ref strobe::fs::StatFlags -- \copybrief strobe::fs::StatFlags
 * - \ref strobe::open -- \copybrief strobe::open
 * - \ref strobe::fs::open -- \copybrief strobe::fs::open
 * - \ref strobe::fs::ls -- \copybrief strobe::fs::ls
 * - \ref strobe::fs::exists -- \copybrief strobe::fs::exists
 * - \ref strobe::fs::stat -- \copybrief strobe::fs::stat
 * - \ref strobe::fs::mkdir -- \copybrief strobe::fs::mkdir
 * - \ref strobe::fs::cp -- \copybrief strobe::fs::cp
 * - \ref strobe::fs::mv -- \copybrief strobe::fs::mv
 * - \ref strobe::fs::rm -- \copybrief strobe::fs::rm

 * \subsection core_memory Memory:
 * - \ref strobe::Allocator -- \copybrief strobe::Allocator
 * - \ref strobe::AllocatorTraits -- \copybrief strobe::AllocatorTraits
 * - \ref strobe::ComparableAllocator -- \copybrief strobe::ComparableAllocator
 * - \ref strobe::OverAllocator -- \copybrief strobe::OverAllocator
 * - \ref strobe::ReAllocator -- \copybrief strobe::ReAllocator
 * - \ref strobe::OwningAllocator -- \copybrief strobe::OwningAllocator
 * - \ref strobe::SizeIndependentAllocator -- \copybrief strobe::SizeIndependentAllocator
 * - \ref strobe::StatelessAllocator -- \copybrief strobe::StatelessAllocator
 * - \ref strobe::AllocatorReference -- \copybrief strobe::AllocatorReference
 * - \ref strobe::NamedAllocator -- \copybrief strobe::NamedAllocator
 * - \ref strobe::Mallocator -- \copybrief strobe::Mallocator
 * - \ref strobe::PageAllocator -- \copybrief strobe::PageAllocator
 * - \ref strobe::NullAllocator -- \copybrief strobe::NullAllocator
 * - \ref strobe::PolyMemoryResource -- \copybrief strobe::PolyMemoryResource
 * - \ref strobe::MemoryResource -- \copybrief strobe::MemoryResource
 * - \ref strobe::PolyResourceReference -- \copybrief strobe::PolyResourceReference
 * - \ref strobe::StlAllocator -- \copybrief strobe::StlAllocator
 * - \ref strobe::Box -- \copybrief strobe::Box
 * - \ref strobe::Rc -- \copybrief strobe::Rc
 * - \ref strobe::Arc -- \copybrief strobe::Arc
 * - \ref strobe::MonotonicResource -- \copybrief strobe::MonotonicResource
 * - \ref strobe::SmallMonotonicResource -- \copybrief strobe::SmallMonotonicResource
 * - \ref strobe::SyncMonotonicResource -- \copybrief strobe::SyncMonotonicResource
 * - \ref strobe::SyncResource -- \copybrief strobe::SyncResource
 * - \ref strobe::MonotonicPoolResource -- \copybrief strobe::MonotonicPoolResource
 * - \ref strobe::MPSCMonotonicPoolResource -- \copybrief strobe::MPSCMonotonicPoolResource
 * - \ref strobe::MPMCMonotonicPoolResource -- \copybrief strobe::MPMCMonotonicPoolResource
 * - \ref strobe::SyncMonotonicPoolResource -- \copybrief strobe::SyncMonotonicPoolResource
 * - \ref strobe::RankedMPSCResource -- \copybrief strobe::RankedMPSCResource
 * - \ref strobe::RankedMPMCResource -- \copybrief strobe::RankedMPMCResource
 * - \ref strobe::RankedSyncResource -- \copybrief strobe::RankedSyncResource
 * - \ref strobe::BuddyResource -- \copybrief strobe::BuddyResource
 * - \ref strobe::page_size -- \copybrief strobe::page_size
 *
 * \subsection core_events Events:
 * - \ref strobe::BasicEvent -- \copybrief strobe::BasicEvent
 * - \ref strobe::EventListenerRef -- \copybrief strobe::EventListenerRef
 * - \ref strobe::EventListenerHandle -- \copybrief strobe::EventListenerHandle
 * - \ref strobe::EventDispatcher -- \copybrief strobe::EventDispatcher
 *
 *
 */
