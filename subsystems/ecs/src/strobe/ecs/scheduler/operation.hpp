#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/ecs/scheduler/job_scheduler.hpp"
#include "strobe/ecs/scheduler/location.hpp"
#include "strobe/ecs/scheduler/op_scope.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

namespace strobe::ecs {

class Scheduler;

namespace details {

struct operation_builder;
}

struct operation {
  friend class Scheduler;
  friend struct details::operation_builder;
  static inline location empty_location_list[1] = {null_location};

  operation() noexcept
      : object(nullptr), execute(nullptr), acquire(empty_location_list),
        release(empty_location_list), acq_rel(empty_location_list) {}

private:
  // job fn
  void *object = nullptr;
  uint32_t (*execute)(void *, uint32_t) noexcept = nullptr;
  // scope
  const location *acquire = empty_location_list; // null terminated
  const location *release = empty_location_list; // null terminated
  const location *acq_rel = empty_location_list; // null terminated
};

namespace details {

template <size_t N> struct static_location_storage {
  location data[N];
};
template <> struct static_location_storage<0> {};

} // namespace details

template <size_t AcquireCount, size_t ReleaseCount, size_t AcqRelCount>
struct operation_storage {
private:
  static constexpr size_t acquire_buf_size =
      AcquireCount == 0 ? 0 : AcquireCount + 1;
  static constexpr size_t release_buf_size =
      ReleaseCount == 0 ? 0 : ReleaseCount + 1;
  static constexpr size_t acq_rel_buf_size =
      AcqRelCount == 0 ? 0 : AcqRelCount + 1;
  static constexpr size_t buf_size =
      acquire_buf_size + release_buf_size + acq_rel_buf_size;

  static constexpr size_t acquire_offset = 0;
  static constexpr size_t release_offset = acquire_offset + acquire_buf_size;
  static constexpr size_t acq_rel_offset = release_offset + release_buf_size;

  [[no_unique_address]] details::static_location_storage<buf_size> buf;

public:
  static constexpr size_t acquire_count = AcquireCount;
  static constexpr size_t release_count = ReleaseCount;
  static constexpr size_t acq_rel_count = AcqRelCount;

  constexpr span<location, acquire_count> acquire_locations() noexcept {
    if constexpr (AcquireCount == 0) {
      return {};
    } else {
      return span<location, AcquireCount>{
          buf.data + acquire_offset,
          AcquireCount,
      };
    }
  }

  constexpr span<const location, acquire_count>
  acquire_locations() const noexcept {
    if constexpr (AcquireCount == 0) {
      return {};
    } else {
      return span<location, acquire_count>{
          buf.data + acquire_offset,
          acquire_count,
      };
    }
  }

  constexpr span<location, release_count> release_locations() noexcept {
    if constexpr (ReleaseCount == 0) {
      return {};
    } else {
      return span<location, release_count>{
          buf.data + release_offset,
          release_count,
      };
    }
  }

  constexpr span<const location, release_count>
  release_locations() const noexcept {
    if constexpr (ReleaseCount == 0) {
      return {};
    } else {
      return span<location, release_count>{
          buf.data + release_offset,
          release_count,
      };
    }
  }

  constexpr span<location, acq_rel_count> acq_rel_locations() noexcept {
    if constexpr (AcqRelCount == 0) {
      return {};
    } else {
      return span<location, acq_rel_count>{
          buf.data + acq_rel_offset,
          acq_rel_count,
      };
    }
  }

  constexpr span<const location, acq_rel_count>
  acq_rel_locations() const noexcept {
    if constexpr (AcqRelCount == 0) {
      return {};
    } else {
      return span<location, acq_rel_count>{
          buf.data + acq_rel_offset,
          acq_rel_count,
      };
    }
  }

  constexpr location *acquire_buffer() noexcept {
    if constexpr (AcquireCount == 0) {
      return nullptr;
    } else {
      return buf.data + acquire_offset;
    }
  }

  constexpr location *release_buffer() noexcept {
    if constexpr (ReleaseCount == 0) {
      return nullptr;
    } else {
      return buf.data + release_offset;
    }
  }

