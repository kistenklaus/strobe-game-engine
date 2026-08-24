#pragma once

#include <GLFW/glfw3.h>
#include <fmt/printf.h>

#include <array>
#include <atomic>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <new>
#include <semaphore>
#include <thread>
#include <type_traits>
#include <utility>

namespace strobe {

namespace platform_details {

struct MainThreadRequest {
  using ExecuteFunction = void (*)(MainThreadRequest *) noexcept;

  explicit MainThreadRequest(ExecuteFunction execute) noexcept
      : execute(execute) {}

  MainThreadRequest(const MainThreadRequest &) = delete;

  MainThreadRequest &operator=(const MainThreadRequest &) = delete;

  ExecuteFunction execute;

  std::atomic<bool> completed{false};
};

// Value result.
template <typename R, typename Enable = void> class ResultSlot;

template <typename R>
class ResultSlot<
    R, std::enable_if_t<!std::is_void_v<R> && !std::is_reference_v<R>>> {
private:
  using StoredType = std::remove_cv_t<R>;

public:
  ResultSlot() noexcept = default;

  ResultSlot(const ResultSlot &) = delete;

  ResultSlot &operator=(const ResultSlot &) = delete;

  ~ResultSlot() {
    if (m_engaged) {
      std::destroy_at(pointer());
    }
  }

  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    assert(!m_engaged);
    void *storage = static_cast<void *>(std::addressof(m_storage[0]));
    ::new (storage) StoredType(std::invoke(std::forward<Producer>(producer)));
    m_engaged = true;
  }

  R take() noexcept {
    assert(m_engaged);

    if constexpr (std::is_move_constructible_v<StoredType>) {
      return std::move(*pointer());
    } else {
      static_assert(std::is_copy_constructible_v<StoredType>,
                    "The return value must be movable or copyable.");
      return *pointer();
    }
  }

private:
  StoredType *pointer() noexcept {
    return std::launder(reinterpret_cast<StoredType *>(m_storage));
  }

  alignas(StoredType) std::byte m_storage[sizeof(StoredType)];

  bool m_engaged = false;
};

// Lvalue-reference result.
template <typename R>
class ResultSlot<R, std::enable_if_t<std::is_lvalue_reference_v<R>>> {
private:
  using ReferencedType = std::remove_reference_t<R>;

public:
  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    auto &&result = std::invoke(std::forward<Producer>(producer));

    m_value = std::addressof(result);
  }

  R take() const noexcept {
    assert(m_value != nullptr);

    return *m_value;
  }

private:
  ReferencedType *m_value = nullptr;
};

// Rvalue-reference result.
template <typename R>
class ResultSlot<R, std::enable_if_t<std::is_rvalue_reference_v<R>>> {
private:
  using ReferencedType = std::remove_reference_t<R>;

public:
  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    auto &&result = std::invoke(std::forward<Producer>(producer));

    m_value = std::addressof(result);
  }

  R take() const noexcept {
    assert(m_value != nullptr);

    return static_cast<R>(*m_value);
  }

private:
  ReferencedType *m_value = nullptr;
};

// Void result.
template <typename R> class ResultSlot<R, std::enable_if_t<std::is_void_v<R>>> {
public:
  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    std::invoke(std::forward<Producer>(producer));
  }

  void take() const noexcept {}
};

template <typename Fn, typename Result>
class CallableRequest final : public MainThreadRequest {
private:
  using Callable = std::remove_reference_t<Fn>;

public:
  explicit CallableRequest(Fn &&callable) noexcept
      : MainThreadRequest(&CallableRequest::execute_request),
        m_callable(std::addressof(callable)) {}

  CallableRequest(const CallableRequest &) = delete;

  CallableRequest &operator=(const CallableRequest &) = delete;

  decltype(auto) take_result() noexcept { return m_result.take(); }

private:
  static void execute_request(MainThreadRequest *base) noexcept {
    auto &self = *static_cast<CallableRequest *>(base);

    self.m_result.emplace_with([&self]() -> decltype(auto) {
      /*
       * Preserve the value category with which the callable
       * was passed to Platform::run().
       */
      if constexpr (std::is_lvalue_reference_v<Fn>) {
        return std::invoke(*self.m_callable);
      } else {
        return std::invoke(std::move(*self.m_callable));
      }
    });
  }

  Callable *m_callable;

  ResultSlot<Result> m_result;
};

} // namespace platform_details

class Platform final {
private:
  static constexpr std::size_t job_capacity = 256;

