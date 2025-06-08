#pragma once

#include <algorithm>
#include <concepts>
#include <exception>
#include <iostream>
#include <ostream>
#include <semaphore>
#include <thread>
#include <tuple>
#include <type_traits>
#include <stop_token>
#include <thread>

#include "strobe/core/sync/mpsc.hpp"
#include "strobe/window/allocator.hpp"
namespace strobe::window {

class GlfwWindowContext {
 public:
  struct DeferredLambda {
    void* userData;
    void (*func)(void*);
    void operator()() { func(userData); }
  };

  GlfwWindowContext(window::allocator_ref allocator)
      : m_allocator(std::move(allocator)),
        m_jobQueue(
            std::move(strobe::mpsc::channel<DeferredLambda, 10>(m_allocator))),
        m_waitForShutdown(0) {
    std::binary_semaphore initComp{0};
    m_thread = std::jthread(&GlfwWindowContext::glfwMain, this, &initComp);
    initComp.acquire();
  }

  GlfwWindowContext(const GlfwWindowContext&) = delete;
  GlfwWindowContext& operator=(const GlfwWindowContext&) = delete;

  void glfwMain(std::stop_token token, std::binary_semaphore* initComp) {
    if (glfwInit() == GLFW_FALSE) {
      // std::println("FATAL-ERROR: Failed to initalize glfw");
      std::flush(std::cout);
      std::terminate();
      return;
    }
    initComp->release();

    while (!token.stop_requested()) {
      glfwWaitEvents();

      std::optional<DeferredLambda> job;
      while ((job = m_jobQueue.second.recv()).has_value()) {
        (*job)();
      }
    }
    m_waitForShutdown.acquire();
    glfwTerminate();
  }

  ~GlfwWindowContext() {
    m_thread.request_stop();
    // TODO: Here we actually have a small problem because
    // glfwTermiante could have already been executed.
    glfwPostEmptyEvent();
    m_waitForShutdown.release();
    m_thread.join();
  }

  // NOTE: the func is just enqueued it's not executed directly!
  void unsafeExec(void* userData, void (*func)(void*)) {
    auto job = DeferredLambda{
        .userData = userData,
        .func = func,
    };
    if (std::this_thread::get_id() == m_thread.get_id()) {
      job();
    } else {
      while (!m_jobQueue.first.send(job)) {
        glfwPostEmptyEvent();       // <- can be called from any thread.
        std::this_thread::yield();  // <- should essentially never happen.
      }
      glfwPostEmptyEvent();
    }
  }

  template <typename Fn, typename... Args>
    requires std::invocable<Fn, Args...>
  auto exec(Fn&& fn, Args&&... args) -> std::invoke_result_t<Fn, Args...> {
    using return_type = std::invoke_result_t<Fn, Args...>;
    using mock_void =
        std::conditional_t<std::is_void_v<return_type>, int, return_type>;
    struct FnWrapper {
      Fn func;
      std::tuple<std::decay_t<Args>...> args;
      std::binary_semaphore sem;
      std::optional<mock_void> ret;
    };
    FnWrapper wrapper{
        .func = std::forward<Fn>(fn),
        .args = std::make_tuple(std::forward<Args>(args)...),
        .sem = std::binary_semaphore(0),
        .ret = std::nullopt,
    };
    static auto lambda = [](void* userData) {
      auto wrap = static_cast<FnWrapper*>(userData);
      if constexpr (std::is_void_v<return_type>) {
        std::apply(wrap->func, wrap->args);
      } else {
        wrap->ret = std::move(std::apply(wrap->func, wrap->args));
      }
      wrap->sem.release();
    };
    unsafeExec(&wrapper, lambda);
    wrapper.sem.acquire();
    if constexpr (!std::is_void_v<return_type>) {
      assert(wrapper.ret.has_value());
      return std::move(*wrapper.ret);
    }
  }

  window::allocator_ref getAllocator() { return m_allocator; }

 private:
  window::allocator_ref m_allocator;
  std::jthread m_thread;
  std::binary_semaphore m_waitForShutdown;
  std::pair<strobe::mpsc::Sender<DeferredLambda>,
            strobe::mpsc::Receiver<DeferredLambda>>
      m_jobQueue;
};

};  // namespace strobe::window
