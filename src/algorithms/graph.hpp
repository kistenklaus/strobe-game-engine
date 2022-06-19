#pragma once
#include <vector>

#include "types/inttypes.hpp"

namespace sge {

std::vector<u32> topologicialSort(std::vector<std::vector<u32>>& graph);
}
