#pragma once

#include "strobe/core/containers/vector.hpp"
#include "strobe/core/lina/vec.hpp"
#include "strobe/platform/video_mode.hpp"

namespace strobe::platform {

namespace detail {
struct MonitorAccess;
}

struct WorkArea {
  ivec2 offset;
  uvec2 extent;
};

struct MonitorInfo {
  ivec2 position;
  WorkArea workArea;
  vec2 contentScale;
  VideoMode videoMode;
};

class Monitor {
public:
  Monitor(const Monitor &) noexcept;
  Monitor(Monitor &&) noexcept;
  Monitor &operator=(const Monitor &) noexcept;
  Monitor &operator=(Monitor &&) noexcept;
  ~Monitor() noexcept;

  static Vector<Monitor> monitors() noexcept;
  static Monitor primary() noexcept;

  [[nodiscard]] bool connected() const noexcept;

  [[nodiscard]] std::optional<std::string> name() const;
  [[nodiscard]] std::optional<ivec2> position() const noexcept;
  [[nodiscard]] std::optional<WorkArea> workarea() const noexcept;
  [[nodiscard]] std::optional<uvec2> physical_size_mm() const noexcept;
  [[nodiscard]] std::optional<vec2> content_scale() const noexcept;
  [[nodiscard]] std::optional<VideoMode> video_mode() const noexcept;
  [[nodiscard]] std::optional<Vector<VideoMode>> supported_video_modes() const;

  [[nodiscard]] std::optional<MonitorInfo> info() const noexcept;

  [[nodiscard]] void *native() const noexcept;

private:
  friend struct detail::MonitorAccess;
  explicit Monitor(void *) noexcept;
  void *m_internals;
};

} // namespace strobe::platform