  constexpr location *acq_rel_buffer() noexcept {
    if constexpr (AcqRelCount == 0) {
      return nullptr;
    } else {
      return buf.data + acq_rel_offset;
    }
  }
};

namespace details {

template <size_t AcquireCount, size_t ReleaseCount, size_t AcqRelCount>
class OperationMemoryScope {
public:
  using storage_type =
      operation_storage<AcquireCount, ReleaseCount, AcqRelCount>;

  explicit OperationMemoryScope(storage_type &storage) noexcept
      : m_storage(std::addressof(storage)) {}

  void acquire(location loc) noexcept {
    if constexpr (AcquireCount == 0) {
      (void)loc;
      assert(false);
    } else {
      assert(m_acquireIndex < AcquireCount);
      m_storage->acquire_locations()[m_acquireIndex++] = loc;
    }
  }

  void release(location loc) noexcept {
    if constexpr (ReleaseCount == 0) {
      (void)loc;
      assert(false);
    } else {
      assert(m_releaseIndex < ReleaseCount);
      m_storage->release_buffer()[m_releaseIndex++] = loc;
    }
  }

  void acq_rel(location loc) noexcept {
    if constexpr (AcqRelCount == 0) {
      (void)loc;
      assert(false);
    } else {
      assert(m_acqRelIndex < AcqRelCount);
      m_storage->acq_rel_buffer()[m_acqRelIndex++] = loc;
    }
  }

  void finalize() noexcept {
    assert(m_acquireIndex == AcquireCount);
    assert(m_releaseIndex == ReleaseCount);
    assert(m_acqRelIndex == AcqRelCount);

    if constexpr (AcquireCount != 0) {
      m_storage->acquire_buffer()[AcquireCount] = location{};
    }

    if constexpr (ReleaseCount != 0) {
      m_storage->release_buffer()[ReleaseCount] = location{};
    }

    if constexpr (AcqRelCount != 0) {
      m_storage->acq_rel_buffer()[AcqRelCount] = location{};
    }
  }

private:
  storage_type *m_storage = nullptr;

  size_t m_acquireIndex = 0;
  size_t m_releaseIndex = 0;
  size_t m_acqRelIndex = 0;
};

struct operation_builder {
  static void bind(operation &op, void *object,
                   uint32_t (*execute)(void *, uint32_t) noexcept,
                   const location *acquire, const location *release,
                   const location *acq_rel) noexcept {
    assert(execute != nullptr);
    op.object = object;
    op.execute = execute;
    op.acquire = acquire != nullptr ? acquire : operation::empty_location_list;
    op.release = release != nullptr ? release : operation::empty_location_list;
    op.acq_rel = acq_rel != nullptr ? acq_rel : operation::empty_location_list;
  }
};

} // namespace details

template <job_fn Fn, size_t AcquireCount, size_t ReleaseCount,
          size_t AcqRelCount>
struct operation_block {
public:
  using fn_type = Fn;

  using storage_type =
      operation_storage<AcquireCount, ReleaseCount, AcqRelCount>;

  using memory_scope_type =
      details::OperationMemoryScope<AcquireCount, ReleaseCount, AcqRelCount>;

  static_assert(!std::is_reference_v<Fn>);
  static_assert(!std::is_const_v<Fn>);
  static_assert(!std::is_volatile_v<Fn>);

  operation_block() = delete;

  operation_block(const operation_block &) = delete;
  operation_block &operator=(const operation_block &) = delete;

  operation_block(operation_block &&) = delete;
  operation_block &operator=(operation_block &&) = delete;

  template <typename ScopeFn, typename FnArg>
    requires(std::is_nothrow_constructible_v<Fn, FnArg &&> &&
             std::is_nothrow_invocable_v<ScopeFn &&, memory_scope_type &>)
  explicit operation_block(ScopeFn &&scopeFn, FnArg &&fn) noexcept
      : m_fn(std::forward<FnArg>(fn)) {
    memory_scope_type scope{m_storage};

    std::forward<ScopeFn>(scopeFn)(scope);

    scope.finalize();

    details::operation_builder::bind(
        m_operation, static_cast<void *>(std::addressof(m_fn)),
        &operation_block::execute, m_storage.acquire_buffer(),
        m_storage.release_buffer(), m_storage.acq_rel_buffer());
  }

