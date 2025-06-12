#include "../exists.hpp"

#include <unistd.h>

namespace strobe::fs {

bool exists(PathView path) { return (::access(path.c_str(), F_OK) == 0); }

} // namespace strobe::fs
