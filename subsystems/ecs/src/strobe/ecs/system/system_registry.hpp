#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/core/memory/sync_monotonic_pool_allocator.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/cmd/cmd_domain.hpp"
#include "strobe/ecs/cmd/cmd_traits.hpp"
#include "strobe/ecs/scheduler/op_scope.hpp"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <limits>
#include <memory>
#include <type_traits>

namespace strobe::ecs {

class SystemRegistry;

struct system_header;
struct Universe;

struct null_system_id_t {};

struct system_id {
  friend class SystemRegistry;
  system_id() : m_index(invalid_index) {}
  system_id(null_system_id_t) : m_index(invalid_index) {}

  friend inline bool operator==(const system_id &id,
                                null_system_id_t) noexcept {
    return id.m_index == invalid_index;
  }
  friend inline bool operator==(null_system_id_t,
                                const system_id &id) noexcept {
    return id.m_index == invalid_index;
  }
  friend inline bool operator!=(const system_id &id,
                                null_system_id_t) noexcept {
    return id.m_index != invalid_index;
  }
  friend inline bool operator!=(null_system_id_t,
                                const system_id &id) noexcept {
    return id.m_index != invalid_index;
  }

private:
  static constexpr uint32_t invalid_index =
      std::numeric_limits<uint32_t>::max();
  explicit system_id(uint32_t index) : m_index(index) {}
  uint32_t m_index;
};

class SystemRegistry {
public:
  using allocator = strobe::ecs::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

  explicit SystemRegistry(Universe *universe, const allocator &alloc) noexcept;

  SystemRegistry(const SystemRegistry &) = delete;
  SystemRegistry &operator=(const SystemRegistry &) = delete;
  SystemRegistry(SystemRegistry &&) = delete;
  SystemRegistry &operator=(SystemRegistry &&) = delete;

  ~SystemRegistry() noexcept;

private:
  using block_allocator = MonotonicResource<allocator>;

  using system_curry = void (*)(Universe *);

public:
  enum system_cmd_tag : uint8_t {
    SYSTEM_CMD_CREATE_TAG,
    SYSTEM_CMD_DESTROY_TAG,
    SYSTEM_CMD_ENABLE_TAG,
    SYSTEM_CMD_DISABLE_TAG,
  };

  struct system_cmd {
    std::atomic<system_cmd *> next = nullptr;
    cmd_index index = null_cmd_index;

    system_cmd() noexcept = default;
    explicit system_cmd(cmd_index index, system_cmd_tag tag, system_id id,
                        system_curry curry) noexcept
        : index(index), m_tag(tag), m_id(id), m_curry(curry) {}

    void operator()(Universe *universe) noexcept;

  private:
    system_cmd_tag m_tag;
    system_id m_id;
    system_curry m_curry;
  };
  using cmd_allocator =
      LockFreeMonotonicPoolResource<sizeof(system_cmd), alignof(system_cmd),
                                    allocator>;

  struct system_cmdbuf {
  public:
    using scope = release_access;

    explicit system_cmdbuf(Universe *universe, const allocator &alloc) noexcept;

    system_cmdbuf(const system_cmdbuf &) = delete;
    system_cmdbuf &operator=(const system_cmdbuf &) = delete;
    system_cmdbuf(system_cmdbuf &&) = delete;
    system_cmdbuf &operator=(system_cmdbuf &&) = delete;

    ~system_cmdbuf() noexcept {
      assert(peek() == null_cmd_index);
      if (m_head != &m_stub) {
        std::destroy_at(m_head);
        m_cmdPool.deallocate(m_head);
      }
    }

    template <typename... Args> void emplace(Args &&...args) {
      cmd_index index = m_domain->next();
      assert(index != null_cmd_index);
      auto *cmd = static_cast<system_cmd *>(m_cmdPool.allocate());
      std::construct_at(cmd, index, std::forward<Args>(args)...);
      cmd->next.store(nullptr, std::memory_order_relaxed);
      m_tail->next.store(cmd, std::memory_order_release);
      m_tail = cmd;
    }

    cmd_index peek() const noexcept {
      system_cmd *cmd = m_head->next.load(std::memory_order_acquire);
      if (cmd == nullptr) {
        return null_cmd_index;
      }
      return cmd->index;
    }

    void step() noexcept;

    const cmd_domain *domain() const noexcept { return m_domain; }
    cmd_domain *domain() noexcept { return m_domain; }

  private:
    Universe *m_universe;
    cmd_allocator m_cmdPool;
    cmd_domain *m_domain;
    system_cmd m_stub;
    system_cmd *m_head;
    system_cmd *m_tail;
  };
  static_assert(command_buffer<system_cmdbuf>);

public:
  system_cmdbuf *cmds() noexcept { return &m_cmdbuf; }

  template <typename S> system_id cmd_create();

  void cmd_destroy(system_id id) noexcept {
    m_cmdbuf.emplace(SYSTEM_CMD_DESTROY_TAG, id, nullptr);
  }
  template <typename S> void cmd_destroy() noexcept {
    using system_type = std::remove_cvref_t<S>;
    system_id id{system_type_id<system_type>()};
    cmd_destroy(id);
  }

  void cmd_enable(system_id id) noexcept {
    m_cmdbuf.emplace(SYSTEM_CMD_ENABLE_TAG, id, nullptr);
  }

  template <typename S> void cmd_enable() noexcept {
    using system_type = std::remove_cvref_t<S>;
    system_id id{system_type_id<system_type>()};
    cmd_enable(id);
  }

  void cmd_disable(system_id id) noexcept {
    m_cmdbuf.emplace(SYSTEM_CMD_DISABLE_TAG, id, nullptr);
  }

  template <typename S> void cmd_disable() noexcept {
    using system_type = std::remove_cvref_t<S>;
    system_id id{system_type_id<system_type>()};
    cmd_disable(id);
  }

  location get_registry_location() const noexcept { return m_location; }

  // may only be called from the submitting thread!
  template <typename S> system_header *require_system_header() noexcept;

private:
  static uint32_t next_system_type_id() noexcept {
    static std::atomic<uint32_t> id_acc = 0;
    return id_acc.fetch_add(1, std::memory_order_relaxed);
  }

  // thread-safe!
  template <typename S>
    requires(!std::is_reference_v<S>)
  static uint32_t system_type_id() noexcept {
    static uint32_t id = next_system_type_id();
    return id;
  }

private:
  Universe *m_universe;
  location m_location;
  Vector<system_header *, allocator> m_headers;
  block_allocator m_blockAlloc;
  system_cmdbuf m_cmdbuf;
};

} // namespace strobe::ecs
