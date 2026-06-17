#include "strobe/ecs/ecs.hpp"
#include "strobe/ecs/universe.hpp"
#include <tracy/Tracy.hpp>

void strobe::ECS::main_thread(std::stop_token stop_token) {
  tracy::SetThreadName("strobe-ecs-main");
  TracySetProgramName("strobe");

  while (!stop_token.stop_requested()) {
    ZoneScopedN("submit-frame");
    {
      ZoneScopedN("submit-tasks");
      m_universe.treg.drain_cmds();
    }
    {
      ZoneScopedN("submit-sr-cmds");
      ecs::drain_cmds(&m_universe.scheduler, &m_universe.sr_domain,
                      ecs::op_scope(ecs::acq_rel(m_universe.sr_location)),
                      m_universe.sreg.cmds(), m_universe.rreg.cmds());
    }
    {
      ZoneScopedN("submit-schedule");
      m_universe.schedule.submit_all(&m_universe.scheduler);
    }
    FrameMark;
  }
}
