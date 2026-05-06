#pragma once

#include <cstdint>
#include <limits>
namespace strobe::ecs {

class Sequencer;

struct location {
  friend class Sequencer;

private:
  location(void* ptr) : m_ptr(ptr) {}
  void* m_ptr = nullptr;
};

} // namespace strobe::ecs
