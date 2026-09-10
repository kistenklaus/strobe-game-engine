#pragma once

#include "strobe/rhi/context/context.hpp"
#include "strobe/rhi/memory/memory_allocation.hpp"
#include "strobe/rhi/memory/memory_requirements.hpp"
#include "strobe/rhi/objects/object.hpp"
#include "strobe/rhi/types/memory_lifetime.hpp"

namespace strobe::rhi {

class MemoryPool : public Object<MemoryPool> {
  friend class Object<MemoryPool>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  void commit();

  bool memory_overlaps(const MemoryAllocation &lhs,
                       const MemoryAllocation &rhs) const noexcept;

  MemoryAllocation allocate_memory(const MemoryRequirements &requirements,
                                   const MemoryLifetime &lifetime) const;

  const Context &context() const noexcept;
};

} // namespace strobe::rhi
