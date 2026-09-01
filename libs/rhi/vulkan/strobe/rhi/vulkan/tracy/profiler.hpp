#pragma once

#include "strobe/rhi/vulkan/command_buffer.hpp"
#include <cassert>
#include <client/TracyCallstack.hpp>
#include <client/TracyProfiler.hpp>
#include <common/TracyAlign.hpp>
#include <common/TracyAlloc.hpp>
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>
#include <vulkan/vulkan.h>
#ifdef STROBE_TRACY
#include "strobe/rhi/utils/always_inline.hpp"
#include "strobe/rhi/vulkan/context/pnf.hpp"
#include "strobe/rhi/vulkan/query_pool.hpp"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <utility>
#endif

namespace strobe::rhi::profiler {

#ifdef STROBE_TRACY

class CommandBufferScope;
class CmdScope;

class Context {
  friend CommandBufferScope;
  friend CmdScope;

  static constexpr uint32_t QUERY_COUNT = 1 << 16;
  static constexpr uint8_t PAGE_INDEX_BITS = 6;
  static constexpr uint8_t QUERY_INDEX_BITS = 10;

  static constexpr uint16_t PAGE_INDEX_MASK =
      (uint16_t{1} << PAGE_INDEX_BITS) - uint16_t{1};
  static constexpr uint16_t QUERY_INDEX_MASK =
      (uint16_t{1} << QUERY_INDEX_BITS) - uint16_t{1};

  static constexpr uint16_t MAX_QUERY_RANGE = 1024 * 4;

public:
  explicit Context(vulkan::Context *context) noexcept
      : m_context(context),
        m_pool(vulkan::create_query_pool(m_context,
                                         {
                                             .type = VK_QUERY_TYPE_TIMESTAMP,
                                             .count = QUERY_COUNT,
                                         })),
        m_tracyCtxId(::tracy::GetGpuCtxCounter().fetch_add(
            1, std::memory_order_relaxed)) {
    assert(m_context != nullptr);
    assert(
        m_context->properties().calibratedTimestamps &&
        "strobe::tracy requires the VK_EXT_calibrated_timestamps extension!");
    assert(m_context->properties().hostQueryReset &&
           "strobe::tracy requires the VK_EXT_host_query_reset extension!");
    assert(m_tracyCtxId != 255);

    vulkan::reset_query_pool(m_context, m_pool, 0, QUERY_COUNT);

    selectTimeDomain();
    determineCalibrationDeviation();

    int64_t tGpu = 0;
    [[maybe_unused]] const bool calibrated =
        sampleCalibration(m_prevCalibration, tGpu);
    assert(calibrated);

    const int64_t tCpu = ::tracy::Profiler::GetTime();
    writeInitialItem(tCpu, tGpu);
  }

  Context(const Context &) = delete;
  Context(Context &&) = delete;
  Context &operator=(const Context &) = delete;
  Context &operator=(Context &&) = delete;

  ~Context() noexcept {
    // destroy tracy context.
    vulkan::destroy_query_pool(m_context, m_pool);
  }

  void setName(const char *name) {
    const size_t length = std::strlen(name);
    assert(length <= UINT16_MAX);

    auto *copy = static_cast<char *>(::tracy::tracy_malloc(length));
    std::memcpy(copy, name, length);

    auto item = ::tracy::Profiler::QueueSerial();
    ::tracy::MemWrite(&item->hdr.type, ::tracy::QueueType::GpuContextName);
    ::tracy::MemWrite(&item->gpuContextNameFat.context,
                      static_cast<uint8_t>(m_tracyCtxId));
    ::tracy::MemWrite(&item->gpuContextNameFat.ptr,
                      reinterpret_cast<uint64_t>(copy));
    ::tracy::MemWrite(&item->gpuContextNameFat.size,
                      static_cast<uint16_t>(length));

#ifdef TRACY_ON_DEMAND
    ::tracy::GetProfiler().DeferItem(*item);
#endif

    ::tracy::Profiler::QueueSerialFinish();
  }

private:
  STROBE_ALWAYS_INLINE void selectTimeDomain() noexcept {
    const auto &supportedTimeDomains =
        m_context->deviceInfo().properties.calibratableTimeDomains;

    VkTimeDomainEXT supportedDomain = static_cast<VkTimeDomainEXT>(-1);

#if defined _WIN32
    supportedDomain = VK_TIME_DOMAIN_QUERY_PERFORMANCE_COUNTER_EXT;
#elif defined __linux__ && defined CLOCK_MONOTONIC_RAW
    supportedDomain = VK_TIME_DOMAIN_CLOCK_MONOTONIC_RAW_EXT;
#endif

    m_timeDomain = VK_TIME_DOMAIN_DEVICE_EXT;

    for (uint32_t i = 0; i < supportedTimeDomains.size(); ++i) {
      if (supportedTimeDomains[i] == supportedDomain) {
        m_timeDomain = supportedTimeDomains[i];
        break;
      }
    }

    assert(m_timeDomain != VK_TIME_DOMAIN_DEVICE_EXT);
  }

