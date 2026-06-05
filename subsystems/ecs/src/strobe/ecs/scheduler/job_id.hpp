#pragma once

#include <cstdint>
namespace strobe::ecs {

class JobScheduler;
class Sequencer;

struct job_id {
  friend class JobScheduler;
  friend class Sequencer;

  friend bool operator==(const job_id& lhs, const job_id& rhs) {
    return rhs.m_index == lhs.m_index && rhs.m_gen == lhs.m_gen;
  }
private:
  uint32_t m_index;
  uint32_t m_gen;
};

}
