#include "strobe/platform.hpp"

#include <GLFW/glfw3.h>

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fmt/ostream.h>
#include <memory>
#include <mutex>
#include <semaphore>
#include <thread>

extern "C" int __real_main(int argc, char **argv);

namespace strobe::platform::detail {

void monitor_entry() noexcept;

void monitor_exit() noexcept;

static void glfw_error_callback(int error, const char *description) noexcept {
  switch (error) {
  case GLFW_NOT_INITIALIZED:
    fmt::println(stderr, "GLFW_NOT_INITIALIZED: {}", description);
    break;
  case GLFW_NO_CURRENT_CONTEXT:
    fmt::println(stderr, "GLFW_NO_CURRENT_CONTEXT: {}", description);
    break;
  case GLFW_INVALID_ENUM:
    fmt::println(stderr, "GLFW_INVALID_ENUM: {}", description);
    break;
  case GLFW_INVALID_VALUE:
    fmt::println(stderr, "GLFW_INVALID_VALUE: {}", description);
    break;
  case GLFW_OUT_OF_MEMORY:
    fmt::println(stderr, "GLFW_OUT_OF_MEMORY: {}", description);
    break;
  case GLFW_API_UNAVAILABLE:
    fmt::println(stderr, "GLFW_API_UNAVAILABLE: {}", description);
    break;
  case GLFW_PLATFORM_ERROR:
    fmt::println(stderr, "GLFW_PLATFORM_ERROR: {}", description);
    break;
  case GLFW_FORMAT_UNAVAILABLE:
    fmt::println(stderr, "GLFW_FORMAT_UNAVAILABLE: {}", description);
    break;
  default:
    fmt::println(stderr, "GLFW_UNKNOWN_ERROR: {}", description);
    break;
  }
  std::terminate();
}

namespace {

constexpr std::size_t request_capacity = 256;

using RequestSemaphore =
    std::counting_semaphore<static_cast<std::ptrdiff_t>(request_capacity)>;

using ExecuteFunction = void (*)(void *userdata);

enum class ExecutorState {
  inactive,
  running,
  closing,
};

struct Request {
  ExecuteFunction execute;
  void *userdata;

  std::atomic<bool> completed{false};
};

std::thread::id g_platformThread;
std::atomic<ExecutorState> g_executorState{
    ExecutorState::inactive,
};
std::array<Request *, request_capacity> g_requests{};
std::mutex g_submissionMutex;
RequestSemaphore g_freeSlots{
    static_cast<std::ptrdiff_t>(request_capacity),
};
RequestSemaphore g_readyRequests{0};
std::atomic<std::size_t> g_admittedSubmitters{0};
std::size_t g_writeIndex = 0;
std::size_t g_readIndex = 0;
bool g_acceptingRequests = false;

[[noreturn]] void invalid_run() noexcept {
  assert(false && "platform::run() called outside its valid lifetime");
  std::terminate();
}

void drain_requests() noexcept {
  while (g_readyRequests.try_acquire()) {
    Request *request = g_requests[g_readIndex];

    assert(request);

    g_requests[g_readIndex] = nullptr;
    g_readIndex = (g_readIndex + 1) % request_capacity;

    g_freeSlots.release();

    request->execute(request->userdata);

    request->completed.store(true, std::memory_order_release);
    request->completed.notify_one();
  }
}

void platform_event_loop() noexcept {
  while (g_executorState.load(std::memory_order_acquire) ==
         ExecutorState::running) {
    drain_requests();
    if (g_executorState.load(std::memory_order_acquire) !=
        ExecutorState::running) {
      break;
    }
    glfwWaitEvents();
  }

  while (g_admittedSubmitters.load(std::memory_order_acquire) != 0) {
    drain_requests();
    if (g_admittedSubmitters.load(std::memory_order_acquire) != 0) {
      glfwWaitEvents();
    }
  }

  drain_requests();
}

void close_executor() noexcept {
  {
    std::lock_guard lock{g_submissionMutex};
    g_acceptingRequests = false;
    g_executorState.store(ExecutorState::closing, std::memory_order_release);
  }

  glfwPostEmptyEvent();
}

int platform_main(int argc, char **argv) {
  assert(g_executorState.load(std::memory_order_relaxed) ==
         ExecutorState::inactive);
  g_platformThread = std::this_thread::get_id();
  glfwSetErrorCallback(glfw_error_callback);
  if (glfwInit() != GLFW_TRUE) {
    return EXIT_FAILURE;
  }
  monitor_entry();
  {
    std::lock_guard lock{g_submissionMutex};
    g_acceptingRequests = true;
  }
  g_executorState.store(ExecutorState::running, std::memory_order_release);
  int applicationResult = EXIT_FAILURE;
  std::thread applicationThread{
      [&]() noexcept {
        applicationResult = __real_main(argc, argv);
        close_executor();
      },
  };
  platform_event_loop();
  applicationThread.join();
  monitor_exit();
  glfwTerminate();
  g_executorState.store(ExecutorState::inactive, std::memory_order_release);
  return applicationResult;
}

} // namespace

void run(ExecuteFunction execute, void *userdata) noexcept {
  assert(execute);
  const ExecutorState state = g_executorState.load(std::memory_order_acquire);
  if (state == ExecutorState::inactive) {
    invalid_run();
  }
  if (std::this_thread::get_id() == g_platformThread) {
    execute(userdata);
    return;
  }
  {
    std::lock_guard lock{g_submissionMutex};
    if (!g_acceptingRequests) {
      invalid_run();
    }
    g_admittedSubmitters.fetch_add(1, std::memory_order_relaxed);
  }
  Request request{
      .execute = execute,
      .userdata = userdata,
  };
  g_freeSlots.acquire();
  {
    std::lock_guard lock{g_submissionMutex};
    assert(g_requests[g_writeIndex] == nullptr);
    g_requests[g_writeIndex] = std::addressof(request);
    g_writeIndex = (g_writeIndex + 1) % request_capacity;
    g_readyRequests.release();
    g_admittedSubmitters.fetch_sub(1, std::memory_order_release);
  }
  glfwPostEmptyEvent();
  request.completed.wait(false, std::memory_order_acquire);
}

} // namespace strobe::platform::detail

extern "C" int __wrap_main(int argc, char **argv) {
  return strobe::platform::detail::platform_main(argc, argv);
}
