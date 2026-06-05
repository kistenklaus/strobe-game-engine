#pragma once

namespace strobe::ecs {

struct Universe;

class lifetime_hook {
public:
  virtual void enter(Universe *scheduler) noexcept = 0;
  virtual void exit(Universe *scheduler) noexcept = 0;

protected:
  lifetime_hook() = default;
  virtual ~lifetime_hook() = default;
};

} // namespace strobe::ecs
