#include "strobe/ecs/resource/resource_registry.hpp"

namespace strobe::ecs {

uint32_t ResourceRegistry::next_resource_type_id() noexcept {
  static std::atomic<uint32_t> id_acc = 0;
  return id_acc.fetch_add(1, std::memory_order_relaxed);
}

} // namespace strobe::ecs
