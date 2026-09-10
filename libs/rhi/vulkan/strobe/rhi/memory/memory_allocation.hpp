#pragma once

#include "strobe/rhi/memory/memory_binding.hpp"
#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

struct MemoryAllocation : public Object<MemoryAllocation> {
  friend class Object<MemoryAllocation>;
  static void pin(void *) noexcept;
  static void unpin(void *) noexcept;

public:
  using Object::Object;

  explicit operator bool() const noexcept {
    return m_handle != nullptr;
  }

  void *map() const ;
  void flush() const;
  void invalidate() const;
  bool commit() const;

  const MemoryBinding& binding() const;
};

} // namespace strobe::rhi
