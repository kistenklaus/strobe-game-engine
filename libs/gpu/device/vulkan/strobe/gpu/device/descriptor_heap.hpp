#pragma once

namespace strobe::gpu {

class DescriptorHeap {
  friend class Device;
  friend class MemoryPool;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  DescriptorHeap() noexcept : m_handle(nullptr) {}
  DescriptorHeap(const DescriptorHeap &) = delete;
  DescriptorHeap(DescriptorHeap &&) = delete;
  DescriptorHeap& operator=(const DescriptorHeap &) = delete;
  DescriptorHeap& operator=(DescriptorHeap &&) = delete;
  ~DescriptorHeap() noexcept {}

private:
  explicit DescriptorHeap(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
