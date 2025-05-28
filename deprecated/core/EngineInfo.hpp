#pragma once

#include <string>
#include "core/version.hpp"
namespace strobe {

struct EngineInfo {
  std::string name;
  strobe::version version;
};
}