  STROBE_ALWAYS_INLINE bool
  getCalibratedTimestamps(int64_t &tCpu, int64_t &tGpu,
                          uint64_t &deviation) noexcept {
    assert(m_timeDomain != VK_TIME_DOMAIN_DEVICE_EXT);

    const VkCalibratedTimestampInfoEXT timestampInfos[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT,
            .pNext = nullptr,
            .timeDomain = VK_TIME_DOMAIN_DEVICE_EXT,
        },
        {
            .sType = VK_STRUCTURE_TYPE_CALIBRATED_TIMESTAMP_INFO_EXT,
            .pNext = nullptr,
            .timeDomain = m_timeDomain,
        },
    };

    uint64_t timestamps[2];

    const VkResult result = vulkan::vk_get_calibrated_timestamps(
        m_context->pnf(), m_context->device(), 2, timestampInfos, timestamps,
        &deviation);

    if (result != VK_SUCCESS) {
      return false;
    }

    tGpu = static_cast<int64_t>(timestamps[0]);

#if defined _WIN32
    tCpu = static_cast<int64_t>(timestamps[1]) * m_qpcToNs;
#elif defined __linux__ && defined CLOCK_MONOTONIC_RAW
    tCpu = static_cast<int64_t>(timestamps[1]);
#else
    assert(false);
    return false;
#endif

    return true;
  }

  STROBE_ALWAYS_INLINE bool sampleCalibration(
      int64_t &tCpu, int64_t &tGpu,
      uint32_t maxSamples = std::numeric_limits<uint32_t>::max()) noexcept {
    for (uint32_t i = 0; i < maxSamples; ++i) {
      int64_t cpu;
      int64_t gpu;
      uint64_t deviation;

      if (!getCalibratedTimestamps(cpu, gpu, deviation)) {
        continue;
      }

      if (deviation > m_deviation) {
        continue;
      }

      tCpu = cpu;
      tGpu = gpu;
      return true;
    }

    return false;
  }

  void determineCalibrationDeviation() noexcept {
#if defined _WIN32
    m_qpcToNs = int64_t(1000000000. / ::tracy::GetFrequencyQpc());
#endif

    constexpr uint32_t NUM_PROBES = 32;

    uint64_t minDeviation = std::numeric_limits<uint64_t>::max();

    for (uint32_t i = 0; i < NUM_PROBES; ++i) {
      int64_t tCpu;
      int64_t tGpu;
      uint64_t deviation;

      if (!getCalibratedTimestamps(tCpu, tGpu, deviation)) {
        continue;
      }

      minDeviation = std::min(minDeviation, deviation);
    }

    assert(minDeviation != std::numeric_limits<uint64_t>::max());

    if (minDeviation == std::numeric_limits<uint64_t>::max()) {
      m_deviation = minDeviation;
    } else {
      m_deviation = minDeviation + minDeviation / 2;
    }
  }

