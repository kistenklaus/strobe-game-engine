#pragma once

#include "strobe/gpu/vulkan/context/context.hpp"

#include <cassert>
#include <cstdint>

#include <vulkan/vulkan_core.h>

namespace strobe::gpu::vulkan {

struct QueryPool {
  VkQueryPool handle = VK_NULL_HANDLE;

  [[nodiscard]]
  explicit operator bool() const noexcept {
    return handle != VK_NULL_HANDLE;
  }
};

struct QueryPoolInfo {
  VkQueryPoolCreateFlags flags = 0;

  VkQueryType type = VK_QUERY_TYPE_TIMESTAMP;

  uint32_t count = 0;

  VkQueryPipelineStatisticFlags pipeline_statistics = 0;
};

[[nodiscard]]
QueryPool create_query_pool(Context *context, const QueryPoolInfo &info);

void destroy_query_pool(Context *context, QueryPool pool) noexcept;

struct QueryPoolQueryInfo {
  uint32_t firstQuery = 0;
  uint32_t valuesPerQuery = 1;
  bool wait = false;
};

bool get_query_pool_results_u32(Context *context, QueryPool pool,
                                span<uint32_t> results,
                                const QueryPoolQueryInfo &info = {});

bool get_query_pool_results_u64(Context *context, QueryPool pool,
                                span<uint64_t> results,
                                const QueryPoolQueryInfo &info = {});

void reset_query_pool(Context *context, QueryPool pool, uint32_t first_query,
                      uint32_t query_count) noexcept;

} // namespace strobe::gpu::vulkan
