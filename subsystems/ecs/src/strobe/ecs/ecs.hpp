#pragma once
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/registries.hpp"
#include "strobe/ecs/resource/resource_manager.hpp"
#include "strobe/ecs/system/system_manager.hpp"

namespace strobe {

class ECS {
  static constexpr uint32_t THREAD_COUNT = 8;

public:
  // ECS(const ecs::allocator &alloc = {})
  //     : m_alloc(alloc), m_rreg(&m_alloc), m_ereg(&m_alloc) {}

  ECS(const ECS &) = delete;
  ECS &operator=(const ECS &) = delete;
  ECS(ECS &&) = delete;
  ECS &operator=(ECS &&) = delete;

private:
  ecs::allocator m_alloc;
  ecs::Registries m_regs;
  ecs::SystemManager m_systemManager;
  ecs::ResourceManager m_resourceManager;
};

} // namespace strobe
