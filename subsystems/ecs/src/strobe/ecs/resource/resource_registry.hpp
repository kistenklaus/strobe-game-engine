#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/memory/monotonic_pool_allocator.hpp"
#include "strobe/core/memory/monotonic_resource.hpp"
#include "strobe/core/memory/sync_monotonic_pool_allocator.hpp"
#include "strobe/core/type_traits/fixed_string.hpp"
#include "strobe/core/type_traits/type_name.hpp"
#include "strobe/ecs/allocator.hpp"
#include "strobe/ecs/cmd/cmd_domain.hpp"
#include "strobe/ecs/cmd/cmd_traits.hpp"
#include "strobe/ecs/lifetime/lifetime_hook.hpp"
#include "strobe/ecs/lifetime/lifetime_registry.hpp"
#include "strobe/ecs/scheduler/location.hpp"
#include "strobe/ecs/scheduler/op_scope.hpp"
#include "strobe/ecs/scheduler/scheduler.hpp"
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <limits>
#include <memory>
#include <type_traits>

namespace strobe::ecs {

class ResourceRegistry;

struct null_resource_id_t {};
static constexpr null_resource_id_t null_resource_id = {};

struct resource_id {
  friend class ResourceRegistry;

public:
  resource_id() : m_index(invalid_index) {}

  friend inline bool operator==(const resource_id &id,
                                null_resource_id_t) noexcept {
    return id.m_index == invalid_index;
  }
  friend inline bool operator==(null_resource_id_t,
                                const resource_id &id) noexcept {
    return id.m_index == invalid_index;
  }
  friend inline bool operator!=(const resource_id &id,
                                null_resource_id_t) noexcept {
    return id.m_index != invalid_index;
  }
  friend inline bool operator!=(null_resource_id_t,
                                const resource_id &id) noexcept {
    return id.m_index != invalid_index;
  }

private:
  static constexpr uint32_t invalid_index =
      std::numeric_limits<uint32_t>::max();
  resource_id(uint32_t index) : m_index(index) {}
  uint32_t m_index;
};

class ResourceRegistry {
public:
  // must be thread-safe!
  using allocator = strobe::ecs::allocator_ref;
  using allocator_traits = AllocatorTraits<allocator>;

  explicit ResourceRegistry(Universe *universe, const allocator &alloc);

  ResourceRegistry(const ResourceRegistry &o) = delete;
  ResourceRegistry &operator=(const ResourceRegistry &o) = delete;
  ResourceRegistry(ResourceRegistry &&o) = delete;
  ResourceRegistry &operator=(ResourceRegistry &&o) = delete;

  ~ResourceRegistry() noexcept;

private:
  // must not be thread-safe, and should later probably be changed to something
  // like a local arena (best fit or something).
  using resource_allocator = MonotonicResource<allocator>;

  struct resource_header : lifetime_hook {
    ~resource_header() = default;

    resource_header(const resource_header &) = delete;
    resource_header &operator=(const resource_header &) = delete;
    resource_header(resource_header &&) = delete;
    resource_header &operator=(resource_header &&) = delete;

    // submission-thread only
    inline lifetime_id require_lifetime(LifetimeRegistry *lreg) noexcept {
      if (m_lifetime == null_lifetime_id) {
        m_lifetime = lreg->alloc([](auto &) noexcept {});
        lreg->install(m_lifetime, this);
      }
      return m_lifetime;
    }

    // submission-thread only
    inline location require_location(Scheduler *scheduler) noexcept {
      if (m_location == null_location) {
        m_location = scheduler->alloc();
      }
      return m_location;
    }

    lifetime_id get_lifetime() const noexcept {
      assert(m_lifetime != null_lifetime_id);
      return m_lifetime;
    }

    location get_location() const noexcept {
      assert(m_location != null_location);
      return m_location;
    }

    bool has_lifetime() const noexcept {
      return m_lifetime != null_lifetime_id;
    }

    bool has_location() const noexcept { return m_location != null_location; }

    virtual size_t resource_size() const noexcept = 0;
    virtual size_t resource_alignment() const noexcept = 0;

