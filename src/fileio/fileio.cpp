#include "fileio/fileio.hpp"

#include <fstream>
#include <stdexcept>

namespace sge::fileio {

const std::vector<char> read(const std::string& path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (file) {
    size_t fileSize = static_cast<size_t>(
        file.tellg());  // points to the last element because of ios::ate
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
  } else {
    throw std::runtime_error("RUNTIME-ERROR: coudn't read file [" + path + "]");
  }
}

}  // namespace sge::fileio
