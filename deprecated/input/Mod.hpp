#pragma once

#include <cstdint>
namespace strobe::input {

enum class Mod : std::uint8_t {
  SHIFT = 0x1,
  CONTROL = 0x2,
  ALT = 0x4,
  SUPER = 0x8,
  CAPS_LOCK = 0x10,
  NUM_LOCK = 0x20,
};

// Enable bitwise operations for enum class Mod
inline Mod operator|(Mod lhs, Mod rhs) {
  return static_cast<Mod>(static_cast<std::uint8_t>(lhs) |
                          static_cast<std::uint8_t>(rhs));
}

inline Mod operator&(Mod lhs, Mod rhs) {
  return static_cast<Mod>(static_cast<std::uint8_t>(lhs) &
                          static_cast<std::uint8_t>(rhs));
}

inline Mod operator^(Mod lhs, Mod rhs) {
  return static_cast<Mod>(static_cast<std::uint8_t>(lhs) ^
                          static_cast<std::uint8_t>(rhs));
}

inline Mod operator~(Mod m) {
  return static_cast<Mod>(~static_cast<std::uint8_t>(m));
}

// Compound assignment operators (|=, &=, ^=)
inline Mod& operator|=(Mod& lhs, Mod rhs) {
  lhs = lhs | rhs;
  return lhs;
}

inline Mod& operator&=(Mod& lhs, Mod rhs) {
  lhs = lhs & rhs;
  return lhs;
}

inline Mod& operator^=(Mod& lhs, Mod rhs) {
  lhs = lhs ^ rhs;
  return lhs;
}

}  // namespace strobe::input
