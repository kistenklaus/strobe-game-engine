#pragma once

#include "strobe/core/containers/small_vector.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

namespace strobe::ecs {

using component_id = uint32_t;

template <typename T>
concept component = std::is_object_v<std::remove_cvref_t<T>> &&
                    !std::is_const_v<std::remove_cvref_t<T>> &&
                    !std::is_volatile_v<std::remove_cvref_t<T>> &&
                    std::is_move_constructible_v<std::remove_cvref_t<T>>;

struct ComponentTypeInfo {
  uint32_t size = 0;
  uint32_t alignment = 0;

  void (*move_construct)(void *dst, void *src) = nullptr;
  void (*destroy)(void *ptr) = nullptr;
};

namespace component_types::detail {

inline std::atomic<component_id> component_count{0};
inline std::mutex mutex{};
inline SmallVector<ComponentTypeInfo, 256, strobe::Mallocator> type_infos{};

inline component_id next_id() {
  return component_count.fetch_add(1, std::memory_order_relaxed);
}

template <component T> ComponentTypeInfo make_type_info() {
  using U = std::remove_cvref_t<T>;

  return ComponentTypeInfo{
      .size = static_cast<uint32_t>(sizeof(U)),
      .alignment = static_cast<uint32_t>(alignof(U)),

      .move_construct =
          [](void *dst, void *src) {
            std::construct_at(static_cast<U *>(dst),
                              std::move(*static_cast<U *>(src)));
          },

      .destroy =
          std::is_trivially_destructible_v<U>
              ? nullptr
              : [](void *ptr) { std::destroy_at(static_cast<U *>(ptr)); },
  };
}

template <component T> component_id static_component_id() {
  using U = std::remove_cvref_t<T>;

  static const component_id cid = [] {
    const component_id cid = next_id();

    std::lock_guard lock{mutex};

    if (type_infos.size() <= cid) {
      type_infos.resize(std::max(type_infos.size() * 3 / 2 + 1,
                                 static_cast<size_t>(cid) + 1));
    }

    assert(cid < type_infos.size());
    type_infos[cid] = make_type_info<U>();

    return cid;
  }();

  return cid;
}

} // namespace component_types::detail

template <component T> component_id component_type_id() {
  using U = std::remove_cvref_t<T>;
  return component_types::detail::static_component_id<U>();
}

inline const ComponentTypeInfo &component_type_info(component_id id) {
  using namespace component_types::detail;

  assert(id < component_count.load(std::memory_order_acquire));
  assert(id < type_infos.size());
  assert(type_infos[id].size != 0);

  return type_infos[id];
}

inline component_id component_type_count() {
  return component_types::detail::component_count.load(
      std::memory_order_acquire);
}

} // namespace strobe::ecs