  STROBE_ALWAYS_INLINE void writeInitialItem(int64_t tCpu, int64_t tGpu) {
    uint8_t flags = 0;

    if (m_timeDomain != VK_TIME_DOMAIN_DEVICE_EXT) {
      flags |= ::tracy::GpuContextCalibration;
    }

    const float period =
        m_context->deviceInfo().properties.limits.timestampPeriod;

    auto item = ::tracy::Profiler::QueueSerial();
    ::tracy::MemWrite(&item->hdr.type, ::tracy::QueueType::GpuNewContext);
    ::tracy::MemWrite(&item->gpuNewContext.cpuTime, tCpu);
    ::tracy::MemWrite(&item->gpuNewContext.gpuTime, tGpu);
    std::memset(&item->gpuNewContext.thread, 0,
                sizeof(item->gpuNewContext.thread));
    ::tracy::MemWrite(&item->gpuNewContext.period, period);
    ::tracy::MemWrite(&item->gpuNewContext.context,
                      static_cast<uint8_t>(m_tracyCtxId));
    ::tracy::MemWrite(&item->gpuNewContext.flags,
                      ::tracy::GpuContextFlags{flags});
    ::tracy::MemWrite(&item->gpuNewContext.type,
                      ::tracy::GpuContextType::Vulkan);

#ifdef TRACY_ON_DEMAND
    ::tracy::GetProfiler().DeferItem(*item);
#endif

    ::tracy::Profiler::QueueSerialFinish();
  }

  STROBE_ALWAYS_INLINE uint16_t nextPage() noexcept {
    uint64_t available = m_availablePages.load(std::memory_order_relaxed);

    while (available != 0) {
      const uint16_t page = static_cast<uint16_t>(std::countr_zero(available));
      const uint64_t bit = uint64_t{1} << page;

      const uint64_t previous =
          m_availablePages.fetch_and(~bit, std::memory_order_acquire);

      if ((previous & bit) != 0) {
        return page;
      }

      available = previous;
    }

    assert(false);
    std::unreachable();
  }

  STROBE_ALWAYS_INLINE void releasePages(uint64_t pages) noexcept {
    if (pages == 0) {
      return;
    }

    [[maybe_unused]] const uint64_t previous =
        m_availablePages.fetch_or(pages, std::memory_order_release);

    assert((previous & pages) == 0);
  }

  void collect(uint16_t firstQuery, uint16_t count, bool emitEvents) {
    ZoneScopedC(::tracy::Color::Red4);

    assert(count != 0);
    assert(uint32_t{firstQuery} + uint32_t{count} <= QUERY_COUNT);

    if (emitEvents) {
      uint32_t query = firstQuery;
      uint32_t remaining = count;

      while (remaining != 0) {
        const uint32_t size = std::min<uint32_t>(remaining, MAX_QUERY_RANGE);

        auto results = std::span{m_results.data(), size};

        vulkan::get_query_pool_results_u64(m_context, m_pool, results,
                                           {
                                               .firstQuery = query,
                                               .valuesPerQuery = 1,
                                               .wait = true,
                                           });

        for (uint32_t i = 0; i < size; ++i) {
          writeGpuTime(static_cast<uint16_t>(query + i),
                       static_cast<int64_t>(results[i]));
        }

        query += size;
        remaining -= size;
      }
    }

    vulkan::reset_query_pool(m_context, m_pool, firstQuery, count);
  }

  STROBE_ALWAYS_INLINE void updateCalibration(bool emitEvent) noexcept {
    int64_t tCpu = m_prevCalibration;
    int64_t tGpu = 0;

    if (!sampleCalibration(tCpu, tGpu, 10)) {
      return;
    }

    const int64_t refCpu = ::tracy::Profiler::GetTime();
    const int64_t delta = tCpu - m_prevCalibration;

    if (delta <= 0) {
      return;
    }

    m_prevCalibration = tCpu;

    if (emitEvent) {
      writeGpuCalibration(tGpu, refCpu, delta);
    }
  }

  STROBE_ALWAYS_INLINE void writeGpuTime(uint16_t queryId, int64_t timestamp) {
    auto item = ::tracy::Profiler::QueueSerial();
    ::tracy::MemWrite(&item->hdr.type, ::tracy::QueueType::GpuTime);
    ::tracy::MemWrite(&item->gpuTime.gpuTime, timestamp);
    ::tracy::MemWrite(&item->gpuTime.queryId, queryId);
    ::tracy::MemWrite(&item->gpuTime.context,
                      static_cast<uint8_t>(m_tracyCtxId));
    ::tracy::Profiler::QueueSerialFinish();
  }

