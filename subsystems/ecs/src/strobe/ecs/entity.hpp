#pragma once

#include <cstdint>
namespace strobe {

struct Entity {
  uint32_t id() const {
    return m_id;
  }
private:
  uint32_t m_id;
  uint32_t m_gen;

  friend bool operator==(const Entity &a, const Entity &b) {
    return (a.m_id == b.m_id) && (a.m_gen == b.m_gen);
  }
};

} // namespace strobe
