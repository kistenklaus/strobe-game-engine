#pragma once

#include "strobe/rhi/objects/buffer.hpp"
#include "strobe/rhi/objects/image.hpp"
#include "strobe/rhi/objects/tlas.hpp"
#include <variant>

namespace strobe::rhi {

using Resource = std::variant<Buffer, Image, Tlas>;

}
