#pragma once

#include <string>

#include "core/EngineInfo.hpp"
#include "core/version.hpp"
namespace strobe {

struct ApplicationInfo {
  std::string name;
  strobe::version version;
  EngineInfo engine;
};

}  // namespace strobe
