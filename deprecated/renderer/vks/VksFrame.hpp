#pragma once

#include <vector>
#include "renderer/vks/resources/text/TextResource.hpp"
namespace strobe::renderer::vks {

struct VksFrame {

  std::vector<VksTextResource*> drawCommands;

  void reset() {
    drawCommands.clear();
  }

};

};
