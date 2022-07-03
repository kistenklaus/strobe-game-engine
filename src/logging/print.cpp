#include "logging/print.hpp"

#include <iostream>
namespace sge {

void flushPrint() { std::cout.flush(); }

void println() { print(NL); }
}  // namespace sge
