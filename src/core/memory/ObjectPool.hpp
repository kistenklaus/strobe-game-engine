#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>

#include "core/memory/FreelistPool.hpp"

namespace strobe::memory {

template <typename Object,
          size_t CHUNK_SIZE = std::max((size_t)1, 4096 / sizeof(Object) * 8),
          typename ChunkAllocator = std::allocator<
              internal::LinkedPoolBucketChunk<sizeof(Object), CHUNK_SIZE>>>
class ObjectPool : FreelistPool<sizeof(Object), CHUNK_SIZE, ChunkAllocator> {
  using Super = FreelistPool<sizeof(Object), CHUNK_SIZE, ChunkAllocator>;

 public:
  [[nodiscard]] Object *allocate() {
    return reinterpret_cast<Object *>(Super::allocate());
  }

  template <typename... Args>
  [[nodiscard]] Object* construct(Args &&...args) {
    Object *object = allocate();
    new (object) Object(std::forward<Args>(args)...);
    return object;
  }

  void deallocate(Object *p) { Super::deallocate(p); }

  void destruct(Object* p) {
    p->~Object();
    deallocate(p);
  }


};

}  // namespace strobe::memory
