#include "strobe/rhi/rhi.hpp"
#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/handle_allocators.hpp"
#include "strobe/rhi/sync/sync.hpp"
#include "strobe/rhi/types/device_info.hpp"
#include "strobe/rhi/vulkan/context/create_info.hpp"

namespace strobe::rhi {

Device create_device(const DeviceInfo &info) {
  vulkan::ContextCreateInfo createInfo{};
  using vulkan::feature::disable;
  using vulkan::feature::optional;
  using vulkan::feature::required;

  createInfo.debug_utils = info.debug_utils ? required : disable;
  createInfo.surface = info.swapchain ? required : disable;
  createInfo.swapchain = info.swapchain ? required : disable;
  createInfo.timeline_semaphore = required;
  createInfo.shaderObjects = info.shaders ? required : disable;
#ifdef STROBE_TRACY
  createInfo.hostQueryReset = required;
  createInfo.calibratedTimestamps = required;
#else
  createInfo.hostQueryReset = optional;
  createInfo.calibratedTimestamps = optional;
#endif
  createInfo.bufferDeviceAddress = required;
  createInfo.raytracingPipeline = info.raytracing ? required : disable;
  const bool bvh = info.raytracing || info.rayQuery;
  createInfo.accelerationStructure = bvh ? required : disable;
  createInfo.deferredHostOperations = bvh ? required : disable;
  createInfo.rayQuery = info.rayQuery ? required : disable;
  createInfo.descriptorHeap = required;

  vulkan::QueueDescription queueDescriptions[]{
      // universal queue (maybe later of those later)
      vulkan::QueueDescription{
          .require = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
                     VK_QUEUE_TRANSFER_BIT,
          .present = info.swapchain ? vulkan::feature::required
                                    : vulkan::feature::optional,
          .available = vulkan::feature::required,
      },
      // async-transfer queue
      vulkan::QueueDescription{
          .require = VK_QUEUE_TRANSFER_BIT,
          .exclude = VK_QUEUE_GRAPHICS_BIT,
          .available = vulkan::feature::optional,
      },
  };
  createInfo.queue_count =
      sizeof(queueDescriptions) / sizeof(vulkan::QueueDescription);
  createInfo.pQueues = queueDescriptions;

  // create context
  Context context{createInfo};

  // setup handle / subsystem allocators.
  auto *allocs =
      static_cast<handle_allocators *>(context.get_allocator().allocate(
          sizeof(handle_allocators), alignof(handle_allocators)));
  std::construct_at(allocs, context.get_allocator());
  context.delete_hook(allocs, [](void *ptr) noexcept {
    auto *allocs = static_cast<handle_allocators *>(ptr);
    auto alloc = allocs->alloc;
    alloc.deallocate(allocs, sizeof(handle_allocators),
                     alignof(handle_allocators));
  });

  // create pools
  FencePool fencePool = sync::create_fence_pool(context, &allocs->syncAlloc);
  BinarySemaphorePool semPool =
      sync::create_binary_pool(context, &allocs->syncAlloc);
  MemoryPool memory = mem::create_mem_pool(context, &allocs->memAlloc);
  StagingPool staging = stage::create_staging_pool(memory, &allocs->stageAlloc);
  ScratchBuffer scratch = bvh::create_scratch(memory, &allocs->bvhAlloc);

  vulkan::Queue queue0 = context.ctx()->queue(0);
  vulkan::Queue queue1 = context.ctx()->queue(1);

  Vector<Timeline, strobe::rhi::allocator_ref> timelines{allocs->alloc};
  timelines.reserve(2);

  Timeline universalTimeline =
      sync::create_timeline(context, &allocs->syncAlloc);
  timelines.push_back(universalTimeline);

  Timeline dmaTimeline;
  if (queue1) {
    dmaTimeline = sync::create_timeline(context, &allocs->syncAlloc);
    timelines.push_back(dmaTimeline);
  }
  assert(universalTimeline);
  assert(dmaTimeline);

  GarbageCollector gc = gc::create_gc(context, timelines, &allocs->gcAlloc);

  assert(queue0 && "the universal queue is required!!");
  QueueFlags universalQueueFlags =
      QueueFlags::compute | QueueFlags::graphics | QueueFlags::transfer;
  if (info.swapchain) {
    universalQueueFlags |= QueueFlags::present;
  }
  Queue universalQueue = que::create_queue(
      universalTimeline, gc, queue0, universalQueueFlags, &allocs->queAlloc);

  Queue dmaQueue;
  if (queue1) {
    assert(dmaTimeline);
    dmaQueue = que::create_queue(dmaTimeline, gc, queue1, QueueFlags::transfer,
                                 &allocs->queAlloc);
  } else {
    dmaQueue = universalQueue;
  }

  return Device{make_void_handle<DeviceImpl>( //
      &allocs->deviceAlloc,                   //
      std::move(context),                     //
      std::move(fencePool),                   //
      std::move(semPool),                     //
      std::move(memory),                      //
      std::move(staging),                     //
      std::move(scratch),                     //
      std::move(gc),                          //
      std::move(universalQueue),              //
      std::move(dmaQueue)                     //
      )};
}

} // namespace strobe::rhi
