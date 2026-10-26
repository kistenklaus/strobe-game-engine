#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/argument_group.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/type_id.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/string.hpp>
#include <strobe/core/containers/vector.hpp>

#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe::flow {

namespace detail {

using ResolvedOperationId = std::uint32_t;
inline constexpr ResolvedOperationId null_resolved_operation_id =
    std::numeric_limits<ResolvedOperationId>::max();

struct ResolvedOperation {
  struct Immediate {
    TypeId type;
    std::size_t offset;
    std::size_t size;
    std::size_t alignment;
    std::size_t element_size;
    std::uint32_t element_count;
  };

  OpMetadata metadata;
  std::optional<ExternalOp> external;
  Vector<TypeId, allocator_ref> argument_types;
  Vector<AccessMode, allocator_ref> argument_accesses;
  Vector<ArgumentGroup, allocator_ref> argument_groups;
  Vector<std::uint32_t, allocator_ref> argument_elements;
  Vector<Immediate, allocator_ref> immediates;
  Vector<std::byte, allocator_ref> immediate_data;
  std::move_only_function<void(Lifetime, span<void *const>,
                               span<const std::uint32_t>,
                               span<const ArgumentGroup>,
                               span<const std::uint32_t>, span<void *const>)>
      runtime;
};

} // namespace detail

/**
 * \brief Owns typed sidecar state for one Flow compilation.
 * \code{.cpp}
 * class CompileContext;
 * \endcode
 *
 * State is identified by a plugin-defined string and remains alive through all
 * compiler plugin stages. The completed context is retained by ProgramData.
 */
class CompileContext {
public:
  /**
   * \brief Registers an implementation for a compiler-produced node.
   * \code{.cpp}
   * detail::ResolvedOperationId
   * resolve_operation(detail::ResolvedOperation operation);
   * \endcode
   *
   * \param operation Runtime or external implementation and its argument
   * contract.
   * \return Stable handle carried by compiler IR nodes.
   */
  detail::ResolvedOperationId
  resolve_operation(detail::ResolvedOperation operation);

  /**
   * \brief Transfers a registered implementation to program emission.
   * \code{.cpp}
   * detail::ResolvedOperation release_resolved_operation(
   *     detail::ResolvedOperationId operation);
   * \endcode
   *
   * \param operation Handle returned by `resolve_operation`.
   * \return Registered runtime implementation and argument contract.
   * \attention 1 Each registered implementation can be released once.
   */
  detail::ResolvedOperation
  release_resolved_operation(detail::ResolvedOperationId operation);

  /**
   * \brief Constructs or destroys a per-compilation context.
   * \code{.cpp}
   * CompileContext();
   * CompileContext(const CompileContext&) = delete;
   * CompileContext& operator=(const CompileContext&) = delete;
   * CompileContext(CompileContext&&) noexcept;
   * CompileContext& operator=(CompileContext&&) noexcept;
   * ~CompileContext();
   * \endcode
   */
  CompileContext() = default;
  CompileContext(const CompileContext &) = delete;
  CompileContext &operator=(const CompileContext &) = delete;
  CompileContext(CompileContext &&other) noexcept;
  CompileContext &operator=(CompileContext &&other) noexcept;
  ~CompileContext();

private:
  void clear() noexcept;

public:
  /**
   * \brief Constructs typed state for a plugin identifier.
   * \code{.cpp}
   * template<typename T, typename... Args>
   * T& emplace_plugin_data(std::string_view id, Args&&... args);
   * \endcode
   *
   * \param id Plugin-defined state identifier.
   * \param args Arguments used to construct the state.
   * \return The newly constructed state.
   * \throws std::invalid_argument if the identifier is empty or already used.
   */
  template <typename T, typename... Args>
    requires std::is_nothrow_destructible_v<T> &&
             std::constructible_from<T, Args...>
  T &emplace_plugin_data(std::string_view id, Args &&...args) {
    if (id.empty()) {
      throw std::invalid_argument("Flow plugin data id cannot be empty");
    }
    if (find(id) != nullptr) {
      throw std::invalid_argument("Flow plugin data id is already present");
    }
    void *storage = allocator_ref{}.allocate(sizeof(T), alignof(T));
    if (storage == nullptr) {
      throw std::bad_alloc();
    }
    bool constructed = false;
    try {
      std::construct_at(static_cast<T *>(storage), std::forward<Args>(args)...);
      constructed = true;
      m_entries.push_back(Entry{
          .id = String<allocator_ref>(id, allocator_ref{}),
          .type = type_id<T>(),
          .value = storage,
          .size = sizeof(T),
          .alignment = alignof(T),
          .destroy =
              [](void *value) noexcept {
                std::destroy_at(static_cast<T *>(value));
              },
      });
    } catch (...) {
      if (constructed) {
        std::destroy_at(static_cast<T *>(storage));
      }
      allocator_ref{}.deallocate(storage, sizeof(T), alignof(T));
      throw;
    }
    return *static_cast<T *>(storage);
  }

  /**
   * \brief Returns typed state stored for a plugin identifier.
   * \code{.cpp}
   * template<typename T>
   * T& plugin_data(std::string_view id);
   *
   * template<typename T>
   * const T& plugin_data(std::string_view id) const;
   * \endcode
   *
   * \param id Plugin-defined state identifier.
   * \return The stored state.
   * \throws std::out_of_range if the identifier is absent.
   * \throws std::invalid_argument if the stored type differs from T.
   */
  template <typename T> T &plugin_data(std::string_view id) {
    return const_cast<T &>(std::as_const(*this).plugin_data<T>(id));
  }

  template <typename T> const T &plugin_data(std::string_view id) const {
    const auto *entry = find(id);
    if (entry == nullptr) {
      throw std::out_of_range("Flow plugin data id is not present");
    }
    if (entry->type != type_id<T>()) {
      throw std::invalid_argument("Flow plugin data type does not match");
    }
    return *static_cast<const T *>(entry->value);
  }

private:
  struct Entry {
    String<allocator_ref> id;
    TypeId type;
    void *value;
    std::size_t size;
    std::size_t alignment;
    void (*destroy)(void *) noexcept;
  };

  Entry *find(std::string_view id);
  const Entry *find(std::string_view id) const;

  Vector<Entry, allocator_ref> m_entries;
  Vector<detail::ResolvedOperation, allocator_ref> m_resolved_operations;
};

} // namespace strobe::flow