    void *ptr = nullptr;

    void *pop_storage(resource_allocator *alloc) noexcept {
      ZoneScopedN("rreg::pop-resource-storage");
      void *head = m_freelist.load(std::memory_order_acquire);
      while (head != nullptr) {
        void *next = *static_cast<void **>(head);
        if (m_freelist.compare_exchange_weak(head, next,
                                             std::memory_order_acquire,
                                             std::memory_order_acquire)) {
          return head;
        }
      }
      {
        ZoneScopedN("rreg::allocate-resource-storage");
        return alloc->allocate(resource_size(), resource_alignment());
      }
    }

    void push_storage(void *storage) noexcept {
      assert(storage != nullptr);
      void *head = m_freelist.load(std::memory_order_relaxed);
      do {
        *static_cast<void **>(storage) = head;
      } while (!m_freelist.compare_exchange_weak(
          head, storage, std::memory_order_release, std::memory_order_relaxed));
    }

    void release_storage(resource_allocator *alloc) noexcept {
      void *head = m_freelist.exchange(nullptr, std::memory_order_acquire);
      while (head != nullptr) {
        void *next = *static_cast<void **>(head);
        alloc->deallocate(head, resource_size(), resource_alignment());
        head = next;
      }
    }

  protected:
    explicit resource_header() noexcept = default;

  private:
    lifetime_id m_lifetime = null_lifetime_id;
    location m_location = null_location;

    std::atomic<void *> m_freelist = nullptr;
  };

  template <typename R> struct resource_block final : resource_header {
    using resource_type = std::remove_cvref_t<R>;

    explicit resource_block() noexcept = default;

    void enter(Universe *) noexcept override {
      // doesn't do anything the actual work is done in
      // resource_cmd::operator()
    }

    void exit(Universe *universe) noexcept override;

    size_t resource_size() const noexcept override {
      return std::max(sizeof(resource_type), sizeof(void *));
    }

    size_t resource_alignment() const noexcept override {
      return std::max(alignof(resource_type), alignof(void *));
    }
  };
  using block_allocator =
      MonotonicPoolResource<sizeof(resource_header), alignof(resource_header),
                            allocator>;
  static_assert(sizeof(resource_block<int>) == sizeof(resource_header));
  static_assert(alignof(resource_block<int>) <= alignof(resource_header));

public:
  enum resource_cmd_tag {
    RESOURCE_CMD_CREATE_TAG,
    RESOURCE_CMD_DESTROY_TAG,
  };

  struct resource_cmd {
    std::atomic<resource_cmd *> next = nullptr;
    cmd_index index = null_cmd_index;

    resource_cmd() noexcept = default;
    explicit resource_cmd(cmd_index idx, resource_cmd_tag tag,
                          resource_header *header, void *ptr) noexcept
        : index(idx), m_tag(tag), m_header(header), m_ptr(ptr) {}

    void operator()(Universe *universe) noexcept;

  private:
    resource_cmd_tag m_tag;
    resource_header *m_header;
    void *m_ptr;
  };
  using cmd_allocator =
      MPSCMonotonicPoolResource<sizeof(resource_cmd), alignof(resource_cmd),
                                    allocator>;

  struct resource_cmdbuf {
  public:
    using scope = release_access;

    explicit resource_cmdbuf(Universe *universe, cmd_domain *domain)
        : m_universe(universe), m_domain(domain), m_stub{}, m_head(&m_stub),
          m_tail(&m_stub) {
      m_stub.next.store(nullptr, std::memory_order_relaxed);
    }

    resource_cmdbuf(const resource_cmdbuf &) = delete;
    resource_cmdbuf &operator=(const resource_cmdbuf &) = delete;
    resource_cmdbuf(resource_cmdbuf &&) = delete;
    resource_cmdbuf &operator=(resource_cmdbuf &&) = delete;

    ~resource_cmdbuf() noexcept;

    template <typename... Args> void emplace(Args &&...args) noexcept;

