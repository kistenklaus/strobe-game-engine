#pragma once

#include "strb/core/stl/string.hpp"

namespace strb {

typedef struct runtime_exception {
private:
  const strb::string msg;

public:
  runtime_exception(strb::string msg) { msg = "RUNTIME_EXCEPTION:" + msg; }
  const char *what() const throw() { return msg.c_str(); }
} runtime_exception;

} // namespace strb
