#pragma once
#include "types/inttypes.hpp"

namespace sge::vulkan {

static u32 INVALID_INDEX_HANDLE = -1;

struct pipeline_layout {
  friend class VRendererBackend;

 public:
  pipeline_layout() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const pipeline_layout& layout) const {
    return layout.m_index == m_index;
  }
  bool operator!=(const pipeline_layout& layout) const {
    return layout.m_index != m_index;
  }
  bool operator<(const pipeline_layout& layout) const {
    return layout.m_index < m_index;
  }

 private:
  explicit pipeline_layout(u32 index) : m_index(index) {}
  u32 m_index;
};
struct imageview {
  friend class VRendererBackend;

 public:
  imageview() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const imageview& imageview) const {
    return imageview.m_index == m_index;
  }
  bool operator!=(const imageview& imageview) const {
    return imageview.m_index != m_index;
  }
  bool operator<(const imageview& imageview) const {
    return imageview.m_index < m_index;
  }

 private:
  explicit imageview(u32 index) : m_index(index) {}
  u32 m_index = INVALID_INDEX_HANDLE;
};

struct command_pool {
  friend class VRendererBackend;

 public:
  command_pool() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const command_pool& commandPool) const {
    return commandPool.m_index == m_index;
  }
  bool operator!=(const command_pool& commandPool) const {
    return commandPool.m_index != m_index;
  }
  bool operator<(const command_pool& commandPool) const {
    return commandPool.m_index < m_index;
  }

 private:
  explicit command_pool(u32 index) : m_index(index) {}
  u32 m_index;
};

struct command_buffer {
  friend class VRendererBackend;

 public:
  command_buffer() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const command_buffer& buffer) const {
    return buffer.m_index == m_index;
  }
  bool operator!=(const command_buffer& buffer) const {
    return buffer.m_index != m_index;
  }
  bool operator<(const command_buffer& buffer) const {
    return buffer.m_index < m_index;
  }

 private:
  explicit command_buffer(u32 index) : m_index(index) {}
  u32 m_index;
};

struct fence {
  friend class VRendererBackend;

 public:
  fence() : m_index(INVALID_INDEX_HANDLE) {}

  bool operator==(const fence& fence) const { return fence.m_index == m_index; }
  bool operator!=(const fence& fence) const { return fence.m_index != m_index; }
  bool operator<(const fence& fence) const { return fence.m_index < m_index; }

 private:
  explicit fence(u32 index) : m_index(index) {}
  u32 m_index;
};

struct queue {
  friend class VRendererBackend;

 public:
  queue() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const queue& queue) const { return queue.m_index == m_index; }
  bool operator!=(const queue& queue) const { return queue.m_index != m_index; }
  bool operator<(const queue& queue) const { return queue.m_index < m_index; }

 private:
  explicit queue(u32 index) : m_index(index) {}
  u32 m_index;
};

struct framebuffer {
  friend class VRendererBackend;

 public:
  framebuffer() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const framebuffer& framebuffer) const {
    return framebuffer.m_index == m_index;
  }
  bool operator!=(const framebuffer& framebuffer) const {
    return framebuffer.m_index != m_index;
  }
  bool operator<(const framebuffer& framebuffer) const {
    return framebuffer.m_index < m_index;
  }

 private:
  explicit framebuffer(u32 index) : m_index(index) {}
  u32 m_index;
};

struct pipeline {
  friend class VRendererBackend;

 public:
  pipeline() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const pipeline& pipeline) const {
    return pipeline.m_index == m_index;
  }
  bool operator!=(const pipeline& pipeline) const {
    return pipeline.m_index != m_index;
  }
  bool operator<(const pipeline& pipeline) const {
    return pipeline.m_index < m_index;
  }

 private:
  explicit pipeline(u32 index) : m_index(index) {}
  u32 m_index;
};

struct renderpass {
  friend class VRendererBackend;

 public:
  renderpass() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const renderpass& renderpass) const {
    return renderpass.m_index == m_index;
  }
  bool operator!=(const renderpass& renderpass) const {
    return renderpass.m_index != m_index;
  }
  bool operator<(const renderpass& renderpass) const {
    return renderpass.m_index < m_index;
  }

 private:
  explicit renderpass(u32 index) : m_index(index) {}
  u32 m_index;
};

struct shader_module {
  friend class VRendererBackend;

 public:
  shader_module() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const shader_module& shader) const {
    return shader.m_index == m_index;
  }
  bool operator!=(const shader_module& shader) const {
    return shader.m_index != m_index;
  }
  bool operator<(const shader_module& shader) const {
    return shader.m_index < m_index;
  }

 private:
  explicit shader_module(u32 index) : m_index(index) {}
  u32 m_index;
};

struct semaphore {
  friend class VRendererBackend;

 public:
  semaphore() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const semaphore& semaphore) const {
    return semaphore.m_index == m_index;
  }
  bool operator!=(const semaphore& semaphore) const {
    return semaphore.m_index != m_index;
  }
  bool operator<(const semaphore& semaphore) const {
    return semaphore.m_index < m_index;
  }

 private:
  explicit semaphore(u32 index) : m_index(index) {}
  u32 m_index;
};

struct buffer {
  friend class VRendererBackend;

 public:
  buffer() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const buffer& buffer) const {
    return buffer.m_index == m_index;
  }
  bool operator!=(const buffer& buffer) const {
    return buffer.m_index != m_index;
  }
  bool operator<(const buffer& buffer) const {
    return buffer.m_index < m_index;
  }

 private:
  explicit buffer(u32 index) : m_index(index) {}
  u32 m_index;
};

typedef buffer vertex_buffer;
typedef buffer index_buffer;

struct uniform_buffer {
  buffer m_bufferHandle;
};

struct simple_handle {
  friend class VRendererBackend;

 public:
  simple_handle() : m_index(INVALID_INDEX_HANDLE) {}
  bool operator==(const simple_handle& descriptorSetLayout) const {
    return descriptorSetLayout.m_index == m_index;
  }
  bool operator!=(const simple_handle& descriptorSetLayout) const {
    return descriptorSetLayout.m_index != m_index;
  }
  bool operator<(const simple_handle& descriptorSetLayout) const {
    return descriptorSetLayout.m_index < m_index;
  }

 private:
  explicit simple_handle(u32 index) : m_index(index) {}
  u32 m_index;
};

typedef simple_handle descriptor_set_layout;
typedef simple_handle descriptor_pool;
typedef simple_handle descriptor_set;

}  // namespace sge::vulkan
