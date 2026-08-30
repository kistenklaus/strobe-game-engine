#pragma once

#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/sync/fence_node.hpp"
#include "strobe/rhi/sync/fence_pool.hpp"

namespace strobe::rhi {

struct FenceImpl {

  explicit FenceImpl(FencePool pool, FenceNode *node) noexcept
      : pool(std::move(pool)), node(node) {}
  FenceImpl(const FenceImpl &) = delete;
  FenceImpl(FenceImpl &&) = delete;
  FenceImpl &operator=(const FenceImpl &) = delete;
  FenceImpl &operator=(FenceImpl &&) = delete;
  ~FenceImpl() noexcept;

  FencePool pool;
  FenceNode *node;
};

} // namespace strobe::rhi
