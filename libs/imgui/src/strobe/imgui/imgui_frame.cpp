#include "strobe/imgui/imgui_frame.hpp"

namespace strobe::imgui {

void Frame::ensure_vertex_capacity(rhi::Device &device,
                                   uint64_t capacity) noexcept {
  if (m_vertex_capacity < capacity) {
    m_vertex_capacity = std::max(m_vertex_capacity * 2, capacity);
    free(host_vertex_stage);
    host_vertex_stage = malloc(m_vertex_capacity);
    assert(host_vertex_stage);
    vertex_buffer = device.create_buffer({
        .size = m_vertex_capacity,
        .bufferUsage =
            rhi::BufferUsage::vertex | rhi::BufferUsage::transfer_dst,
        .memoryUsage = rhi::MemoryUsage::automatic,
    });
    assert(vertex_buffer);
  }
}
void Frame::ensure_index_capacity(rhi::Device &device,
                                     uint64_t capacity) noexcept {
  if (m_index_capacity < capacity) {
    m_index_capacity = std::max(m_index_capacity * 2, capacity);
    free(host_index_stage);
    host_index_stage = malloc(m_index_capacity);
    assert(host_index_stage);
    index_buffer = device.create_buffer({
        .size = m_index_capacity,
        .bufferUsage = rhi::BufferUsage::index | rhi::BufferUsage::transfer_dst,
        .memoryUsage = rhi::MemoryUsage::automatic,
    });
    assert(index_buffer);
  }
}

} // namespace strobe::imgui