  STROBE_ALWAYS_INLINE void writeGpuCalibration(int64_t tGpu, int64_t refCpu,
                                                int64_t delta) {
    auto item = ::tracy::Profiler::QueueSerial();
    ::tracy::MemWrite(&item->hdr.type, ::tracy::QueueType::GpuCalibration);
    ::tracy::MemWrite(&item->gpuCalibration.gpuTime, tGpu);
    ::tracy::MemWrite(&item->gpuCalibration.cpuTime, refCpu);
    ::tracy::MemWrite(&item->gpuCalibration.cpuDelta, delta);
    ::tracy::MemWrite(&item->gpuCalibration.context,
                      static_cast<uint8_t>(m_tracyCtxId));
    ::tracy::Profiler::QueueSerialFinish();
  }

  STROBE_ALWAYS_INLINE void
  writeGpuZoneBegin(uint16_t queryId,
                    const ::tracy::SourceLocationData *srcloc) {
    auto item = ::tracy::Profiler::QueueSerial();
    ::tracy::MemWrite(&item->hdr.type, ::tracy::QueueType::GpuZoneBeginSerial);
    ::tracy::MemWrite(&item->gpuZoneBegin.cpuTime,
                      ::tracy::Profiler::GetTime());
    ::tracy::MemWrite(&item->gpuZoneBegin.srcloc,
                      reinterpret_cast<uint64_t>(srcloc));
    ::tracy::MemWrite(&item->gpuZoneBegin.thread, ::tracy::GetThreadHandle());
    ::tracy::MemWrite(&item->gpuZoneBegin.queryId, queryId);
    ::tracy::MemWrite(&item->gpuZoneBegin.context,
                      static_cast<uint8_t>(m_tracyCtxId));
    ::tracy::Profiler::QueueSerialFinish();
  }

  STROBE_ALWAYS_INLINE void writeGpuZoneEnd(uint16_t queryId) {
    auto item = ::tracy::Profiler::QueueSerial();
    ::tracy::MemWrite(&item->hdr.type, ::tracy::QueueType::GpuZoneEndSerial);
    ::tracy::MemWrite(&item->gpuZoneEnd.cpuTime, ::tracy::Profiler::GetTime());
    ::tracy::MemWrite(&item->gpuZoneEnd.thread, ::tracy::GetThreadHandle());
    ::tracy::MemWrite(&item->gpuZoneEnd.queryId, queryId);
    ::tracy::MemWrite(&item->gpuZoneEnd.context,
                      static_cast<uint8_t>(m_tracyCtxId));
    ::tracy::Profiler::QueueSerialFinish();
  }

private:
  vulkan::Context *const m_context;
  const vulkan::QueryPool m_pool;
  const int32_t m_tracyCtxId;

  VkTimeDomainEXT m_timeDomain;
  uint64_t m_deviation;

#ifdef _WIN32
  int64_t m_qpcToNs;
#endif

  int64_t m_prevCalibration = 0;

  std::atomic<uint64_t> m_availablePages{std::numeric_limits<uint64_t>::max()};

  std::array<uint64_t, MAX_QUERY_RANGE> m_results{};
};

class CommandBufferScope {
  friend CmdScope;

public:
  explicit CommandBufferScope(Context *context) noexcept
      : m_context(context), m_pages(0), m_next(0) {}

  CommandBufferScope(const CommandBufferScope &) = delete;
  CommandBufferScope(CommandBufferScope &&) = delete;
  CommandBufferScope &operator=(const CommandBufferScope &) = delete;
  CommandBufferScope &operator=(CommandBufferScope &&) = delete;

  ~CommandBufferScope() noexcept {
    if (m_pages == 0) {
      return;
    }

#ifdef TRACY_ON_DEMAND
    const bool emitEvents = ::tracy::GetProfiler().IsConnected();
#else
    constexpr bool emitEvents = true;
#endif

    constexpr uint32_t PAGE_SIZE = uint32_t{1} << Context::QUERY_INDEX_BITS;
    constexpr uint32_t PAGE_COUNT = Context::QUERY_COUNT / PAGE_SIZE;

    const uint32_t lastQuery = m_next - 1;
    const uint32_t lastPage = lastQuery >> Context::QUERY_INDEX_BITS;
    const uint32_t lastPageSize = (lastQuery & Context::QUERY_INDEX_MASK) + 1;

    uint64_t remainingPages = m_pages;

    while (remainingPages != 0) {
      const uint32_t firstPage = std::countr_zero(remainingPages);

      uint32_t page = firstPage;
      uint32_t rangeSize = 0;

      while (page < PAGE_COUNT &&
             (remainingPages & (uint64_t{1} << page)) != 0) {
        remainingPages &= ~(uint64_t{1} << page);

        const uint32_t pageSize = page == lastPage ? lastPageSize : PAGE_SIZE;

        rangeSize += pageSize;
        ++page;

        if (pageSize != PAGE_SIZE) {
          break;
        }
      }

      uint32_t firstQuery = firstPage * PAGE_SIZE;

      while (rangeSize != 0) {
        const uint32_t chunkSize =
            std::min<uint32_t>(rangeSize, std::numeric_limits<uint16_t>::max());

        m_context->collect(static_cast<uint16_t>(firstQuery),
                           static_cast<uint16_t>(chunkSize), emitEvents);

        firstQuery += chunkSize;
        rangeSize -= chunkSize;
      }
    }

    m_context->releasePages(m_pages);
    m_context->updateCalibration(emitEvents);
  }

