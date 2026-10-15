#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/type_id.hpp"

#include <strobe/core/containers/string.hpp>
#include <strobe/core/containers/vector.hpp>

#include <concepts>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe::flow {

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
};

} // namespace strobe::flow
