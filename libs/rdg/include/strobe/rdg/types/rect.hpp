#pragma once

#include "strobe/flow/flow.hpp"
#include "strobe/flow/vec.hpp"
#include "strobe/rhi/types/rect.hpp"

namespace strobe::rdg {

class Rect {
public:
  Rect(rhi::Rect value) noexcept : m_runtime(flow::resource<rhi::Rect>()) {
    flow::construct(m_runtime, [value](rhi::Rect *ptr) {
      ZoneScopedN("Rect::Rect");
      std::construct_at(ptr, value);
    });
  }

  flow::ivec2 offset() const noexcept {
    return flow::ivec2{flow::project<strobe::ivec2>(
        m_runtime, "offset", flow::ProjectionKind::disjoint,
        [](rhi::Rect &rect) noexcept -> strobe::ivec2 * {
          ZoneScopedN("Rect::offset");
          return &rect.offset;
        })};
  }

  flow::uvec2 extent() const noexcept {
    return flow::uvec2{flow::project<strobe::uvec2>(
        m_runtime, "extent", flow::ProjectionKind::disjoint,
        [](rhi::Rect &rect) noexcept -> strobe::uvec2 * {
          ZoneScopedN("Rect::extent");
          return &rect.extent;
        })};
  }

  flow::Resource<rhi::Rect> runtime() const noexcept { return m_runtime; }

private:
  flow::Resource<rhi::Rect> m_runtime;
};

} // namespace strobe::rdg
