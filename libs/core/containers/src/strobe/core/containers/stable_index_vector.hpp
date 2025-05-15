#pragma once

#include "strobe/core/containers/vector.hpp"
namespace strobe {

template <typename T, Allocator A>
class StableIndexVector {
 private:
  Vector<T> m_compact;
  Vector<std::size_t> m_idMap;
};

}  // namespace strobe
