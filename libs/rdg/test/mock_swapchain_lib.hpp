#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/op_source.hpp"
#include "strobe/rdg/swapchain.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/image_view.hpp"

#include <cstdint>
#include <mutex>

namespace strobe::rdg::test {

enum class MockSwapchainEventKind : std::uint8_t {
  acquire,
  wait,
  present,
};

struct MockSwapchainEvent {
  MockSwapchainEventKind kind;
  std::uint32_t image;
  flow::Lifetime lifetime;
};

class MockSwapchain {
public:
  MockSwapchain() = default;
  MockSwapchain(const MockSwapchain &) = delete;
  MockSwapchain &operator=(const MockSwapchain &) = delete;
  MockSwapchain(MockSwapchain &&) = delete;
  MockSwapchain &operator=(MockSwapchain &&) = delete;

  void add(rhi::Image image, rhi::ImageView view);

  swapchain_runtime runtime() noexcept;

  Vector<MockSwapchainEvent> events() const;

  void clear_events();

  static MockSwapchain &from_token(void *token) noexcept;

  void *acquire(flow::Lifetime lifetime) noexcept;
  rhi::Image image(void *token) const noexcept;
  rhi::ImageView view(void *token) const noexcept;
  void wait(void *token, flow::Lifetime lifetime) noexcept;
  void present(void *token, flow::Lifetime lifetime) noexcept;

private:
  struct Slot {
    MockSwapchain *owner;
    std::uint32_t index;
    rhi::Image image;
    rhi::ImageView view;
    bool acquired;
  };

  Slot &checked_slot(void *token) noexcept;
  const Slot &checked_slot(void *token) const noexcept;

  mutable std::mutex m_mutex;
  Vector<Slot> m_slots;
  Vector<MockSwapchainEvent> m_events;
  std::uint32_t m_next = 0;
  bool m_started = false;
};

flow::Library make_mock_swapchain_lib() noexcept;

} // namespace strobe::rdg::test