  using job_semaphore =
      std::counting_semaphore<static_cast<std::ptrdiff_t>(job_capacity)>;

public:
  // Platform::start() itself must be called from the actual main thread.
  template <typename Fn>
    requires std::invocable<std::decay_t<Fn> &>
  static void start(Fn &&fn) {
    auto &self = instance();
    assert(!self.m_thread.joinable());
    self.m_stopRequested.store(false, std::memory_order_relaxed);
    if (glfwInit() != GLFW_TRUE) {
      return;
    }
    self.m_thread = std::thread([&self, fn = std::decay_t<Fn>(std::forward<Fn>(
                                            fn))]() mutable noexcept {
      std::invoke(fn);
      self.m_stopRequested.store(true, std::memory_order_release);
      glfwPostEmptyEvent();
    });
    self.native_main_impl();
    self.m_thread.join();
    glfwTerminate();
  }

  template <typename Fn>
    requires std::invocable<Fn &&>
  static decltype(auto) run(Fn &&callable) noexcept {
    using Result = std::invoke_result_t<Fn &&>;

    if constexpr (std::is_void_v<Result>) {
      instance().run_impl(std::forward<Fn>(callable));

      return;
    } else {
      return instance().run_impl(std::forward<Fn>(callable));
    }
  }

private:
  Platform() noexcept
      : m_freeSlots(static_cast<std::ptrdiff_t>(job_capacity)), m_readyJobs(0) {
  }

  Platform(const Platform &) = delete;
  Platform &operator=(const Platform &) = delete;

  Platform(Platform &&) = delete;
  Platform &operator=(Platform &&) = delete;

  ~Platform() = default;

  static Platform &instance() noexcept {
    static Platform platform;
    return platform;
  }

  void native_main_impl() noexcept {
    while (!m_stopRequested.load(std::memory_order_acquire)) {
      drain_jobs();
      if (m_stopRequested.load(std::memory_order_acquire)) {
        break;
      }
      glfwWaitEvents();
    }
    while (m_admittedSubmitters.load(std::memory_order_acquire) != 0) {
      drain_jobs();
      if (m_admittedSubmitters.load(std::memory_order_acquire) != 0) {
        glfwWaitEvents();
      }
    }
    drain_jobs();
  }

  template <typename Fn>
    requires std::invocable<Fn &&>
  decltype(auto) run_impl(Fn &&callable) noexcept {
    using Result = std::invoke_result_t<Fn &&>;
    platform_details::CallableRequest<Fn, Result> request{
        std::forward<Fn>(callable)};
    submit_request(request);
    while (!request.completed.load(std::memory_order_acquire)) {
      request.completed.wait(false, std::memory_order_acquire);
    }
    if constexpr (std::is_void_v<Result>) {
      request.take_result();
      return;
    } else {
      return request.take_result();
    }
  }

  void submit_request(platform_details::MainThreadRequest &request) noexcept {
    {
      std::lock_guard lock{m_submissionMutex};
      m_admittedSubmitters.fetch_add(1, std::memory_order_relaxed);
    }
    m_freeSlots.acquire();
    {
      std::lock_guard lock{m_submissionMutex};
      assert(m_jobs[m_writeIndex] == nullptr);
      m_jobs[m_writeIndex] = std::addressof(request);
      m_writeIndex = (m_writeIndex + 1) % job_capacity;
      m_readyJobs.release();
      glfwPostEmptyEvent();
      m_admittedSubmitters.fetch_sub(1, std::memory_order_release);
    }
  }

  void drain_jobs() noexcept {
    while (m_readyJobs.try_acquire()) {
      auto *request = m_jobs[m_readIndex];

      assert(request != nullptr);

      m_jobs[m_readIndex] = nullptr;

      m_readIndex = (m_readIndex + 1) % job_capacity;

      m_freeSlots.release();

      request->execute(request);

      request->completed.store(true, std::memory_order_release);

      request->completed.notify_one();
    }
  }

  std::thread m_thread;
  std::atomic<bool> m_stopRequested;

  std::array<platform_details::MainThreadRequest *, job_capacity> m_jobs{};

  std::mutex m_submissionMutex;

  job_semaphore m_freeSlots;
  job_semaphore m_readyJobs;

  std::atomic<std::size_t> m_admittedSubmitters{0};

  std::size_t m_writeIndex = 0;
  std::size_t m_readIndex = 0;
};

} // namespace strobe