    cmd_index peek() const noexcept {
      resource_cmd *cmd = m_head->next.load(std::memory_order_acquire);
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
    cmd_domain *m_domain;

    resource_cmd m_stub;
    resource_cmd *m_head;
    resource_cmd *m_tail;
  };

public:
  resource_cmdbuf *cmds() noexcept { return &m_cmdbuf; }

  // May be called from a scheduled thread.
  template <typename R, typename... Args>
  resource_id cmd_emplace(Args &&...args) noexcept {
    static constexpr auto debug_name =
        fixed_string{"rreg::cmd_emplace<"} + type_name<R>() + fixed_string{">"};
    ZoneScopedN(debug_name.data());

    using resource_type = std::remove_cvref_t<R>;

    // TODO: make SPSC lockfree from a freelist.

    const resource_id id = get_resource_id<resource_type>();
    resource_header *header = m_resources[id.m_index];
    assert(header != nullptr);

    resource_type *ptr =
        static_cast<resource_type *>(header->pop_storage(&m_resourceAlloc));
    {
      static constexpr auto debug_name =
          fixed_string{"rreg::construct<"} + type_name<R>() + fixed_string{">"};
      ZoneScopedN(debug_name.data());
      std::construct_at<resource_type, Args...>(ptr,
                                                std::forward<Args>(args)...);
    }
    m_cmdbuf.emplace(RESOURCE_CMD_CREATE_TAG, header, ptr);
    return id;
  }

  void cmd_destroy(resource_id id) {
    ZoneScopedN("rreg::cmd_destroy");
    resource_header *header = m_resources[id.m_index];
    assert(header != nullptr);
    m_cmdbuf.emplace(RESOURCE_CMD_DESTROY_TAG, header, nullptr);
  }

  template <typename R> void cmd_destroy() {
    static constexpr auto debug_name =
        fixed_string{"rreg::cmd_destroy<"} + type_name<R>() + fixed_string{">"};
    ZoneScopedN(debug_name.data());
    using resource_type = std::remove_cvref_t<R>;
    const resource_id id = get_resource_id<resource_type>();
    return cmd_destroy(id);
  }

  template <typename R> resource_id get_resource_id() noexcept {
    using resource_type = std::remove_cvref_t<R>;
    const resource_id id{resource_type_id<resource_type>()};
    assert(id != null_resource_id);
    const size_t rsize = m_resources.size();
    if (rsize <= id.m_index) {
      const size_t new_size = std::max<size_t>(
          static_cast<size_t>(id.m_index) + 1, rsize * 3 / 2 + 1);
      m_resources.resize(new_size, nullptr);
    }
    if (m_resources[id.m_index] != nullptr) {
      return id;
    }
    static constexpr auto debug_name =
        fixed_string{"rreg::register_resource_type<"} + type_name<R>() +
        fixed_string{">"};
    using resource_block = resource_block<resource_type>;
    static_assert(sizeof(resource_block) == sizeof(resource_header));
    static_assert(alignof(resource_block) <= alignof(resource_header));
    auto *block = static_cast<resource_block *>(m_blockPool.allocate());
    std::construct_at(block);
    m_resources[id.m_index] = block;

    return id;
  }

  void **get_resource_ptr(resource_id id) noexcept {
    assert(id != null_resource_id);
    assert(id.m_index < m_resources.size());
    resource_header *header = m_resources[id.m_index];
    return &header->ptr;
  }

  location get_resource_location(resource_id id) noexcept;

  lifetime_id get_resource_lifetime(resource_id id) noexcept;

  location get_registry_location() const noexcept;

  void destroy_all() noexcept;

private:
  static uint32_t next_resource_type_id() noexcept;

  template <typename R>
    requires(!std::is_reference_v<R>)
  static uint32_t resource_type_id() noexcept {
    static uint32_t id = next_resource_type_id();
    return id;
  }

private:
  Universe *m_universe;
  Vector<resource_header *, allocator> m_resources;

  resource_allocator m_resourceAlloc;
  block_allocator m_blockPool;
  cmd_allocator m_cmdPool;

  resource_cmdbuf m_cmdbuf;
};

} // namespace strobe::ecs
