#pragma once
#include "strobe/core/type_traits/type_name.hpp"
#include "strobe/ecs/cmd/drain_cmds.hpp"
#include "strobe/ecs/lifetime/lifetime_registry.hpp"
#include "strobe/ecs/system/system_registry.hpp"
#include "strobe/ecs/system/system_traits.hpp"
#include "strobe/ecs/universe.hpp"
#include <algorithm>
#include <cassert>
#include <memory>

namespace strobe::ecs {

inline SystemRegistry::SystemRegistry(Universe *universe,
                                      const allocator &alloc) noexcept
    : m_universe(universe),
      m_sreg_lifetime(universe->lreg.alloc([](auto &) noexcept {})),
      m_headers(alloc), m_blockAlloc(alloc), m_cmdbuf(universe, alloc) {
  m_headers.resize(64, nullptr);
  universe->lreg.construct(universe, m_sreg_lifetime);
}

inline void
SystemRegistry::system_cmd::operator()(Universe *universe) noexcept {
  switch (m_tag) {
  case SYSTEM_CMD_CREATE_TAG: {
    ZoneScopedN("sreg::create_cmd");
    SystemRegistry *sreg = &universe->sreg;
    size_t ssize = sreg->m_headers.size();
    if (sreg->m_headers.size() <= m_id.m_index) {
      const size_t new_size = std::max<size_t>(
          static_cast<size_t>(m_id.m_index) + 1, ssize * 3 / 2 + 1);
      sreg->m_headers.resize(new_size, nullptr);
    }
    if (sreg->m_headers[m_id.m_index] == nullptr) {
      m_curry(universe);
    }
    system_header *header = sreg->m_headers[m_id.m_index];
    universe->lreg.construct(universe, header->get_ready_lifetime());
    break;
  }
  case SYSTEM_CMD_DESTROY_TAG: {
    ZoneScopedN("sreg::destroy_cmd");
    SystemRegistry *sreg = &universe->sreg;
    assert(m_id.m_index < sreg->m_headers.size());
    system_header *header = sreg->m_headers[m_id.m_index];
    assert(header != nullptr);
    if (universe->lreg.is_constructed(header->get_active_lifetime())) {
      universe->lreg.destruct(universe, header->get_active_lifetime());
    }
    universe->lreg.destruct(universe, header->get_ready_lifetime());
    break;
  }
  case SYSTEM_CMD_ENABLE_TAG: {
    ZoneScopedN("sreg::enable_cmd");
    SystemRegistry *sreg = &universe->sreg;
    assert(m_id.m_index < sreg->m_headers.size());
    system_header *header = sreg->m_headers[m_id.m_index];
    assert(header != nullptr);
    universe->lreg.construct(universe, header->get_active_lifetime());
    break;
  }
  case SYSTEM_CMD_DISABLE_TAG: {
    ZoneScopedN("sreg::disable_cmd");
    SystemRegistry *sreg = &universe->sreg;
    assert(m_id.m_index < sreg->m_headers.size());
    system_header *header = sreg->m_headers[m_id.m_index];
    assert(header != nullptr);
    universe->lreg.destruct(universe, header->get_active_lifetime());
    break;
  }
  }
}

inline SystemRegistry::system_cmdbuf::system_cmdbuf(
    Universe *universe, const allocator &alloc) noexcept
    : m_universe(universe), m_cmdPool(alloc), m_domain(&universe->sr_domain),
      m_stub{}, m_head(&m_stub), m_tail(&m_stub) {
  m_stub.next.store(nullptr, std::memory_order_relaxed);
}

inline void SystemRegistry::system_cmdbuf::step() noexcept {
  ZoneScopedN("sreg::cmdbuf::step");
  system_cmd *cmd = m_head->next.load(std::memory_order_acquire);
  assert(cmd != nullptr);
  (*cmd)(m_universe);
  system_cmd *old = m_head;
  m_head = cmd;
  if (old != &m_stub) {
    std::destroy_at(old);
    m_cmdPool.deallocate(old);
  }
}

template <typename S> system_id SystemRegistry::cmd_create() {
  static constexpr auto debug_name =
      fixed_string{"sreg::cmd_create<"} + type_name<S>() + fixed_string{">"};
  ZoneScopedN(debug_name.data());
  using system_type = std::remove_cvref_t<S>;
  const system_id id{system_type_id<system_type>()};
  m_cmdbuf.emplace(SYSTEM_CMD_CREATE_TAG, id, //
                   [](Universe *universe) noexcept -> void {
                     universe->sreg.require_system_header<system_type>();
                   });
  return id;
}

template <typename S>
system_header *SystemRegistry::require_system_header() noexcept {
  using system_type = std::remove_cvref_t<S>;
  using system_traits = system_traits<system_type>;
  const system_id id{system_type_id<system_type>()};
  const size_t ssize = m_headers.size();
  if (ssize <= id.m_index) {
    const size_t new_size = std::max<size_t>(
        static_cast<size_t>(id.m_index) + 1, ssize * 3 / 2 + 1);
    m_headers.resize(new_size, nullptr);
  }
  system_header *header = m_headers[id.m_index];

  if (header != nullptr) {
    return header;
  }

  static constexpr auto debug_name =
      fixed_string{"sreg::register_system<"} + type_name<S>() + fixed_string{">"};
  ZoneScopedN(debug_name.data());

  using system_block = system_block<system_type>;
  system_block *block = static_cast<system_block *>(
      m_blockAlloc.allocate(sizeof(system_block), alignof(system_block)));
  location loc = m_universe->scheduler.alloc();
  using Scope = LifetimeRegistry::DependencyScope;
  lifetime_id ready_lifetime =
      m_universe->lreg.alloc([universe = m_universe](Scope &scope) noexcept {
        using persistant =
            typename system_traits::ready_persistent_requirements;
        using entry = typename system_traits::ready_entry_only_requirements;
        for_each_type<persistant>([&]<typename object>() noexcept {
          lifetime_id dependency = get_object_lifetime<object>(universe);
          scope.require(dependency, lifetime_requirement::persistent);
        });
        for_each_type<entry>([&]<typename object>() noexcept {
          lifetime_id dependency = get_object_lifetime<object>(universe);
          scope.require(dependency, lifetime_requirement::entry);
        });
        scope.require(universe->sreg.m_sreg_lifetime,
                      lifetime_requirement::persistent);
      });
  lifetime_id active_lifetime = m_universe->lreg.alloc(
      [universe = m_universe, ready_lifetime](Scope &scope) noexcept {
        using persistant =
            typename system_traits::active_persistent_requirements;
        using entry = typename system_traits::active_entry_only_requirements;
        for_each_type<persistant>([&]<typename object>() noexcept {
          lifetime_id dependency = get_object_lifetime<object>(universe);
          scope.require(dependency, lifetime_requirement::persistent);
        });
        for_each_type<entry>([&]<typename object>() noexcept {
          lifetime_id dependency = get_object_lifetime<object>(universe);
          scope.require(dependency, lifetime_requirement::entry);
        });
        scope.require(ready_lifetime, lifetime_requirement::persistent);
      });
  std::construct_at(block, loc, ready_lifetime, active_lifetime, m_universe);
  header = static_cast<system_header *>(block);
  m_headers[id.m_index] = header;

  schedule_op self = block->op();

  using after = typename system_traits::sequenced_after;
  using before = typename system_traits::sequenced_before;

  for_each_type<after>([&]<typename Other>() noexcept {
    using other_type = std::remove_cvref_t<Other>;
    static_assert(!std::same_as<system_type, other_type>,
                  "A system cannot be sequenced after itself");
    system_header *other = require_system_header<other_type>();
    m_universe->schedule.sequenced_after(other->op(), self);
  });
  for_each_type<before>([&]<typename Other>() noexcept {
    using other_type = std::remove_cvref_t<Other>;
    static_assert(!std::same_as<system_type, other_type>,
                  "A system cannot be sequenced before itself");
    system_header *other = require_system_header<other_type>();
    m_universe->schedule.sequenced_after(self, other->op());
  });

  return header;
}

inline SystemRegistry::~SystemRegistry() noexcept {
  assert(m_cmdbuf.peek() == null_cmd_index);
  destroy_all();

  drain_cmds(&m_universe->scheduler, &m_universe->sr_domain,
             op_scope(acq_rel(m_universe->sr_location)), &m_cmdbuf,
             m_universe->rreg.cmds());

  for (uint32_t i = 0; i < m_headers.size(); ++i) {
    system_header *header = m_headers[i];
    if (header != nullptr) {
      if (m_universe->lreg.is_constructed(header->get_active_lifetime())) {
        m_universe->lreg.destruct(m_universe, header->get_active_lifetime());
      }
      if (m_universe->lreg.is_constructed(header->get_ready_lifetime())) {
        m_universe->lreg.destruct(m_universe, header->get_ready_lifetime());
      }
      m_universe->scheduler.fence(op_scope(acq_rel(header->get_location())));
      m_universe->scheduler.free(header->get_location());
      m_universe->lreg.free(header->get_active_lifetime());
      m_universe->lreg.free(header->get_ready_lifetime());
      std::destroy_at(header);
    }
  }
}

inline location SystemRegistry::get_registry_location() const noexcept {
  return m_universe->sr_location;
}

inline void SystemRegistry::destroy_all() noexcept {
  m_universe->lreg.destruct(m_universe, m_sreg_lifetime);
}

} // namespace strobe::ecs
