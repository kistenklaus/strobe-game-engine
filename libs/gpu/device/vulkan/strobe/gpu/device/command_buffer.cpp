#include "strobe/gpu/device/command_buffer.hpp"
#include "strobe/gpu/device/command_buffer_impl.hpp"
#include "strobe/gpu/device/handle.hpp"
#include "strobe/gpu/vulkan/command_buffer.hpp"

namespace strobe::gpu {

CommandBuffer::CommandBuffer(const CommandBuffer &o) noexcept
    : m_handle(o.m_handle) {
  if (m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl>(m_handle);
  }
}

CommandBuffer::CommandBuffer(CommandBuffer &&o) noexcept
    : m_handle(std::exchange(o.m_handle, nullptr)) {}

CommandBuffer &CommandBuffer::operator=(const CommandBuffer &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<CommandBufferImpl>(o.m_handle);
  }
  unpin_void_handle<CommandBufferImpl>(m_handle);
  m_handle = o.m_handle;
  return *this;
}

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<CommandBufferImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

CommandBuffer::~CommandBuffer() noexcept {
  unpin_void_handle<CommandBufferImpl>(m_handle);
}

void CommandBuffer::begin() {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::begin_command_buffer(impl->cmd);
}

void CommandBuffer::end() {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::end_command_buffer(impl->cmd);
}

void CommandBuffer::reset() {
  auto *impl = void_handle_ptr<CommandBufferImpl>(m_handle);
  vulkan::reset_command_buffer(impl->cmd);
}

} // namespace strobe::gpu
