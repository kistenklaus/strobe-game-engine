#include "strobe/rhi/memory/memory_lifetime_allocator.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include "strobe/core/memory/align.hpp"
#include <cstddef>
#include <type_traits>

namespace strobe::rhi {

VkDeviceSize MemoryLifetimeAllocator::greedy_pack_intervals(
    std::span<AllocationInfo *> allocations, std::span<VkDeviceSize> offsets,
    VkDeviceSize bufferImageGranularity,
    MemoryLifetimeAllocator::allocator &alloc) {
  ZoneScopedN("greedy-pack-intervals");

  assert(offsets.empty() || allocations.size() == offsets.size());

  const size_t intervalCount = allocations.size();

  ZoneValue(static_cast<uint64_t>(intervalCount));

  if (intervalCount == 0)
    return 0;

  assert(bufferImageGranularity != 0);
  assert(std::has_single_bit(bufferImageGranularity));

  struct ActiveRangeNode {
    ActiveRangeNode *previous;
    ActiveRangeNode *next;
    MemoryGranularityClass granularity;
  };

  struct Hole {
    VkDeviceSize begin;
    VkDeviceSize end;
    ActiveRangeNode *leftActive;
    ActiveRangeNode *rightActive;
  };

  static constexpr uint32_t EVENT_BEGIN_BIT = uint32_t{1} << 31;

  static constexpr uint32_t EVENT_INDEX_MASK = ~EVENT_BEGIN_BIT;

  struct Event {
    uint32_t position;
    uint32_t indexAndFlags;
  };

  assert(intervalCount <= static_cast<size_t>(EVENT_INDEX_MASK));

  const size_t eventCount = intervalCount * 2;

  assert(eventCount <= std::numeric_limits<uint32_t>::max());

  const AllocationInfo *referenceInfo = allocations.front();

  assert(referenceInfo != nullptr);

  const VkDeviceSize commonAlignment = referenceInfo->reservation.alignment;

  const VkDeviceSize referenceSize = referenceInfo->size;

  const uint32_t referencePosition = referenceInfo->lifetime.begin;

  const MemoryGranularityClass commonGranularity = referenceInfo->granularity;

  VkDeviceSize largestAlignment = 1;

  uint64_t sizeVariationMask = 0;
  uint32_t positionVariationMask = 0;

  bool homogeneousAlignment = true;
  bool homogeneousGranularity = true;

  {
    ZoneScopedN("pack/scan-input");

    for (const AllocationInfo *info : allocations) {
      assert(info != nullptr);
      assert(info->lifetime.begin < info->lifetime.end);
      assert(info->size != 0);

      const VkDeviceSize alignment = info->reservation.alignment;

      assert(alignment != 0);
      assert(std::has_single_bit(alignment));

      largestAlignment = std::max(largestAlignment, alignment);

      homogeneousAlignment &= alignment == commonAlignment;

      homogeneousGranularity &= info->granularity == commonGranularity;

      sizeVariationMask |= static_cast<uint64_t>(info->size ^ referenceSize);

      positionVariationMask |= info->lifetime.begin ^ referencePosition;

      positionVariationMask |= info->lifetime.end ^ referencePosition;
    }
  }

  const uint32_t sizeKeyBits =
      static_cast<uint32_t>(std::bit_width(sizeVariationMask));

  const uint32_t positionKeyBits =
      static_cast<uint32_t>(std::bit_width(positionVariationMask));

  const VkDeviceSize domainAlignment =
      std::max(largestAlignment, bufferImageGranularity);

  VkDeviceSize domainSize = 0;

  {
    ZoneScopedN("pack/compute-domain");

    for (const AllocationInfo *info : allocations) {
      assert(info->size <=
             std::numeric_limits<VkDeviceSize>::max() - (domainAlignment - 1));

      const VkDeviceSize slotSize =
          (info->size + domainAlignment - 1) & ~(domainAlignment - 1);

      assert(domainSize <= std::numeric_limits<VkDeviceSize>::max() - slotSize);

      domainSize += slotSize;
    }
  }

  static constexpr size_t RADIX_SORT_EVENT_THRESHOLD = 256;

  const bool useRadixSort = eventCount >= RADIX_SORT_EVENT_THRESHOLD;

  const size_t holeCapacity = intervalCount + 1;

  const bool ownsPlacementOffsets = offsets.empty();

  const size_t radixEventSize = eventCount * sizeof(Event);

  const size_t radixSizeKeyOffset =
      (radixEventSize + alignof(uint64_t) - 1) & ~(alignof(uint64_t) - 1);

  const size_t radixSizeKeySize = intervalCount * sizeof(uint64_t);

  const size_t radixAlignmentKeyOffset = radixSizeKeyOffset + radixSizeKeySize;

  const size_t radixAlignmentKeySize = intervalCount * sizeof(uint8_t);

  const size_t radixStorageSize =
      radixAlignmentKeyOffset + radixAlignmentKeySize;

  const size_t holeStorageSize =
      useRadixSort ? std::max(holeCapacity * sizeof(Hole), radixStorageSize)
                   : holeCapacity * sizeof(Hole);

  size_t scratchSize = 0;

  auto reserve_scratch = [&](size_t size, size_t alignment) {
    assert(alignment != 0);
    assert(std::has_single_bit(alignment));

    scratchSize = (scratchSize + alignment - 1) & ~(alignment - 1);

    const size_t offset = scratchSize;

    scratchSize += size;

    return offset;
  };

  const size_t holeStorageOffset =
      reserve_scratch(holeStorageSize, alignof(std::max_align_t));

  const size_t activeNodeOffset = reserve_scratch(
      intervalCount * sizeof(ActiveRangeNode), alignof(ActiveRangeNode));

  size_t placementOffsetOffset = 0;

  if (ownsPlacementOffsets) {
    placementOffsetOffset = reserve_scratch(
        intervalCount * sizeof(VkDeviceSize), alignof(VkDeviceSize));
  }

  const size_t eventOffset =
      reserve_scratch(eventCount * sizeof(Event), alignof(Event));

  using alloc_traits = AllocatorTraits<std::remove_cvref_t<decltype(alloc)>>;

  void *scratch = nullptr;

  {
    ZoneScopedN("pack/allocate-scratch");

    scratch =
        alloc_traits::allocate(alloc, scratchSize, alignof(std::max_align_t));
  }

  assert(scratch != nullptr);

  auto *scratchBytes = static_cast<std::byte *>(scratch);

  auto *holeStorage = scratchBytes + holeStorageOffset;

  auto *activeNodes =
      reinterpret_cast<ActiveRangeNode *>(scratchBytes + activeNodeOffset);

  auto *events = reinterpret_cast<Event *>(scratchBytes + eventOffset);

  VkDeviceSize *placementOffsets =
      ownsPlacementOffsets ? reinterpret_cast<VkDeviceSize *>(
                                 scratchBytes + placementOffsetOffset)
                           : offsets.data();

  {
    ZoneScopedN("pack/initialize-scratch");

    std::uninitialized_default_construct_n(activeNodes, intervalCount);

    std::uninitialized_default_construct_n(events, eventCount);

    if (ownsPlacementOffsets) {
      std::uninitialized_default_construct_n(placementOffsets, intervalCount);
    }

    for (size_t i = 0; i < intervalCount; ++i) {
      activeNodes[i] = ActiveRangeNode{
          .previous = nullptr,
          .next = nullptr,
          .granularity = allocations[i]->granularity,
      };
    }
  }

  {
    ZoneScopedN("pack/build-events");

    for (size_t i = 0; i < intervalCount; ++i) {
      const uint32_t intervalIndex = static_cast<uint32_t>(i);

      events[2 * i] = Event{
          .position = allocations[i]->lifetime.begin,

          .indexAndFlags = intervalIndex | EVENT_BEGIN_BIT,
      };

      events[2 * i + 1] = Event{
          .position = allocations[i]->lifetime.end,

          .indexAndFlags = intervalIndex,
      };
    }
  }

  {
    ZoneScopedN("pack/sort-events");

    if (!useRadixSort) {
      std::sort(
          events, events + eventCount, [&](const Event &a, const Event &b) {
            if (a.position != b.position) {
              return a.position < b.position;
            }

            const bool aBegins = (a.indexAndFlags & EVENT_BEGIN_BIT) != 0;

            const bool bBegins = (b.indexAndFlags & EVENT_BEGIN_BIT) != 0;

            if (aBegins != bBegins)
              return !aBegins;

            const uint32_t aIndex = a.indexAndFlags & EVENT_INDEX_MASK;

            const uint32_t bIndex = b.indexAndFlags & EVENT_INDEX_MASK;

            if (aBegins) {
              const AllocationInfo *aInfo = allocations[aIndex];

              const AllocationInfo *bInfo = allocations[bIndex];

              if (!homogeneousAlignment && aInfo->reservation.alignment !=
                                               bInfo->reservation.alignment) {
                return aInfo->reservation.alignment >
                       bInfo->reservation.alignment;
              }

              if (aInfo->size != bInfo->size) {
                return aInfo->size > bInfo->size;
              }
            }

            return aIndex < bIndex;
          });
    } else {
      static constexpr uint32_t RADIX_BITS = 11;

      static constexpr uint32_t RADIX_SIZE = uint32_t{1} << RADIX_BITS;

      auto *temporaryEvents = reinterpret_cast<Event *>(holeStorage);

      auto *descendingSizeKeys =
          reinterpret_cast<uint64_t *>(holeStorage + radixSizeKeyOffset);

      auto *alignmentKeys =
          reinterpret_cast<uint8_t *>(holeStorage + radixAlignmentKeyOffset);

      std::uninitialized_default_construct_n(temporaryEvents, eventCount);

      std::uninitialized_default_construct_n(descendingSizeKeys, intervalCount);

      std::uninitialized_default_construct_n(alignmentKeys, intervalCount);

      if (sizeKeyBits != 0) {
        for (size_t i = 0; i < intervalCount; ++i) {
          descendingSizeKeys[i] = ~static_cast<uint64_t>(allocations[i]->size);
        }
      }

      if (!homogeneousAlignment) {
        for (size_t i = 0; i < intervalCount; ++i) {
          const unsigned exponent = static_cast<unsigned>(
              std::countr_zero(allocations[i]->reservation.alignment));

          alignmentKeys[i] = static_cast<uint8_t>(1u + (63u - exponent));
        }
      }

      uint32_t histogram[RADIX_SIZE];

      Event *source = events;

      Event *destination = temporaryEvents;

      auto radix_pass = [&](auto &&getDigit, uint32_t bucketCount) {
        assert(bucketCount <= RADIX_SIZE);

        std::fill_n(histogram, bucketCount, uint32_t{0});

        for (size_t i = 0; i < eventCount; ++i) {
          const uint32_t digit = getDigit(source[i]);

          assert(digit < bucketCount);

          ++histogram[digit];
        }

        uint32_t destinationOffset = 0;

        for (uint32_t bucket = 0; bucket < bucketCount; ++bucket) {
          const uint32_t count = histogram[bucket];

          histogram[bucket] = destinationOffset;

          destinationOffset += count;
        }

        assert(destinationOffset == eventCount);

        for (size_t i = 0; i < eventCount; ++i) {
          const Event event = source[i];

          const uint32_t digit = getDigit(event);

          destination[histogram[digit]++] = event;
        }

        std::swap(source, destination);
      };

      /*
       * The bits above sizeKeyBits are identical for every begin
       * event, so they cannot affect ordering and require no pass.
       */
      for (uint32_t shift = 0; shift < sizeKeyBits; shift += RADIX_BITS) {
        const uint32_t passBits = std::min(RADIX_BITS, sizeKeyBits - shift);

        const uint32_t bucketCount = uint32_t{1} << passBits;

        const uint64_t digitMask = static_cast<uint64_t>(bucketCount - 1);

        radix_pass(
            [&](const Event &event) {
              if ((event.indexAndFlags & EVENT_BEGIN_BIT) == 0) {
                return uint32_t{0};
              }

              const uint32_t intervalIndex =
                  event.indexAndFlags & EVENT_INDEX_MASK;

              return static_cast<uint32_t>(
                  (descendingSizeKeys[intervalIndex] >> shift) & digitMask);
            },
            bucketCount);
      }

      /*
       * This pass places end events before begin events. For
       * heterogeneous alignments, it also orders begin events by
       * decreasing alignment.
       */
      if (homogeneousAlignment) {
        radix_pass(
            [&](const Event &event) {
              return (event.indexAndFlags & EVENT_BEGIN_BIT) != 0 ? uint32_t{1}
                                                                  : uint32_t{0};
            },
            2);
      } else {
        radix_pass(
            [&](const Event &event) {
              if ((event.indexAndFlags & EVENT_BEGIN_BIT) == 0) {
                return uint32_t{0};
              }

              const uint32_t intervalIndex =
                  event.indexAndFlags & EVENT_INDEX_MASK;

              return static_cast<uint32_t>(alignmentKeys[intervalIndex]);
            },
            65);
      }

      /*
       * Likewise, position bits above positionKeyBits are identical
       * for every event and can be skipped.
       */
      for (uint32_t shift = 0; shift < positionKeyBits; shift += RADIX_BITS) {
        const uint32_t passBits = std::min(RADIX_BITS, positionKeyBits - shift);

        const uint32_t bucketCount = uint32_t{1} << passBits;

        const uint32_t digitMask = bucketCount - 1;

        radix_pass(
            [&](const Event &event) {
              return (event.position >> shift) & digitMask;
            },
            bucketCount);
      }

      /*
       * The adaptive number of passes may be odd. Hole storage must
       * be released after sorting, so copy the final sequence back
       * when it currently resides in the temporary array.
       */
      if (source != events) {
        std::copy_n(source, eventCount, events);
      }

      std::destroy_n(alignmentKeys, intervalCount);

      std::destroy_n(descendingSizeKeys, intervalCount);

      std::destroy_n(temporaryEvents, eventCount);
    }
  }

  auto *holes = reinterpret_cast<Hole *>(holeStorage);

  std::uninitialized_default_construct_n(holes, holeCapacity);

  size_t holeCount = 1;

  holes[0] = Hole{
      .begin = 0,
      .end = domainSize,
      .leftActive = nullptr,
      .rightActive = nullptr,
  };

  VkDeviceSize packedSize = 0;

  auto align_down = [](VkDeviceSize value, VkDeviceSize alignment) {
    return value & ~(alignment - 1);
  };

  {
    ZoneScopedN("pack/line-sweep");

    for (size_t eventIndex = 0; eventIndex < eventCount; ++eventIndex) {
      const Event &event = events[eventIndex];

      const bool begins = (event.indexAndFlags & EVENT_BEGIN_BIT) != 0;

      const uint32_t intervalIndex = event.indexAndFlags & EVENT_INDEX_MASK;

      AllocationInfo &info = *allocations[intervalIndex];

      ActiveRangeNode &active = activeNodes[intervalIndex];

      if (begins) {
        const VkDeviceSize alignment =
            homogeneousAlignment ? commonAlignment : info.reservation.alignment;

        size_t selectedHoleIndex = holeCount;

        VkDeviceSize placementOffset = 0;

        auto find_hole = [&](bool applyGranularity) {
          for (size_t holeIndex = 0; holeIndex < holeCount; ++holeIndex) {
            const Hole &hole = holes[holeIndex];

            assert(hole.begin < hole.end);

            if (hole.end - hole.begin < info.size) {
              continue;
            }

            VkDeviceSize usableBegin = hole.begin;

            VkDeviceSize usableEnd = hole.end;

            if (applyGranularity) {
              if (hole.leftActive != nullptr &&
                  hole.leftActive->granularity != info.granularity) {
                usableBegin = strobe::memory::align_up(usableBegin,
                                                       bufferImageGranularity);
              }

              if (hole.rightActive != nullptr &&
                  hole.rightActive->granularity != info.granularity) {
                usableEnd = align_down(usableEnd, bufferImageGranularity);
              }
            }

            usableBegin = strobe::memory::align_up(usableBegin, alignment);

            if (usableBegin > usableEnd) {
              continue;
            }

            if (info.size > usableEnd - usableBegin) {
              continue;
            }

            selectedHoleIndex = holeIndex;

            placementOffset = usableBegin;

            return;
          }
        };

        if (homogeneousGranularity)
          find_hole(false);
        else
          find_hole(true);

        assert(selectedHoleIndex < holeCount);

        const Hole selectedHole = holes[selectedHoleIndex];

        const VkDeviceSize allocationEnd = placementOffset + info.size;

        assert(placementOffset >= selectedHole.begin);

        assert(allocationEnd <= selectedHole.end);

        ActiveRangeNode *leftActive = selectedHole.leftActive;

        ActiveRangeNode *rightActive = selectedHole.rightActive;

        active.previous = leftActive;

        active.next = rightActive;

        if (leftActive != nullptr)
          leftActive->next = &active;

        if (rightActive != nullptr)
          rightActive->previous = &active;

        placementOffsets[intervalIndex] = placementOffset;

        packedSize = std::max(packedSize, allocationEnd);

        const bool hasPrefix = selectedHole.begin < placementOffset;

        const bool hasSuffix = allocationEnd < selectedHole.end;

        if (hasPrefix && hasSuffix) {
          assert(holeCount < holeCapacity);

          std::move_backward(holes + selectedHoleIndex + 1, holes + holeCount,
                             holes + holeCount + 1);

          holes[selectedHoleIndex] = Hole{
              .begin = selectedHole.begin,

              .end = placementOffset,

              .leftActive = leftActive,

              .rightActive = &active,
          };

          holes[selectedHoleIndex + 1] = Hole{
              .begin = allocationEnd,

              .end = selectedHole.end,

              .leftActive = &active,

              .rightActive = rightActive,
          };

          ++holeCount;
        } else if (hasPrefix) {
          holes[selectedHoleIndex] = Hole{
              .begin = selectedHole.begin,

              .end = placementOffset,

              .leftActive = leftActive,

              .rightActive = &active,
          };
        } else if (hasSuffix) {
          holes[selectedHoleIndex] = Hole{
              .begin = allocationEnd,

              .end = selectedHole.end,

              .leftActive = &active,

              .rightActive = rightActive,
          };
        } else {
          std::move(holes + selectedHoleIndex + 1, holes + holeCount,
                    holes + selectedHoleIndex);

          --holeCount;
        }

        continue;
      }

      const VkDeviceSize allocationBegin = placementOffsets[intervalIndex];

      assert(allocationBegin <=
             std::numeric_limits<VkDeviceSize>::max() - info.size);

      const VkDeviceSize allocationEnd = allocationBegin + info.size;

      const Hole *position =
          std::lower_bound(holes, holes + holeCount, allocationBegin,
                           [](const Hole &hole, VkDeviceSize value) {
                             return hole.begin < value;
                           });

      const size_t insertionIndex = static_cast<size_t>(position - holes);

      const bool hasLeftHole = insertionIndex > 0 &&
                               holes[insertionIndex - 1].end == allocationBegin;

      const bool hasRightHole = insertionIndex < holeCount &&
                                holes[insertionIndex].begin == allocationEnd;

      if (insertionIndex < holeCount) {
        assert(holes[insertionIndex].begin >= allocationEnd);
      }

      ActiveRangeNode *leftActive = active.previous;

      ActiveRangeNode *rightActive = active.next;

      if (leftActive != nullptr)
        leftActive->next = rightActive;

      if (rightActive != nullptr)
        rightActive->previous = leftActive;

      active.previous = nullptr;
      active.next = nullptr;

      if (hasLeftHole && hasRightHole) {
        const size_t leftIndex = insertionIndex - 1;

        const size_t rightIndex = insertionIndex;

        holes[leftIndex] = Hole{
            .begin = holes[leftIndex].begin,

            .end = holes[rightIndex].end,

            .leftActive = leftActive,

            .rightActive = rightActive,
        };

        std::move(holes + rightIndex + 1, holes + holeCount,
                  holes + rightIndex);

        --holeCount;
      } else if (hasLeftHole) {
        Hole &leftHole = holes[insertionIndex - 1];

        leftHole.end = allocationEnd;

        leftHole.rightActive = rightActive;
      } else if (hasRightHole) {
        Hole &rightHole = holes[insertionIndex];

        rightHole.begin = allocationBegin;

        rightHole.leftActive = leftActive;
      } else {
        assert(holeCount < holeCapacity);

        std::move_backward(holes + insertionIndex, holes + holeCount,
                           holes + holeCount + 1);

        holes[insertionIndex] = Hole{
            .begin = allocationBegin,

            .end = allocationEnd,

            .leftActive = leftActive,

            .rightActive = rightActive,
        };

        ++holeCount;
      }
    }
  }

  assert(holeCount == 1);
  assert(holes[0].begin == 0);
  assert(holes[0].end == domainSize);
  assert(holes[0].leftActive == nullptr);
  assert(holes[0].rightActive == nullptr);

  {
    ZoneScopedN("pack/cleanup");

    std::destroy_n(holes, holeCapacity);

    std::destroy_n(events, eventCount);

    if (ownsPlacementOffsets) {
      std::destroy_n(placementOffsets, intervalCount);
    }

    std::destroy_n(activeNodes, intervalCount);

    alloc_traits::deallocate(alloc, scratch, scratchSize,
                             alignof(std::max_align_t));
  }

  return packedSize;
}

std::pair<size_t, uint32_t> MemoryLifetimeAllocator::greedy_preferred_grouping(
    uint32_t start, std::span<AllocationInfo *> reserved) {
  ZoneScopedN("greedy-preferred-grouping");
  assert(start < reserved.size());
  AllocationInfo *origin = reserved[start];
  assert(origin->state == AllocationState::reserved);
  const vulkan::MemoryUsage memoryUsage = origin->reservation.memoryUsage;
  const uint8_t preferredTypeIndex =
      origin->reservation.preferredMemoryTypeIndex;
  assert(preferredTypeIndex < 32);
  const uint32_t preferredTypeBit = uint32_t{1} << preferredTypeIndex;
  assert(origin->reservation.memoryTypeBits & preferredTypeBit);
  uint32_t commonMemoryTypeBits = origin->reservation.memoryTypeBits;
  size_t groupBegin = reserved.size();
  size_t i = 0;
  while (i < groupBegin) {
    AllocationInfo *info = reserved[i];
    assert(info->state == AllocationState::reserved);
    const bool sameUsage = info->reservation.memoryUsage == memoryUsage;
    const bool supportsPreferredType =
        info->reservation.memoryTypeBits & preferredTypeBit;
    if (sameUsage && supportsPreferredType) {
      commonMemoryTypeBits &= info->reservation.memoryTypeBits;
      --groupBegin;
      std::swap(reserved[i], reserved[groupBegin]);
      if (i < groupBegin) {
        reserved[i]->index = static_cast<uint32_t>(i);
      }
    } else {
      ++i;
    }
  }
  const size_t groupSize = reserved.size() - groupBegin;
  assert(groupSize != 0);
  assert(commonMemoryTypeBits & preferredTypeBit);
  return {
      groupSize,
      commonMemoryTypeBits,
  };
}
std::pair<size_t, uint32_t>
MemoryLifetimeAllocator::maximum_preferred_anchor_grouping(
    uint32_t start, std::span<AllocationInfo *> reserved) {
  ZoneScopedN("maximum-preferred-anchor-grouping");
  assert(start < reserved.size());

  AllocationInfo *origin = reserved[start];
  assert(origin != nullptr);
  assert(origin->state == AllocationState::reserved);

  const vulkan::MemoryUsage memoryUsage = origin->reservation.memoryUsage;

  const uint8_t preferredTypeIndex =
      origin->reservation.preferredMemoryTypeIndex;

  assert(preferredTypeIndex < 32);

  const uint32_t preferredTypeBit = uint32_t{1} << preferredTypeIndex;

  assert(origin->reservation.memoryTypeBits & preferredTypeBit);

  // Only preferred types observed among reservations with the same usage are
  // valid candidate anchors. Raw memoryTypeBits can contain types that do not
  // satisfy the MemoryUsage policy.
  uint32_t viableAnchorBits = 0;

  // For every memory type, measure the maximal compatible group it would
  // produce. Total covered bytes are the primary metric and allocation count
  // is the secondary metric.
  VkDeviceSize coveredBytes[32]{};
  size_t coveredCount[32]{};

  for (AllocationInfo *info : reserved) {
    assert(info != nullptr);
    assert(info->state == AllocationState::reserved);

    if (info->reservation.memoryUsage != memoryUsage)
      continue;

    const uint8_t infoPreferredTypeIndex =
        info->reservation.preferredMemoryTypeIndex;

    assert(infoPreferredTypeIndex < 32);

    const uint32_t infoPreferredTypeBit = uint32_t{1} << infoPreferredTypeIndex;

    assert(info->reservation.memoryTypeBits & infoPreferredTypeBit);

    // An anchor must be valid for the origin because the resulting group must
    // always contain the allocation that triggered the commit.
    if (origin->reservation.memoryTypeBits & infoPreferredTypeBit) {
      viableAnchorBits |= infoPreferredTypeBit;
    }

    // Accumulate this allocation into every origin-supported type group that
    // could contain it. Only observed preferred types are considered when the
    // winner is selected below.
    uint32_t supportedAnchorBits =
        info->reservation.memoryTypeBits & origin->reservation.memoryTypeBits;

    while (supportedAnchorBits != 0) {
      const uint32_t anchorBit =
          supportedAnchorBits & (~supportedAnchorBits + 1);

      supportedAnchorBits &= supportedAnchorBits - 1;

      const uint32_t typeIndex =
          static_cast<uint32_t>(std::countr_zero(anchorBit));

      assert(coveredBytes[typeIndex] <=
             std::numeric_limits<VkDeviceSize>::max() - info->size);

      coveredBytes[typeIndex] += info->size;
      ++coveredCount[typeIndex];
    }
  }

  assert(viableAnchorBits != 0);
  assert(viableAnchorBits & preferredTypeBit);

  // Start with the origin's preferred type. It therefore wins exact ties.
  uint32_t bestAnchorBit = preferredTypeBit;
  uint32_t bestTypeIndex = preferredTypeIndex;

  uint32_t remainingAnchorBits = viableAnchorBits & ~preferredTypeBit;

  while (remainingAnchorBits != 0) {
    const uint32_t anchorBit = remainingAnchorBits & (~remainingAnchorBits + 1);

    remainingAnchorBits &= remainingAnchorBits - 1;

    const uint32_t typeIndex =
        static_cast<uint32_t>(std::countr_zero(anchorBit));

    const bool coversMoreBytes =
        coveredBytes[typeIndex] > coveredBytes[bestTypeIndex];

    const bool coversEqualBytes =
        coveredBytes[typeIndex] == coveredBytes[bestTypeIndex];

    const bool coversMoreAllocations =
        coveredCount[typeIndex] > coveredCount[bestTypeIndex];

    if (coversMoreBytes || (coversEqualBytes && coversMoreAllocations)) {
      bestAnchorBit = anchorBit;
      bestTypeIndex = typeIndex;
    }
  }

  // Move the maximal group supporting the selected anchor into the tail.
  uint32_t commonMemoryTypeBits = origin->reservation.memoryTypeBits;

  size_t groupBegin = reserved.size();
  size_t i = 0;

  while (i < groupBegin) {
    AllocationInfo *info = reserved[i];

    assert(info != nullptr);
    assert(info->state == AllocationState::reserved);

    const bool sameUsage = info->reservation.memoryUsage == memoryUsage;

    const bool supportsBestAnchor =
        (info->reservation.memoryTypeBits & bestAnchorBit) != 0;

    if (sameUsage && supportsBestAnchor) {
      commonMemoryTypeBits &= info->reservation.memoryTypeBits;

      --groupBegin;

      std::swap(reserved[i], reserved[groupBegin]);

      // Selected allocations are about to be committed. Only allocations
      // remaining in the prefix need valid reservation indices.
      if (i < groupBegin) {
        reserved[i]->index = static_cast<uint32_t>(i);
      }
    } else {
      ++i;
    }
  }

  const size_t groupSize = reserved.size() - groupBegin;

  assert(groupSize != 0);
  assert(commonMemoryTypeBits & bestAnchorBit);

  return {
      groupSize,
      commonMemoryTypeBits,
  };
}

} // namespace strobe::rhi
