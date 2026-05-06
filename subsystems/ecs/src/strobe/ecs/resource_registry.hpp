#pragma once

#include "strobe/core/containers/vector_storage.hpp"
#include "strobe/core/memory/AllocatorTraits.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe::ecs {

using resource_id = uint32_t;
inline constexpr resource_id invalid_resource_id =
    std::numeric_limits<resource_id>::max();

namespace details {

inline resource_id next_resource_type_id() {
  static std::atomic<resource_id> id_acc = 0;
  return id_acc.fetch_add(1, std::memory_order_relaxed);
}

template <typename R>
  requires(!std::is_reference_v<R>)
inline resource_id resource_type_id() {
  static resource_id id = next_resource_type_id();
  return id;
}

}; // namespace details

template <Allocator Alloc> class ResourceRegistry {
private:
  using ATraits = AllocatorTraits<Alloc>;

public:
  ResourceRegistry(const Alloc &alloc) : m_alloc(alloc) {}
  ResourceRegistry(const ResourceRegistry &o) = delete;
  ResourceRegistry &operator=(const ResourceRegistry &o) = delete;
  ResourceRegistry(ResourceRegistry &&o) = delete;
  ResourceRegistry &operator=(ResourceRegistry &&o) = delete;

  ~ResourceRegistry() {
    assert(m_typeInfos.size() == m_resources.size());
    const uint32_t rsize = m_resources.size();
    for (resource_id id = 0; id < rsize; ++id) {
      if (m_resources[id] != nullptr) {
        void *ptr = m_resources[id];
        ResourceTypeInfo &info = m_typeInfos[id];
        info.destroy(ptr);
        ATraits::deallocate(m_alloc, ptr, static_cast<size_t>(info.size),
                            static_cast<size_t>(info.alignment));
      }
    }
  }

  template <typename R> bool createResource(R &&resource) {
    assert(m_typeInfos.size() == m_resources.size());
    using resource_type = std::remove_cvref_t<R>;

    resource_id id = details::resource_type_id<resource_type>();
    const uint32_t rsize = m_resources.size();
    if (rsize <= id) {
      size_t new_size = std::max(id + 1, rsize * 3 / 2 + 1);
      m_typeInfos.resize(m_alloc, new_size);
      m_resources.resize(m_alloc, new_size, nullptr);
    }

    bool unregistered = m_resources[id] == nullptr;
    if (!unregistered) {
      return false;
    }

    // derive type info.
    m_typeInfos[id] = ResourceTypeInfo{
        .size = sizeof(resource_type),
        .alignment = alignof(resource_type),
        .destroy =
            [](void *ptr) {
              std::destroy_at<resource_type>(static_cast<resource_type *>(ptr));
            },
    };
    void *ptr = ATraits::template allocate<resource_type>(m_alloc);
    try {
      std::construct_at(static_cast<resource_type *>(ptr),
                        std::forward<R>(resource));
    } catch (...) {
      ATraits::template deallocate<resource_type>(m_alloc, ptr);
      throw;
    }
    m_resources[id] = ptr;
    return unregistered;
  }

  template <typename R>
    requires(!std::is_reference_v<R>)
  bool destroyResource() noexcept {
    assert(m_typeInfos.size() == m_resources.size());
    using resource_type = std::remove_cvref_t<R>; // just for sanity.

    resource_id id = details::resource_type_id<resource_type>();
    const uint32_t rsize = m_resources.size();
    if (id >= rsize) {
      return false;
    }
    void *ptr = m_resources[id];
    if (ptr == nullptr) {
      return false;
    }
    ResourceTypeInfo &info = m_typeInfos[id];
    info.destroy(ptr);
    ATraits::deallocate(m_alloc, ptr, static_cast<size_t>(info.size),
                        static_cast<size_t>(info.alignment));
    m_resources[id] = nullptr;
    return true;
  }

  template <typename R>
    requires(!std::is_reference_v<R>)
  static resource_id get_resource_id() {
    using resource_type = std::remove_cvref_t<R>;
    const resource_id id = details::resource_type_id<resource_type>();
    return id;
  }

  bool exists(resource_id id) const {
    assert(m_resources.size() == m_typeInfos.size());
    const uint32_t rsize = m_resources.size();
    if (rsize <= id) {
      return false;
    }
    return m_resources[id] != nullptr;
  }

  void *get_resource_ptr(resource_id id) {
    assert(m_resources.size() == m_typeInfos.size());
    assert(id < m_resources.size());
    assert(m_resources[id] != nullptr);
    return m_resources[id];
  }

private:
  struct ResourceTypeInfo {
    uint32_t size = 0;
    uint32_t alignment = 0;
    void (*destroy)(void *ptr) = nullptr;
  };

  VectorStorage<void *> m_resources;           // hot
  VectorStorage<ResourceTypeInfo> m_typeInfos; // cold
  Alloc m_alloc;
};

} // namespace strobe::ecs
