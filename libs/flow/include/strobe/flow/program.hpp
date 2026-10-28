#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/detail/job_plan.hpp"
#include "strobe/flow/detail/program_artifact.hpp"
#include "strobe/flow/detail/program_data.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/flow/resource_id.hpp"
#include "strobe/flow/type_id.hpp"

#include <strobe/core/containers/boxed_array.hpp>
#include <strobe/core/containers/boxed_bitset.hpp>
#include <strobe/core/memory/box.hpp>
#include <strobe/core/sync/job_scheduler.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>

namespace strobe::flow {

namespace detail {
class PhysicalStorageState;
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
   * explicit Program(detail::ProgramArtifact artifact);
   * Program(const Program&) = delete;
   * Program& operator=(const Program&) = delete;
   * Program(Program&&) noexcept;
   * Program& operator=(Program&&) noexcept;
   * ~Program();
   * \endcode
   *
   * \param artifact Compiled program tables used to initialize execution.
   */
  Program();
  explicit Program(detail::ProgramArtifact artifact);
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
   * \brief Accesses the compiled scheduler job plan.
   * \code{.cpp}
   * const detail::JobPlan& job_plan() const noexcept;
   * \endcode
   *
   * \return The scheduler job plan derived from the program data.
   */
  const detail::JobPlan &job_plan() const noexcept;

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
   * \attention 2 The void-pointer overload does not validate the object type.
   * The typed overload rejects a type different from the recorded external.
   */
  void bind_external(std::string_view name, void *pointer);

  template <typename T> void bind_external(std::string_view name, T *pointer) {
    static_assert(!std::is_const_v<T> && !std::is_volatile_v<T>,
                  "Flow external bindings must be mutable objects");
    bind_external_typed(name, static_cast<void *>(pointer), type_id<T>());
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
   * void submit(sync::JobScheduler* scheduler);
   * \endcode
   *
   * \param scheduler Persistent scheduler used for this and later submissions.
   * \attention 1 The program and scheduler must outlive all submitted jobs.
   * \attention 2 Calls to submit and binding operations require external
   * synchronization.
   */
  void submit(sync::JobScheduler *scheduler);

  /**
   * \brief Waits for all outstanding work submitted by this program.
   * \code{.cpp}
   * void wait();
   * \endcode
   *
   * \throws Any exception captured while executing program operations.
   * \attention 1 Calls to wait and submit require external synchronization.
   */
  void wait();

private:
  struct FeedbackSubmissionState {
    explicit FeedbackSubmissionState(std::size_t node_count,
                                     std::size_t resource_count,
                                     std::uint32_t max_in_flight,
                                     std::size_t dependency_capacity,
                                     allocator_ref allocator)
        : jobs{BoxedArray<sync::job_id, allocator_ref>(node_count, allocator),
               BoxedArray<sync::job_id, allocator_ref>(node_count, allocator)},
          job_valid{
              BoxedBitset<std::uint64_t, allocator_ref>(node_count, allocator),
              BoxedBitset<std::uint64_t, allocator_ref>(node_count, allocator)},
          dependencies(dependency_capacity, allocator),
          releases(resource_count, allocator),
          retained_acquire_jobs(resource_count, allocator),
          retained_acquire_valid(resource_count, allocator),
          retained_writer_jobs(resource_count, allocator),
          retained_writer_valid(resource_count, allocator),
          retained_acquisition_counts(resource_count, allocator),
          retained_acquired(resource_count, allocator),
          retained_completion_history(resource_count * max_in_flight,
                                      allocator),
          retained_completion_valid(resource_count * max_in_flight, allocator),
          physical_resources_acquired(resource_count, allocator),
          physical_frontiers_started(resource_count, allocator),
          max_in_flight(max_in_flight) {}

    BoxedArray<sync::job_id, allocator_ref> jobs[2];
    BoxedBitset<std::uint64_t, allocator_ref> job_valid[2];
    BoxedArray<sync::job_id, allocator_ref> dependencies;
    BoxedArray<sync::job_id, allocator_ref> releases;
    BoxedArray<sync::job_id, allocator_ref> retained_acquire_jobs;
    BoxedBitset<std::uint64_t, allocator_ref> retained_acquire_valid;
    BoxedArray<sync::job_id, allocator_ref> retained_writer_jobs;
    BoxedBitset<std::uint64_t, allocator_ref> retained_writer_valid;
    BoxedArray<std::uint64_t, allocator_ref> retained_acquisition_counts;
    BoxedBitset<std::uint64_t, allocator_ref> retained_acquired;
    BoxedArray<sync::job_id, allocator_ref> retained_completion_history;
    BoxedBitset<std::uint64_t, allocator_ref> retained_completion_valid;
    BoxedBitset<std::uint64_t, allocator_ref> physical_resources_acquired;
    BoxedBitset<std::uint64_t, allocator_ref> physical_frontiers_started;
    std::uint32_t max_in_flight = 0;
    std::uint32_t previous_jobs = 0;
  };

  struct SerializationState {
    explicit SerializationState(std::size_t resource_count,
                                allocator_ref allocator)
        : jobs(resource_count, allocator), valid(resource_count, allocator) {}

    BoxedArray<sync::job_id, allocator_ref> jobs;
    BoxedBitset<std::uint64_t, allocator_ref> valid;
  };

  void bind_external_typed(std::string_view name, void *pointer, TypeId type);
  detail::ProgramRuntime &runtime() const;
  void execute_node(std::uint32_t activation, detail::NodeId node, bool prepare,
                    bool force = false);
  bool execute_control_iteration(detail::ControlId control,
                                 std::uint32_t activation,
                                 std::uint32_t invocation);
  void execute_control(detail::ControlId control, std::uint32_t activation);
  void release_after_node(std::uint32_t activation, detail::NodeId node);
  void execute_initialization(std::uint32_t activation);
  void execute_feedback_node(std::uint32_t activation, detail::NodeId node);
  void execute_job(std::uint32_t activation, detail::JobId job);
  void execute_feedback_job(std::uint32_t activation, detail::JobId job);
  void submit_feedback_regions(sync::JobScheduler *scheduler);
  void prune_completions();
  void track_completion(sync::job_id completion);

  detail::ProgramData m_data;
  detail::JobPlan m_job_plan;
  detail::FeedbackSubmissionPlan m_feedback_plan;
  Box<detail::ProgramRuntime, allocator_ref> m_runtime;
  detail::NodeId m_resume_node = 0;
  std::optional<std::uint32_t> m_resume_activation;
  Vector<std::uint64_t, allocator_ref> m_feedback_iterations;
  Vector<std::uint64_t, allocator_ref> m_activation_iterations;
  Vector<FeedbackSubmissionState, allocator_ref> m_feedback_submission;
  std::optional<SerializationState> m_serialization;
  Box<detail::PhysicalStorageState, allocator_ref> m_physical_storage;
  std::optional<BoxedArray<std::uint32_t, allocator_ref>>
      m_activation_storage_slots;
  std::optional<BoxedArray<sync::job_id, allocator_ref>> m_initialization_jobs;
  std::optional<BoxedBitset<std::uint64_t, allocator_ref>>
      m_initialization_job_valid;
  std::optional<sync::job_id> m_initialization_completion;
  bool m_initialization_submitted = false;
  sync::JobScheduler *m_scheduler = nullptr;
  std::array<sync::job_id, sync::JobScheduler::SLOT_COUNT>
      m_pending_completions;
  std::uint32_t m_pending_completion_count = 0;
};

} // namespace strobe::flow
