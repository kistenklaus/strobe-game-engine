#pragma once

#include "strobe/rhi/objects/device.hpp"
#include "strobe/rhi/types/device_info.hpp"

/**
 * \defgroup rhi Render Hardware Interface
 * \brief Low-level Vulkan-oriented graphics and compute abstraction.
 *
 * The Render Hardware Interface (RHI) provides the low-level GPU API.
 * It wraps Vulkan objects while exposing explicit synchronization, memory
 * lifetime management, queues, command buffers, and resource descriptors.
 *
 * The RHI is primarily build around providing a API independent abstraction for 
 * graphics applications, while providing many useful quality of life features.
 *
 * \paragraph object_model Object Model:
 * The library is build around RAII reference counted shared pointers as the primary
 * tool of exposing graphics resources such as 
 * \ref strobe::rhi::Buffer "buffers", 
 * \ref strobe::rhi::Image "images", 
 * \ref strobe::rhi::CommandBuffer "command-buffers", 
 * and many more well known resource types.
 * RAII based abstractions such as the vulkan raii headers, traditionally have 
 * failed to deliver a intuitive APIs, as lifetime of objects has to be maintained on the 
 * host until execution finishes on the device. Our Object Model avoids this by
 * additionally tracking reference counts across device execution, which means 
 * that objects can at any point be dropped, without leading to device-host ownership 
 * problem. Objects without any host references, which are referenced by the device will
 * be destroyed once they are no longer reference by the device.
 * \note Device resource lifetime tracking can be implemented with timeline semaphores 
 * and command buffers which retain reference to all bound resources. 
 * This requires the RHI to posses a internal garbage collection thread, which contigously 
 * checks values of the timeline semaphores and drops references accordingly. 
 * This idea is based on a vulkanized 2026 talk, which can be found on youtube (https://youtu.be/0hvLRsPFAbQ?si=mtPPiwGDunG2zMc2).
 * 
 * \paragraph timelines Timelines, Backpressure, and Batching:
 * Once we already have a background thread, due to the garbage collector, we can do even more fun 
 * stuff with that. This paragraph is not important for the semantics of the public interface, but knowing 
 * about the internal implementation may help with writing more performant applications.
 *
 * The background thread knows which resources are submitted at which timeline value (\ref strobe::rhi::Timepoint "Timepoint") and 
 * because it waits on those timeline semaphores it additionally knows which submits have completed on the 
 * device. Combined this means that it knows how much submissions are live on a given queue. 
 * We use this as a backpressure mechanism where the background thread notifies once queue 
 * occupancy drops below a threshold. This asynchronous notification from the background
 * thread can then be used to implement batching. For example user submissions must not be immediately 
 * send to the GPU, if queue occupancy is already high, instead we batch those submissions together 
 * and once we get notified we submit everything in a larger batch, this avoids the 
 * driver overhead of many small submissions and doesn't significantly effect latencies as 
 * high queue occupancy remains the primary objective.
 * Another beautiful side effect of this deferred submission is that almost all queue submission 
 * driver calls get moved to a background thread, which is especically nice because 
 * queue submissions primarily transfer command buffers to a device, which is a 
 * inherintantly blocking operation.
 *
 * \paragraph queue_synchronization Queue Synchronization:
 * Building ontop of the already existing timeline semaphores per queue, we decided to use 
 * timeline values (\ref strobe::rhi::Timepoint "Timepoint") as the primary synchronization object.
 * A queue submission returns a \ref strobe::rhi::Timepoint "Timepoint", which is conceptually placed after the queue submission
 * within the timeline. We can use this Timepoint with queues to order submissions (similar to 
 * binary semaphores, but more truthful to the underlying execution model) or wait on the 
 * host to wait for completion. 
 *
 * \note Timepoints are quite a powerful abstraction, but might initally feel to granular. 
 * A important propery to keep in mind is that device queues are ordered, so expressing 
 * more fine grained synchronization often doesn't improve performance. 
 * 
 * \paragraph async_transfer Asynchronous Transfer:
 * Yet again building on timelines, we use a similar backpressure, batching 
 * approach with asynchronous transfers. The \ref strobe::rhi::Device "Device" offers 
 * asynchronous upload and copy functions, internally those are batched together into 
 * command buffers and submitted to a internal asynchronous transfer queue. 
 * A asynchronous \ref strobe::rhi::Device "Device" operation returns a \ref strobe::rhi::Timepoint "Timepoint", which 
 * can we used with queues to order submissions after the asynchronous operation completes 
 * or waited on by the host. 
 * 
 * \paragraph cmd_recording Command Recording:
 *
 * \paragraph descriptor_model Descriptor Model:
 *
 * \paragraph swapchain Swapchain recreation:
 *
 *
 * \subsection rhi_resources Objects:
 * - \ref strobe::rhi::Blas -- \copybrief strobe::rhi::Blas
 * - \ref strobe::rhi::Buffer -- \copybrief strobe::rhi::Buffer
 * - \ref strobe::rhi::CommandBuffer -- \copybrief strobe::rhi::CommandBuffer
 * - \ref strobe::rhi::CommandPool -- \copybrief strobe::rhi::CommandPool
 * - \ref strobe::rhi::ComputeShader -- \copybrief strobe::rhi::ComputeShader
 * - \ref strobe::rhi::Device -- \copybrief strobe::rhi::Device
 * - \ref strobe::rhi::FragmentShader -- \copybrief strobe::rhi::FragmentShader
 * - \ref strobe::rhi::Image — \copybrief strobe::rhi::Image
 * - \ref strobe::rhi::ImageView — \copybrief strobe::rhi::ImageView
 * - \ref strobe::rhi::Queue — \copybrief strobe::rhi::Queue
 * - \ref strobe::rhi::ResourceDescriptor — \copybrief strobe::rhi::ResourceDescriptor
 * - \ref strobe::rhi::ResourceDescriptorArray — \copybrief strobe::rhi::ResourceDescriptorArray
 * - \ref strobe::rhi::Swapchain — \copybrief strobe::rhi::Swapchain
 * - \ref strobe::rhi::SwapchainImage — \copybrief strobe::rhi::SwapchainImage
 * - \ref strobe::rhi::Timepoint — \copybrief strobe::rhi::Timepoint
 * - \ref strobe::rhi::Tlas — \copybrief strobe::rhi::Tlas
 * - \ref strobe::rhi::VertexShader — \copybrief strobe::rhi::VertexShader
 *
 * \subsection rhi_types Additional Types:
 * - \ref strobe::rhi::Aabb -- \copybrief strobe::rhi::Aabb
 * - \ref strobe::rhi::AabbGeometryData -- \copybrief strobe::rhi::AabbGeometryData
 * - \ref strobe::rhi::AabbGeometrySizeInfo -- \copybrief strobe::rhi::AabbGeometrySizeInfo
 * - \ref strobe::rhi::Access -- \copybrief strobe::rhi::Access
 * - \ref strobe::rhi::AccessScope -- \copybrief strobe::rhi::AccessScope
 * - \ref strobe::rhi::Attachment -- \copybrief strobe::rhi::Attachment
 * - \ref strobe::rhi::AttachmentLoadOp -- \copybrief strobe::rhi::AttachmentLoadOp
 * - \ref strobe::rhi::AttachmentStoreOp -- \copybrief strobe::rhi::AttachmentStoreOp
 * - \ref strobe::rhi::BlasInfo -- \copybrief strobe::rhi::BlasInfo
 * - \ref strobe::rhi::BlendEquation -- \copybrief strobe::rhi::BlendEquation
 * - \ref strobe::rhi::BlendFactor -- \copybrief strobe::rhi::BlendFactor
 * - \ref strobe::rhi::BlendOp -- \copybrief strobe::rhi::BlendOp
 * - \ref strobe::rhi::BufferInfo -- \copybrief strobe::rhi::BufferInfo
 * - \ref strobe::rhi::BufferOffset -- \copybrief strobe::rhi::BufferOffset
 * - \ref strobe::rhi::BufferRange -- \copybrief strobe::rhi::BufferRange
 * - \ref strobe::rhi::BufferUsage -- \copybrief strobe::rhi::BufferUsage
 * - \ref strobe::rhi::BuildFlags -- \copybrief strobe::rhi::BuildFlags
 * - \ref strobe::rhi::ClearDepthStencil -- \copybrief strobe::rhi::ClearDepthStencil
 * - \ref strobe::rhi::ClearValue -- \copybrief strobe::rhi::ClearValue
 * - \ref strobe::rhi::ColorComponent -- \copybrief strobe::rhi::ColorComponent
 * - \ref strobe::rhi::CommandBufferFlags -- \copybrief strobe::rhi::CommandBufferFlags
 * - \ref strobe::rhi::CompareOp -- \copybrief strobe::rhi::CompareOp
 * - \ref strobe::rhi::ComputeShaderInfo -- \copybrief strobe::rhi::ComputeShaderInfo
 * - \ref strobe::rhi::CullMode -- \copybrief strobe::rhi::CullMode
 * - \ref strobe::rhi::DescriptorType -- \copybrief strobe::rhi::DescriptorType
 * - \ref strobe::rhi::DeviceInfo -- \copybrief strobe::rhi::DeviceInfo
 * - \ref strobe::rhi::FenceInfo -- \copybrief strobe::rhi::FenceInfo
 * - \ref strobe::rhi::Format -- \copybrief strobe::rhi::Format
 * - \ref strobe::rhi::FragmentShaderInfo -- \copybrief strobe::rhi::FragmentShaderInfo
 * - \ref strobe::rhi::FrontFace -- \copybrief strobe::rhi::FrontFace
 * - \ref strobe::rhi::GeometryFlags -- \copybrief strobe::rhi::GeometryFlags
 * - \ref strobe::rhi::GeometryType -- \copybrief strobe::rhi::GeometryType
 * - \ref strobe::rhi::ImageAspect -- \copybrief strobe::rhi::ImageAspect
 * - \ref strobe::rhi::ImageFlags -- \copybrief strobe::rhi::ImageFlags
 * - \ref strobe::rhi::ImageInfo -- \copybrief strobe::rhi::ImageInfo
 * - \ref strobe::rhi::ImageLayout -- \copybrief strobe::rhi::ImageLayout
 * - \ref strobe::rhi::ImageSubresourceRange -- \copybrief strobe::rhi::ImageSubresourceRange
 * - \ref strobe::rhi::ImageType -- \copybrief strobe::rhi::ImageType
 * - \ref strobe::rhi::ImageUsage -- \copybrief strobe::rhi::ImageUsage
 * - \ref strobe::rhi::ImageViewInfo -- \copybrief strobe::rhi::ImageViewInfo
 * - \ref strobe::rhi::ImageViewType -- \copybrief strobe::rhi::ImageViewType
 * - \ref strobe::rhi::IndexType -- \copybrief strobe::rhi::IndexType
 * - \ref strobe::rhi::LogicOp -- \copybrief strobe::rhi::LogicOp
 * - \ref strobe::rhi::MemoryBarrier -- \copybrief strobe::rhi::MemoryBarrier
 * - \ref strobe::rhi::MemoryLifetime -- \copybrief strobe::rhi::MemoryLifetime
 * - \ref strobe::rhi::MemoryUsage -- \copybrief strobe::rhi::MemoryUsage
 * - \ref strobe::rhi::PipelineStage -- \copybrief strobe::rhi::PipelineStage
 * - \ref strobe::rhi::PolygonMode -- \copybrief strobe::rhi::PolygonMode
 * - \ref strobe::rhi::PrimitiveTopology -- \copybrief strobe::rhi::PrimitiveTopology
 * - \ref strobe::rhi::PushConstantRange -- \copybrief strobe::rhi::PushConstantRange
 * - \ref strobe::rhi::QueueFlags -- \copybrief strobe::rhi::QueueFlags
 * - \ref strobe::rhi::Rect -- \copybrief strobe::rhi::Rect
 * - \ref strobe::rhi::RenderingInfo -- \copybrief strobe::rhi::RenderingInfo
 * - \ref strobe::rhi::ResolveMode -- \copybrief strobe::rhi::ResolveMode
 * - \ref strobe::rhi::ResourceDescriptorInfo -- \copybrief strobe::rhi::ResourceDescriptorInfo
 * - \ref strobe::rhi::SampleCount -- \copybrief strobe::rhi::SampleCount
 * - \ref strobe::rhi::ShaderStage -- \copybrief strobe::rhi::ShaderStage
 * - \ref strobe::rhi::StencilFace -- \copybrief strobe::rhi::StencilFace
 * - \ref strobe::rhi::StencilOp -- \copybrief strobe::rhi::StencilOp
 * - \ref strobe::rhi::SwapchainInfo -- \copybrief strobe::rhi::SwapchainInfo
 * - \ref strobe::rhi::TlasInfo -- \copybrief strobe::rhi::TlasInfo
 * - \ref strobe::rhi::TlasInstance -- \copybrief strobe::rhi::TlasInstance
 * - \ref strobe::rhi::TriangleGeometryData -- \copybrief strobe::rhi::TriangleGeometryData
 * - \ref strobe::rhi::TriangleGeometrySizeInfo -- \copybrief strobe::rhi::TriangleGeometrySizeInfo
 * - \ref strobe::rhi::VertexAttribute -- \copybrief strobe::rhi::VertexAttribute
 * - \ref strobe::rhi::VertexBinding -- \copybrief strobe::rhi::VertexBinding
 * - \ref strobe::rhi::VertexInputRate -- \copybrief strobe::rhi::VertexInputRate
 * - \ref strobe::rhi::VertexShaderInfo -- \copybrief strobe::rhi::VertexShaderInfo
 * - \ref strobe::rhi::Viewport -- \copybrief strobe::rhi::Viewport
 */
namespace strobe::rhi {

Device create_device(const DeviceInfo &info);

}