  uint16_t next() noexcept {
    const uint16_t query = m_next & Context::QUERY_INDEX_MASK;

    if (query == 0) {
      const uint16_t pageIndex = m_context->nextPage();

      m_next = (uint32_t{pageIndex} << Context::QUERY_INDEX_BITS) | query;

      m_pages |= uint64_t{1} << pageIndex;
    }

    return static_cast<uint16_t>(m_next++);
  }

private:
  Context *m_context;
  uint64_t m_pages;
  uint32_t m_next;
};

class CmdScope {
public:
  CmdScope(CommandBufferScope *scope, const ::tracy::SourceLocationData *srcloc,
           vulkan::CommandBuffer cmd, bool active)
      :
#ifdef TRACY_ON_DEMAND
        m_active(active && ::tracy::GetProfiler().IsConnected()),
        m_connectionId(::tracy::GetProfiler().ConnectionId()),
#else
        m_active(active),
#endif
        m_scope(scope), m_cmd(cmd) {
    if (!m_active) {
      return;
    }
    const uint16_t queryId = m_scope->next();

    vkCmdWriteTimestamp(m_cmd.handle, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                        m_scope->m_context->m_pool.handle, queryId);

    m_scope->m_context->writeGpuZoneBegin(queryId, srcloc);
  }

  CmdScope(const CmdScope &) = delete;
  CmdScope(CmdScope &&) = delete;
  CmdScope &operator=(const CmdScope &) = delete;
  CmdScope &operator=(CmdScope &&) = delete;

  ~CmdScope() noexcept {
    if (!m_active) {
      return;
    }

    const uint16_t queryId = m_scope->next();

    vkCmdWriteTimestamp(m_cmd.handle, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        m_scope->m_context->m_pool.handle, queryId);

#ifdef TRACY_ON_DEMAND
    if (::tracy::GetProfiler().ConnectionId() != m_connectionId) {
      return;
    }
#endif

    m_scope->m_context->writeGpuZoneEnd(queryId);
  }

private:
  const bool m_active;

#ifdef TRACY_ON_DEMAND
  const uint64_t m_connectionId;
#endif

  CommandBufferScope *const m_scope;
  const vulkan::CommandBuffer m_cmd;
};

#else

class Context {
public:
  explicit Context(vulkan::Context *) noexcept {}
  Context(const Context &) = delete;
  Context(Context &&) = delete;
  Context &operator=(const Context &) = delete;
  Context &operator=(Context &&) = delete;
  ~Context() noexcept = default;
  void setName(const char *) {}
};

class CommandBufferScope {
public:
  explicit CommandBufferScope(Context *) noexcept {}
  CommandBufferScope(const CommandBufferScope &) = delete;
  CommandBufferScope(CommandBufferScope &&) = delete;
  CommandBufferScope &operator=(const CommandBufferScope &) = delete;
  CommandBufferScope &operator=(CommandBufferScope &&) = delete;
  ~CommandBufferScope() noexcept {}
};

class CmdScope {
public:
  CmdScope(CommandBufferScope *, const ::tracy::SourceLocationData *,
           vulkan::CommandBuffer, bool) noexcept {}
  CmdScope(const CmdScope &) = delete;
  CmdScope(CmdScope &&) = delete;
  CmdScope &operator=(const CmdScope &) = delete;
  CmdScope &operator=(CmdScope &&) = delete;
  ~CmdScope() noexcept {}
};

#endif

} // namespace strobe::rhi::profiler
