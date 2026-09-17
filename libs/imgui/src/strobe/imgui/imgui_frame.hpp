#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/device.hpp"

namespace strobe::imgui {

struct Frame {
  rhi::Buffer vertex_buffer{};
  rhi::Buffer index_buffer{};
  void *host_vertex_stage = nullptr;
  void *host_index_stage = nullptr;

  void ensure_vertex_capacity(rhi::Device &device, uint64_t capacity) noexcept;

  void ensure_index_capacity(rhi::Device &device,
                                uint64_t capacity) noexcept;

  Frame() noexcept {}
  Frame(const Frame &) = delete;
  Frame(Frame &&) = delete;
  Frame &operator=(const Frame &) = delete;
  Frame &operator=(Frame &&) = delete;

  ~Frame() noexcept {
    free(host_vertex_stage);
    free(host_index_stage);
  }

private:
  uint64_t m_vertex_capacity{0};
  uint64_t m_index_capacity{0};
};

} // namespace strobe::imgui
