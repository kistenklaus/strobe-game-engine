#pragma once
#include <fstream>

#include "strb/stl.hpp"

namespace strb::io {

const strb::vector<char> read(const strb::string &filename) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (file) {
    uint64_t fileSize = static_cast<uint64_t>(
        file.tellg());  // points to the last element because of ios::ate
    strb::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
  } else {
    throw std::runtime_error("RUNTIME-ERROR: coudn't read file [" + filename +
                             "]");
  }
}

}  // namespace strb::io
