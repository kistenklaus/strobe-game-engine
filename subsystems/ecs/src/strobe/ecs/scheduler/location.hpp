#pragma once

namespace strobe::ecs {

class Sequencer;
class Scheduler;

struct null_location_t {};

static constexpr null_location_t null_location = {};

struct location {
  friend class Sequencer;
  friend class Scheduler;

  location(null_location_t) : m_ptr(nullptr) {}

  friend inline bool operator==(const location &loc, null_location_t) {
    return loc.m_ptr == nullptr;
  }
  friend inline bool operator==(null_location_t, const location &loc) {
    return loc.m_ptr == nullptr;
  }
  friend inline bool operator!=(const location &loc, null_location_t) {
    return loc.m_ptr != nullptr;
  }
  friend inline bool operator!=(null_location_t, const location &loc) {
    return loc.m_ptr != nullptr;
  }

private:
  location(void *ptr) : m_ptr(ptr) {}
  void *m_ptr = nullptr;
};

} // namespace strobe::ecs
