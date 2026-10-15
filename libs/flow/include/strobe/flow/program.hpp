#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/detail/program_data.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/resource_id.hpp"

#include <strobe/core/memory/box.hpp>
#include <strobe/core/sync/job_scheduler.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace strobe::flow {

namespace detail {
class ProgramRuntime;
} // namespace detail

/**
 * \brief Owns a compiled Flow program and its reusable execution state.
 * \code{.cpp}
 * class Program;
 * \endcode
 */
class Program {
public:
  /**
   * \brief Constructs, moves, or destroys a program.
   * \code{.cpp}
   * Program();
   * explicit Program(detail::ProgramData data);
   * Program(const Program&) = delete;
   * Program& operator=(const Program&) = delete;
   * Program(Program&&) noexcept;
   * Program& operator=(Program&&) noexcept;
   * ~Program();
   * \endcode
   *
   * \param data Compiled program tables used to initialize execution.
   */
  Program();
  explicit Program(detail::ProgramData data);
  Program(const Program &) = delete;
  Program &operator=(const Program &) = delete;
  Program(Program &&) noexcept;
  Program &operator=(Program &&) noexcept;
  ~Program();

  /**
   * \brief Accesses the compiled program tables.
   * \code{.cpp}
   * const detail::ProgramData& data() const noexcept;
   * detail::ProgramData& data() noexcept;
   * \endcode
   *
   * \return The compiled program data.
   */
  const detail::ProgramData &data() const noexcept;
  detail::ProgramData &data() noexcept;

  /**
   * \brief Returns the number of recorded operations represented by the
   * program.
   * \code{.cpp}
   * std::size_t operation_count() const noexcept;
   * \endcode
   *
   * \return The source operation count.
   */
  std::size_t operation_count() const noexcept;

  /**
   * \brief Returns the number of recorded resources represented by the program.
   * \code{.cpp}
   * std::size_t resource_count() const noexcept;
   * \endcode
   *
   * \return The source resource count.
   */
  std::size_t resource_count() const noexcept;

  /**
   * \brief Returns the number of external resources expected by the program.
   * \code{.cpp}
   * std::size_t external_count() const noexcept;
   * \endcode
   *
   * \return The source external-resource count.
   */
  std::size_t external_count() const noexcept;

  /**
   * \brief Binds an externally owned object to a symbolic resource name.
   * \code{.cpp}
   * void bind_external(std::string_view name, void* pointer);
   * template<typename T>
   * void bind_external(std::string_view name, T* pointer);
   * \endcode
   *
   * \param name External resource name recorded with flow::external.
   * \param pointer Object supplied for subsequent submissions.
   * \attention 1 The object must remain alive until all work using the binding
   * has completed.
   */
  void bind_external(std::string_view name, void *pointer);

  template <typename T> void bind_external(std::string_view name, T *pointer) {
    bind_external(name, static_cast<void *>(pointer));
  }

  /**
   * \brief Returns the pointer bound to an external resource.
   * \code{.cpp}
   * void* external(ResourceId resource) const;
   * \endcode
   *
   * \param resource External resource identifier.
   * \return The bound object pointer.
   */
  void *external(ResourceId resource) const;

  /**
   * \brief Tests whether every external resource has a binding.
   * \code{.cpp}
   * bool externals_bound() const;
   * \endcode
   *
   * \return Whether all external resource bindings are present.
   */
  bool externals_bound() const;

  /**
   * \brief Tests whether every external operation has been resolved.
   * \code{.cpp}
   * bool libraries_bound() const noexcept;
   * \endcode
   *
   * \return Whether all external operation libraries are bound.
   */
  bool libraries_bound() const noexcept;

  /**
   * \brief Resolves operations associated with an external library identifier.
   * \code{.cpp}
   * void bind_library(std::string_view library_id, Library library);
   * \endcode
   *
   * \param library_id Symbolic library identifier used by external operations.
   * \param library Borrowed symbol resolver.
   * \attention 1 Libraries must be bound before the first submission.
   */
  void bind_library(std::string_view library_id, Library library);

  /**
   * \brief Adds tokens to a named acquire semaphore.
   * \code{.cpp}
   * void release(std::string_view name, std::uint64_t count = 1);
   * \endcode
   *
   * \param name Semaphore name recorded with flow::acquire.
   * \param count Number of tokens to add.
   */
  void release(std::string_view name, std::uint64_t count = 1);

  /**
   * \brief Tries to reserve a storage generation for a resource.
   * \code{.cpp}
   * std::optional<detail::ResourceGeneration>
   * try_acquire_generation(ResourceId resource);
   * \endcode
   *
   * \param resource Resource whose storage is requested.
   * \return A generation when a slot is available, otherwise std::nullopt.
   */
  std::optional<detail::ResourceGeneration>
  try_acquire_generation(ResourceId resource);

  /**
   * \brief Waits for and reserves a storage generation for a resource.
   * \code{.cpp}
   * detail::ResourceGeneration acquire_generation(ResourceId resource);
   * \endcode
   *
   * \param resource Resource whose storage is requested.
   * \return The reserved generation.
   */
  detail::ResourceGeneration acquire_generation(ResourceId resource);

  /**
   * \brief Resolves a live resource generation to its storage address.
   * \code{.cpp}
   * void* resolve(const detail::ResourceGeneration& generation) const;
   * \endcode
   *
   * \param generation Live generation to resolve.
   * \return The generation storage address.
   */
  void *resolve(const detail::ResourceGeneration &generation) const;

  /**
   * \brief Marks a reserved resource generation as constructed.
   * \code{.cpp}
   * void mark_constructed(const detail::ResourceGeneration& generation);
   * \endcode
   *
   * \param generation Reserved generation containing a constructed object.
   */
  void mark_constructed(const detail::ResourceGeneration &generation);

  /**
   * \brief Destroys and releases a resource generation.
   * \code{.cpp}
   * void release_generation(const detail::ResourceGeneration& generation);
   * \endcode
   *
   * \param generation Live generation to release.
   */
  void release_generation(const detail::ResourceGeneration &generation);

  /**
   * \brief Rethrows the first error captured by execution jobs.
   * \code{.cpp}
   * void rethrow_execution_error() const;
   * \endcode
   */
  void rethrow_execution_error() const;

  /**
   * \brief Submits executable work until a boundary prevents further progress.
   * \code{.cpp}
   * sync::job_id submit(sync::JobScheduler* scheduler);
   * \endcode
   *
   * \param scheduler Persistent scheduler used for this and later submissions.
   * \return A job that completes after all work submitted by this call.
   * \attention 1 The program and scheduler must outlive all submitted jobs.
   * \attention 2 Calls to submit and binding operations require external
   * synchronization.
   */
  sync::job_id submit(sync::JobScheduler *scheduler);

private:
  detail::ProgramRuntime &runtime() const;
  void execute_node(std::uint32_t activation, detail::NodeId node, bool prepare,
                    bool force = false);
  bool execute_control_iteration(detail::ControlId control,
                                 std::uint32_t activation,
                                 std::uint32_t invocation);
  void execute_control(detail::ControlId control, std::uint32_t activation);
  void release_after_node(std::uint32_t activation, detail::NodeId node);

  detail::ProgramData m_data;
  Box<detail::ProgramRuntime, allocator_ref> m_runtime;
  detail::NodeId m_resume_node = 0;
  std::optional<std::uint32_t> m_resume_activation;
  sync::JobScheduler *m_scheduler = nullptr;
  std::optional<sync::job_id> m_last_completion;
};

} // namespace strobe::flow