  template <typename ScopeDesc, typename FnArg>
    requires(is_operation_scope_desc_v<ScopeDesc> &&
             std::is_nothrow_constructible_v<Fn, FnArg &&>)
  explicit operation_block(ScopeDesc &&scopeDesc, FnArg &&fn) noexcept
      : m_fn(std::forward<FnArg>(fn)) {
    static_assert(std::remove_cvref_t<ScopeDesc>::acquire_count ==
                  AcquireCount);

    static_assert(std::remove_cvref_t<ScopeDesc>::release_count ==
                  ReleaseCount);

    static_assert(std::remove_cvref_t<ScopeDesc>::acq_rel_count == AcqRelCount);

    memory_scope_type scope{m_storage};

    scopeDesc.apply(scope);

    scope.finalize();

    details::operation_builder::bind(
        m_operation, static_cast<void *>(std::addressof(m_fn)),
        &operation_block::execute, m_storage.acquire_buffer(),
        m_storage.release_buffer(), m_storage.acq_rel_buffer());
  }

  const operation *ptr() const noexcept { return std::addressof(m_operation); }

  const operation &get() const noexcept { return m_operation; }

  fn_type &function() noexcept { return m_fn; }

  const fn_type &function() const noexcept { return m_fn; }

  storage_type &storage() noexcept { return m_storage; }

  const storage_type &storage() const noexcept { return m_storage; }

private:
  static uint32_t execute(void *object, uint32_t invocation) noexcept {
    Fn &fn = *static_cast<Fn *>(object);

    if constexpr (std::is_invocable_v<Fn &, uint32_t>) {
      static_assert(std::is_nothrow_invocable_v<Fn &, uint32_t>);

      if constexpr (std::is_void_v<std::invoke_result_t<Fn &, uint32_t>>) {
        fn(invocation);
        return 0;
      } else {
        return static_cast<uint32_t>(fn(invocation));
      }
    } else {
      static_assert(std::is_nothrow_invocable_v<Fn &>);

      if constexpr (std::is_void_v<std::invoke_result_t<Fn &>>) {
        fn();
        return 0;
      } else {
        return static_cast<uint32_t>(fn());
      }
    }
  }

private:
  operation m_operation;

  [[no_unique_address]]
  storage_type m_storage;

  [[no_unique_address]]
  Fn m_fn;
};

template <size_t AcquireCount, size_t ReleaseCount, size_t AcqRelCount,
          typename ScopeFn, job_fn Fn>
  requires(std::is_nothrow_constructible_v<std::remove_cvref_t<Fn>, Fn &&> &&
           std::is_nothrow_invocable_v<
               ScopeFn &&, details::OperationMemoryScope<
                               AcquireCount, ReleaseCount, AcqRelCount> &>)
auto make_operation_block(ScopeFn &&scopeFn, Fn &&fn) noexcept {
  using fn_type = std::remove_cvref_t<Fn>;

  return operation_block<fn_type, AcquireCount, ReleaseCount, AcqRelCount>{
      std::forward<ScopeFn>(scopeFn), std::forward<Fn>(fn)};
}

template <typename ScopeDesc, job_fn Fn>
  requires(is_operation_scope_desc_v<ScopeDesc> &&
           std::is_nothrow_constructible_v<std::remove_cvref_t<Fn>, Fn &&>)
auto make_operation_block(ScopeDesc &&scopeDesc, Fn &&fn) noexcept {
  using desc_type = std::remove_cvref_t<ScopeDesc>;
  using fn_type = std::remove_cvref_t<Fn>;

  return operation_block<fn_type, desc_type::acquire_count,
                         desc_type::release_count, desc_type::acq_rel_count>{
      std::forward<ScopeDesc>(scopeDesc), std::forward<Fn>(fn)};
}

} // namespace strobe::ecs
