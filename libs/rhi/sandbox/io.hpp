#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace utility {

std::vector<uint32_t> read_spirv(const std::string &path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (!file) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  const std::streamsize size = file.tellg();

  if (size < 0 || (size % sizeof(uint32_t)) != 0) {
    throw std::runtime_error("Invalid SPIR-V file size: " + path);
  }

  file.seekg(0, std::ios::beg);

  std::vector<uint32_t> data(
      static_cast<std::size_t>(size) / sizeof(uint32_t));

  if (!file.read(
          reinterpret_cast<char *>(data.data()),
          size)) {
    throw std::runtime_error("Failed to read file: " + path);
  }

  return data;
}

}
