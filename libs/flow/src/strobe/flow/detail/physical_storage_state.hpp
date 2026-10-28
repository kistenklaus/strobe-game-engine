#pragma once

#include "strobe/flow/detail/program_data.hpp"

#include <strobe/core/containers/boxed_array.hpp>
#include <strobe/core/containers/span.hpp>
#include <strobe/core/sync/job_scheduler.hpp>

#include <algorithm>
#include <cstdint>
#include <stdexcept>

namespace strobe::flow::detail {

class PhysicalStorageState {
public:
  explicit PhysicalStorageState(const ProgramData &data,
                                allocator_ref allocator)
      : m_resource_acquisition_counts(data.resources.size(), allocator),
        m_frontier_counts(data.physical_storage.frontier_offsets.empty()
                              ? 0
                              : data.physical_storage.frontier_offsets.size() -
                                    1,
                          allocator),
        m_frontier_jobs(data.physical_storage.frontier_offsets.empty()
                            ? 0
                            : data.physical_storage.frontier_offsets.back(),
                        allocator),
        m_resource_slot_offsets(data.physical_storage.resource_slot_offsets),
        m_physical_slot_ids(data.physical_storage.slot_physical_ids),
        m_frontier_offsets(data.physical_storage.frontier_offsets) {
    if (data.physical_storage.resource_slot_offsets.size() !=
        data.resources.size() + 1) {
      throw std::logic_error("physical storage plan is incomplete");
    }
    if (data.physical_storage.frontier_offsets.empty()) {
      throw std::logic_error("physical storage frontier plan is incomplete");
    }
  }

  std::uint32_t acquire(ResourceId resource) const {
    validate_resource(resource);
    const auto first = m_resource_slot_offsets[resource];
    const auto end = m_resource_slot_offsets[resource + 1];
    if (first == end) {
      return null_node_id;
    }
    return static_cast<std::uint32_t>(m_resource_acquisition_counts[resource] %
                                      (end - first));
  }

  std::uint32_t physical_slot(ResourceId resource,
                              std::uint32_t logical_slot) const {
    validate_resource(resource);
    const auto first = m_resource_slot_offsets[resource];
    const auto end = m_resource_slot_offsets[resource + 1];
    if (logical_slot >= end - first) {
      throw std::logic_error("physical storage slot is out of range");
    }
    const auto physical_index = first + logical_slot;
    if (physical_index >= m_physical_slot_ids.size()) {
      throw std::logic_error("physical storage slot is out of range");
    }
    return m_physical_slot_ids[physical_index];
  }

  span<const sync::job_id> frontier(std::uint32_t physical_slot) const {
    validate_physical_slot(physical_slot);
    const auto offset = m_frontier_offsets[physical_slot];
    return {m_frontier_jobs.data() + offset, m_frontier_counts[physical_slot]};
  }

  void clear_frontier(std::uint32_t physical_slot) {
    validate_physical_slot(physical_slot);
    m_frontier_counts[physical_slot] = 0;
  }

  void replace_frontier(std::uint32_t physical_slot, sync::job_id job) {
    validate_physical_slot(physical_slot);
    const auto offset = m_frontier_offsets[physical_slot];
    if (offset >= m_frontier_jobs.size()) {
      throw std::logic_error("physical storage frontier is invalid");
    }
    m_frontier_jobs[offset] = job;
    m_frontier_counts[physical_slot] = 1;
  }

  void append_frontier(std::uint32_t physical_slot, sync::job_id job) {
    validate_physical_slot(physical_slot);
    const auto offset = m_frontier_offsets[physical_slot];
    auto &count = m_frontier_counts[physical_slot];
    for (std::uint32_t index = 0; index < count; ++index) {
      if (m_frontier_jobs[offset + index] == job) {
        return;
      }
    }
    if (offset + count >= m_frontier_offsets[physical_slot + 1]) {
      throw std::logic_error("physical storage frontier is exhausted");
    }
    m_frontier_jobs[offset + count++] = job;
  }

  void advance(ResourceId resource) {
    validate_resource(resource);
    ++m_resource_acquisition_counts[resource];
  }

private:
  void validate_resource(ResourceId resource) const {
    if (resource >= m_resource_acquisition_counts.size() ||
        resource + 1 >= m_resource_slot_offsets.size()) {
      throw std::logic_error("physical storage resource is out of range");
    }
  }

  void validate_physical_slot(std::uint32_t physical_slot) const {
    if (physical_slot >= m_frontier_counts.size() ||
        physical_slot + 1 >= m_frontier_offsets.size()) {
      throw std::logic_error("physical storage slot is invalid");
    }
  }

  BoxedArray<std::uint64_t, allocator_ref> m_resource_acquisition_counts;
  BoxedArray<std::uint32_t, allocator_ref> m_frontier_counts;
  BoxedArray<sync::job_id, allocator_ref> m_frontier_jobs;
  span<const std::uint32_t> m_resource_slot_offsets;
  span<const std::uint32_t> m_physical_slot_ids;
  span<const std::uint32_t> m_frontier_offsets;
};

} // namespace strobe::flow::detail
