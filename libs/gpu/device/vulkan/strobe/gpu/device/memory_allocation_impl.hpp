#pragma once

#include "strobe/gpu/device/memory_binding.hpp"
#include "strobe/gpu/device/memory_pool.hpp"
#include <sys/wait.h>

namespace strobe::gpu {

struct MemoryAllocationImpl {
  MemoryAllocationImpl(MemoryPool pool, MemoryBinding binding,
                       void *internals) noexcept
      : pool(std::move(pool)), binding(binding), internals(internals) {}

  ~MemoryAllocationImpl() noexcept;

  void *map();
  void flush();
  void invalidate();

public:
  MemoryPool pool;
  MemoryBinding binding;
  void *internals = nullptr;
  void *mapped = nullptr;
};

} // namespace strobe::gpu
