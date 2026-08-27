#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/type_traits/unroll.hpp"
#include "strobe/rhi/cmd/command_buffer_state_alloctor.hpp"
#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/compute_shader.hpp"
#include "strobe/rhi/objects/fragment_shader.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/vertex_shader.hpp"

namespace strobe::rhi {

struct CommandBufferState {
  // NOTE: A custom vector implementation might make sense
  // if we see this actually showing up on the profiles.
  // A custom vector implementation could,
  // deduplicate on growth and use a memcpy instead of
  // invoking the move constructor (i know this is UB, but
  // it will probably still be fine in practice).

  static constexpr size_t SEARCH_WINDOW_SIZE = 16;

  using allocator = cmd_buf_state_allocator_ref;
  CommandBufferState(const allocator &alloc) noexcept
      : m_boundVertexShaders(alloc), m_boundFragmentShaders(alloc),
        m_boundComputeShaders(alloc), m_boundBuffers(alloc),
        m_boundImages(alloc), m_boundBlas(alloc) {}

  void retain(const VertexShader &obj) {
    auto &bindings = m_boundVertexShaders;
    if (bindings.size() > SEARCH_WINDOW_SIZE) {
      auto it = bindings.end();
      STROBE_UNROLL(SEARCH_WINDOW_SIZE)
      for (size_t i = 0; i < SEARCH_WINDOW_SIZE; ++i) {
        --it;
        if (obj == *it) {
          return;
        }
      }
    } else {
      for (const auto &bound : bindings) {
        if (obj == bound) {
          return;
        }
      }
    }
    bindings.push_back(obj);
  }

  void retain(const FragmentShader &obj) {
    auto &bindings = m_boundFragmentShaders;
    if (bindings.size() > SEARCH_WINDOW_SIZE) {
      auto it = bindings.end();
      STROBE_UNROLL(SEARCH_WINDOW_SIZE)
      for (size_t i = 0; i < SEARCH_WINDOW_SIZE; ++i) {
        --it;
        if (obj == *it) {
          return;
        }
      }
    } else {
      for (const auto &bound : bindings) {
        if (obj == bound) {
          return;
        }
      }
    }
    bindings.push_back(obj);
  }

  void retain(const ComputeShader &obj) {
    auto &bindings = m_boundComputeShaders;
    if (bindings.size() > SEARCH_WINDOW_SIZE) {
      auto it = bindings.end();
      STROBE_UNROLL(SEARCH_WINDOW_SIZE)
      for (size_t i = 0; i < SEARCH_WINDOW_SIZE; ++i) {
        --it;
        if (obj == *it) {
          return;
        }
      }
    } else {
      for (const auto &bound : bindings) {
        if (obj == bound) {
          return;
        }
      }
    }
    bindings.push_back(obj);
  }

  void retain(const Buffer &obj) {
    auto &bindings = m_boundBuffers;
    if (bindings.size() > SEARCH_WINDOW_SIZE) {
      auto it = bindings.end();
      STROBE_UNROLL(SEARCH_WINDOW_SIZE)
      for (size_t i = 0; i < SEARCH_WINDOW_SIZE; ++i) {
        --it;
        if (obj == *it) {
          return;
        }
      }
    } else {
      for (const auto &bound : bindings) {
        if (obj == bound) {
          return;
        }
      }
    }
    bindings.push_back(obj);
  }

  void retain(const Image &obj) {
    auto &bindings = m_boundImages;
    if (bindings.size() > SEARCH_WINDOW_SIZE) {
      auto it = bindings.end();
      STROBE_UNROLL(SEARCH_WINDOW_SIZE)
      for (size_t i = 0; i < SEARCH_WINDOW_SIZE; ++i) {
        --it;
        if (obj == *it) {
          return;
        }
      }
    } else {
      for (const auto &bound : bindings) {
        if (obj == bound) {
          return;
        }
      }
    }
    bindings.push_back(obj);
  }

  void retain(const Blas &obj) {
    auto &bindings = m_boundBlas;
    if (bindings.size() > SEARCH_WINDOW_SIZE) {
      auto it = bindings.end();
      STROBE_UNROLL(SEARCH_WINDOW_SIZE)
      for (size_t i = 0; i < SEARCH_WINDOW_SIZE; ++i) {
        --it;
        if (obj == *it) {
          return;
        }
      }
    } else {
      for (const auto &bound : bindings) {
        if (obj == bound) {
          return;
        }
      }
    }
    bindings.push_back(obj);
  }

private:
  Vector<VertexShader, allocator> m_boundVertexShaders;
  Vector<FragmentShader, allocator> m_boundFragmentShaders;
  Vector<ComputeShader, allocator> m_boundComputeShaders;
  Vector<Buffer, allocator> m_boundBuffers;
  Vector<Image, allocator> m_boundImages;
  Vector<Blas, allocator> m_boundBlas;
};

} // namespace strobe::rhi
