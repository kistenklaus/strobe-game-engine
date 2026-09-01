#include "strobe/rhi/vulkan/query_pool.hpp"
#include "strobe/rhi/error/vulkan_error.hpp"

#include <cassert>
#include <limits>
#include <vulkan/vulkan_core.h>

namespace strobe::rhi::vulkan {

QueryPool create_query_pool(Context *context, const QueryPoolInfo &info) {
  assert(context != nullptr);
  assert(info.count != 0);
  const VkQueryPoolCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = info.flags,
      .queryType = info.type,
      .queryCount = info.count,
      .pipelineStatistics = info.pipeline_statistics,
  };
  QueryPool pool{};
  assert(context->device());
  const VkResult result = vkCreateQueryPool(
      context->device(), &createInfo, context->driver_alloc(), &pool.handle);
  if (result != VK_SUCCESS) {
    vulkan_error(result, "Failed to create Vulkan query pool");
  }
  return pool;
}

void destroy_query_pool(Context *context, QueryPool pool) noexcept {
  assert(context != nullptr);
  assert(pool);
  vkDestroyQueryPool(context->device(), pool.handle, context->driver_alloc());
}

bool get_query_pool_results_u32(Context *context, QueryPool pool,
                                span<uint32_t> results,
                                const QueryPoolQueryInfo &info) {
  assert(context != nullptr);
  assert(pool);
  assert(!results.empty());
  assert(info.valuesPerQuery != 0);
  assert(results.size() % info.valuesPerQuery == 0);
  const size_t queryCount = results.size() / info.valuesPerQuery;
  assert(queryCount <= std::numeric_limits<uint32_t>::max());

  VkQueryResultFlags flags = 0;
  if (info.wait) {
    flags |= VK_QUERY_RESULT_WAIT_BIT;
  }
  const VkResult result = vkGetQueryPoolResults(
      context->device(), pool.handle, info.firstQuery,
      static_cast<uint32_t>(queryCount), results.size_bytes(), results.data(),
      sizeof(uint32_t) * info.valuesPerQuery, flags);
  if (result != VK_SUCCESS && result != VK_NOT_READY) {
    vulkan_error(result, "Failed to get query pool results");
  }
  return result == VK_SUCCESS;
}

bool get_query_pool_results_u64(Context *context, QueryPool pool,
                                span<uint64_t> results,
                                const QueryPoolQueryInfo &info) {
  assert(context != nullptr);
  assert(pool);
  assert(!results.empty());
  assert(info.valuesPerQuery != 0);
  assert(results.size() % info.valuesPerQuery == 0);
  const size_t queryCount = results.size() / info.valuesPerQuery;
  assert(queryCount <= std::numeric_limits<uint32_t>::max());

  VkQueryResultFlags flags = VK_QUERY_RESULT_64_BIT;
  if (info.wait) {
    flags |= VK_QUERY_RESULT_WAIT_BIT;
  }
  const VkResult result = vkGetQueryPoolResults(
      context->device(), pool.handle, info.firstQuery,
      static_cast<uint32_t>(queryCount), results.size_bytes(), results.data(),
      sizeof(uint64_t) * info.valuesPerQuery, flags);
  if (result != VK_SUCCESS && result != VK_NOT_READY) {
    vulkan_error(result, "Failed to get query pool results");
  }
  return result == VK_SUCCESS;
}

void reset_query_pool(Context *context, QueryPool pool, uint32_t first_query,
                      uint32_t query_count) noexcept {
  assert(context != nullptr);
  assert(pool);
  assert(query_count != 0);
  vkResetQueryPool(context->device(), pool.handle, first_query, query_count);
}

} // namespace strobe::rhi::vulkan
