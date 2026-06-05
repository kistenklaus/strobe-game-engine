#pragma once

#include "strobe/ecs/cmd/drain_cmds.hpp"
#include "strobe/ecs/resource/resource_registry.hpp"

// include complete type definition.
#include "strobe/ecs/universe.hpp"

namespace strobe::ecs {

inline ResourceRegistry::ResourceRegistry(Universe *universe,
                                          const allocator &alloc)
    : m_universe(universe), m_resources{alloc}, m_resourceAlloc(alloc),
      m_blockPool(alloc), m_cmdPool(alloc),
      m_cmdbuf{universe, &universe->sr_domain} {
  m_resources.resize(64, nullptr);
}

inline ResourceRegistry::~ResourceRegistry() noexcept {
  assert(m_cmdbuf.peek() == null_cmd_index);
  for (uint32_t i = 0; i < m_resources.size(); ++i) {
    auto *header = m_resources[i];
    if (header != nullptr) {
      location loc = header->require_location(&m_universe->scheduler);
      m_universe->scheduler.submit(
          op_scope(acq_rel(loc), acq_rel(get_registry_location())),
          [rreg = this, id = resource_id{i}]() noexcept {
            auto *header = rreg->m_resources[id.m_index];
            if (header->ptr != nullptr) {
              rreg->cmd_destroy(id);
            }
          });
    }
  }
  drain_cmds(&m_universe->scheduler, &m_universe->sr_domain,
             op_scope(acq_rel(m_universe->sr_location)), &m_cmdbuf);

  for (uint32_t i = 0; i < m_resources.size(); ++i) {
    resource_header *header = m_resources[i];
    if (header == nullptr) {
      continue;
    }
    if (header->has_location()) {
      m_universe->scheduler.fence(op_scope(acq_rel(header->get_location())));

      m_universe->scheduler.free(header->get_location());

      if (header->has_lifetime()) {
        const lifetime_id lifetime = header->get_lifetime();
        if (m_universe->lreg.is_constructed(lifetime)) {
          m_universe->lreg.destruct(m_universe, lifetime);
        }
        m_universe->lreg.free(lifetime);
      }
      std::destroy_at(header);
    }
  }
}

template <typename R>
void ResourceRegistry::resource_block<R>::exit(Universe *universe) noexcept {
  universe->scheduler.submit(
      op_scope(acq_rel(this->get_location())), [header = this]() noexcept {
        void *ptr = header->ptr;
        assert(ptr != nullptr);

        auto *resource = std::launder(reinterpret_cast<resource_type *>(ptr));

        std::destroy_at<resource_type>(resource);
      });
}

inline void
ResourceRegistry::resource_cmd::operator()(Universe *universe) noexcept {
  assert(m_header != nullptr);
  const location loc = m_header->require_location(&universe->scheduler);
  const lifetime_id lifetime = m_header->require_lifetime(&universe->lreg);
  switch (m_tag) {
  case RESOURCE_CMD_CREATE_TAG: {
    assert(m_ptr != nullptr);
    universe->scheduler.submit(
        op_scope(acq_rel(loc)),
        [header = m_header, ptr = m_ptr]() noexcept { header->ptr = ptr; });
    universe->lreg.construct(universe, lifetime);
    break;
  }
  case RESOURCE_CMD_DESTROY_TAG:
    // destruct will call resource_block::exit
    universe->lreg.destruct(universe, lifetime);
    universe->scheduler.submit(
        op_scope(acq_rel(universe->sr_location), acq_rel(loc)),
        [header = m_header]() noexcept {
          void *ptr = header->ptr;
          assert(ptr != nullptr);
          // TODO make SPSC lockfree
          header->push_storage(ptr);
          header->ptr = nullptr;
        });
    break;
  }
}

inline ResourceRegistry::resource_cmdbuf::~resource_cmdbuf() noexcept {
  assert(peek() == null_cmd_index);
  if (m_head != &m_stub) {
    std::destroy_at(m_head);
    m_universe->rreg.m_cmdPool.deallocate(m_head);
  }
}

template <typename... Args>
void ResourceRegistry::resource_cmdbuf::emplace(Args &&...args) noexcept {
  cmd_index index = m_domain->next();
  assert(index != null_cmd_index);
  auto *cmd =
      static_cast<resource_cmd *>(m_universe->rreg.m_cmdPool.allocate());
  std::construct_at(cmd, index, std::forward<Args>(args)...);
  cmd->next.store(nullptr, std::memory_order_relaxed);
  m_tail->next.store(cmd, std::memory_order_release);
  m_tail = cmd;
}

inline void ResourceRegistry::resource_cmdbuf::step() noexcept {
  resource_cmd *cmd = m_head->next.load(std::memory_order_acquire);
  assert(cmd != nullptr);
  (*cmd)(m_universe);
  resource_cmd *old = m_head;
  m_head = cmd;
  if (old != &m_stub) {
    std::destroy_at(old);
    m_universe->rreg.m_cmdPool.deallocate(old);
  }
}

inline location
ResourceRegistry::get_resource_location(resource_id id) noexcept {
  assert(id != null_resource_id);
  assert(id.m_index < m_resources.size());
  resource_header *header = m_resources[id.m_index];
  return header->require_location(&m_universe->scheduler); // lazy alloc
}

inline lifetime_id
ResourceRegistry::get_resource_lifetime(resource_id id) noexcept {
  assert(id != null_resource_id);
  assert(id.m_index < m_resources.size());
  resource_header *header = m_resources[id.m_index];
  return header->require_lifetime(&m_universe->lreg); // lazy alloc
}

inline location ResourceRegistry::get_registry_location() const noexcept {
  return m_universe->sr_location;
}

} // namespace strobe::ecs
