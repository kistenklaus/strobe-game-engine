#pragma once

#include "logging/print.hpp"

namespace sge {

namespace _logging {

struct Channel {
 public:
  const std::string name;
  boolean enabled;
};

static std::vector<Channel> channels(0);

}  // namespace _logging

struct LogChannel {
 public:
  u32 id;
  bool isActive() const { return _logging::channels[id].enabled; }
};

extern LogChannel constexpr createLogChannel(std::string name, boolean active) {
  _logging::Channel channel = {name, active};
  _logging::channels.push_back(channel);
  return {(u32)(_logging::channels.size() - 1)};
}

template <typename T>
void log(const T& value, LogChannel logChannel) {
  logSn(value, logChannel);
  println();
}
template <typename T>
void logSn(const T& value, LogChannel logChannel) {
  const _logging::Channel& channel = _logging::channels[logChannel.id];
  print(channel.name);
  print(" : ");
  print(value);
}

}  // namespace sge
