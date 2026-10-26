#include "mock_swapchain_lib.hpp"

#include "strobe/rhi/objects/queue.hpp"

#include <cassert>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace strobe::rdg::test {
namespace {

MockSwapchain &swapchain_from(void *storage) noexcept {
  auto *runtime = static_cast<swapchain_runtime *>(storage);
  if (runtime == nullptr || *runtime == nullptr) {
    std::terminate();
  }
  return *static_cast<MockSwapchain *>(*runtime);
}

void *token_from(void *storage) noexcept {
  auto *runtime = static_cast<swapchain_image_runtime *>(storage);
  if (runtime == nullptr || *runtime == nullptr) {
    std::terminate();
  }
  return *runtime;
}

void acquire_op(const flow::ExternalOpContext &context) noexcept {
  assert(context.argument_count == 2);
  auto &swapchain = swapchain_from(context.arguments[1]);
  std::construct_at(
      static_cast<swapchain_image_runtime *>(context.arguments[0]),
      swapchain.acquire(context.lifetime));
}

void get_image_op(const flow::ExternalOpContext &context) noexcept {
  assert(context.argument_count == 2);
  auto *output = static_cast<rhi::Image *>(context.arguments[0]);
  void *token = token_from(context.arguments[1]);
  auto &owner = MockSwapchain::from_token(token);
  std::construct_at(output, owner.image(token));
}

void get_view_op(const flow::ExternalOpContext &context) noexcept {
  assert(context.argument_count == 2);
  auto *output = static_cast<rhi::ImageView *>(context.arguments[0]);
  void *token = token_from(context.arguments[1]);
  auto &owner = MockSwapchain::from_token(token);
  std::construct_at(output, owner.view(token));
}

void wait_op(const flow::ExternalOpContext &context) noexcept {
  assert(context.argument_count == 2);
  (void)static_cast<rhi::Queue *>(context.arguments[0]);
  void *token = token_from(context.arguments[1]);
  MockSwapchain::from_token(token).wait(token, context.lifetime);
}

void present_op(const flow::ExternalOpContext &context) noexcept {
  assert(context.argument_count == 2);
  (void)static_cast<rhi::Queue *>(context.arguments[0]);
  void *token = token_from(context.arguments[1]);
  MockSwapchain::from_token(token).present(token, context.lifetime);
}

flow::ExternalOpFunction resolve(void *, std::string_view symbol,
                                 std::uint32_t abi) noexcept {
  if (abi != 1) {
    return nullptr;
  }
  if (symbol == "acquire") {
    return acquire_op;
  }
  if (symbol == "get_image") {
    return get_image_op;
  }
  if (symbol == "get_view") {
    return get_view_op;
  }
  if (symbol == "wait") {
    return wait_op;
  }
  if (symbol == "present") {
    return present_op;
  }
  return nullptr;
}

} // namespace

void MockSwapchain::add(rhi::Image image, rhi::ImageView view) {
  std::lock_guard lock(m_mutex);
  if (m_started) {
    throw std::logic_error("cannot add mock swapchain images after acquire");
  }
  const auto index = static_cast<std::uint32_t>(m_slots.size());
  m_slots.push_back(Slot{
      .owner = this,
      .index = index,
      .image = std::move(image),
      .view = std::move(view),
      .acquired = false,
  });
}

swapchain_runtime MockSwapchain::runtime() noexcept { return this; }

Vector<MockSwapchainEvent> MockSwapchain::events() const {
  std::lock_guard lock(m_mutex);
  return m_events;
}

void MockSwapchain::clear_events() {
  std::lock_guard lock(m_mutex);
  m_events.clear();
}

MockSwapchain &MockSwapchain::from_token(void *token) noexcept {
  auto *slot = static_cast<Slot *>(token);
  if (slot == nullptr || slot->owner == nullptr) {
    std::terminate();
  }
  auto &owner = *slot->owner;
  owner.checked_slot(token);
  return owner;
}

void *MockSwapchain::acquire(flow::Lifetime lifetime) noexcept {
  std::lock_guard lock(m_mutex);
  m_started = true;
  if (m_slots.empty()) {
    std::terminate();
  }
  for (std::uint32_t offset = 0; offset < m_slots.size(); ++offset) {
    const auto index = static_cast<std::uint32_t>(
        (m_next + offset) % static_cast<std::uint32_t>(m_slots.size()));
    auto &slot = m_slots[index];
    if (slot.acquired) {
      continue;
    }
    slot.acquired = true;
    m_next = (index + 1) % m_slots.size();
    m_events.push_back({MockSwapchainEventKind::acquire, index, lifetime});
    return &slot;
  }
  std::terminate();
}

rhi::Image MockSwapchain::image(void *token) const noexcept {
  std::lock_guard lock(m_mutex);
  return checked_slot(token).image;
}

rhi::ImageView MockSwapchain::view(void *token) const noexcept {
  std::lock_guard lock(m_mutex);
  return checked_slot(token).view;
}

void MockSwapchain::wait(void *token, flow::Lifetime lifetime) noexcept {
  std::lock_guard lock(m_mutex);
  const auto &slot = checked_slot(token);
  if (!slot.acquired) {
    std::terminate();
  }
  m_events.push_back({MockSwapchainEventKind::wait, slot.index, lifetime});
}

void MockSwapchain::present(void *token, flow::Lifetime lifetime) noexcept {
  std::lock_guard lock(m_mutex);
  auto &slot = checked_slot(token);
  if (!slot.acquired) {
    std::terminate();
  }
  m_events.push_back({MockSwapchainEventKind::present, slot.index, lifetime});
  slot.acquired = false;
}

MockSwapchain::Slot &MockSwapchain::checked_slot(void *token) noexcept {
  auto *slot = static_cast<Slot *>(token);
  if (slot == nullptr || slot->owner != this || slot->index >= m_slots.size() ||
      &m_slots[slot->index] != slot) {
    std::terminate();
  }
  return *slot;
}

const MockSwapchain::Slot &
MockSwapchain::checked_slot(void *token) const noexcept {
  auto *slot = static_cast<const Slot *>(token);
  if (slot == nullptr || slot->owner != this || slot->index >= m_slots.size() ||
      &m_slots[slot->index] != slot) {
    std::terminate();
  }
  return *slot;
}

flow::Library make_mock_swapchain_lib() noexcept {
  return flow::Library{
      .state = nullptr,
      .resolve = resolve,
  };
}

} // namespace strobe::rdg::test
