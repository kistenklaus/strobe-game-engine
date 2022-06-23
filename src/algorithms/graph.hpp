#pragma once
#include <vector>

#include "types/inttypes.hpp"

namespace sge {

std::vector<u32> topologicialSort(std::vector<std::vector<u32>>& graph);
std::vector<u32> selectiveReverseTopoligicalSort(
    std::vector<std::vector<u32>>& graph, std::vector<u32>& startNodes);
std::vector<u32> selectiveTopoligicalSort(std::vector<std::vector<u32>>& graph,
                                          std::vector<u32>& startNodes);
}  // namespace sge
