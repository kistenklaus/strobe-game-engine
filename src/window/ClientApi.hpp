#pragma once

#include <cassert>
#include <optional>
#include <variant>
namespace strobe::window {

struct ClientApiVulkan {
  constexpr friend bool operator==(const ClientApiVulkan&,
                                   const ClientApiVulkan&) noexcept {
    return true;
  }
};

class ClientApi {
 private:
  using Variants = std::variant<ClientApiVulkan>;

 public:
  ClientApi() : m_variant(std::nullopt) {}
  constexpr ClientApi(const ClientApiVulkan& variant) : m_variant(variant) {}
  const Variants* operator->() const {
    assert(m_variant.has_value());
    return &*m_variant;
  }
  const Variants& operator*() const {
    assert(m_variant.has_value());
    return *m_variant;
  }

  [[maybe_unused]] operator bool() const { return m_variant != std::nullopt; }

  friend inline bool operator==(const ClientApi& lhs, const ClientApi& rhs) {
    return lhs.m_variant == rhs.m_variant;
  }

  friend inline bool operator!=(const ClientApi& lhs, const ClientApi& rhs) {
    return !(lhs == rhs);
  }

 private:
  std::optional<Variants> m_variant;
};

}  // namespace strobe::window
