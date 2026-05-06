#pragma once

namespace strobe {

template <typename>
inline constexpr bool dependent_false_v = false;

template <typename...>
inline constexpr bool dependent_false_pack_v = false;


} // namespace strobe
