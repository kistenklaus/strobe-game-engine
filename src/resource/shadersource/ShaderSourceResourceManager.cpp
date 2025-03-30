#include "./ShaderSourceResourceManager.hpp"

#include "logging/Logger.hpp"

namespace strobe::resource {

std::expected<ShaderSource, ShaderSourceError>
ShaderSourceResourceManager::loadShaderSource(const Path& path) {
  STROBE_RESOURCES_INFO("Access ShaderSource \"{}\"", *path);
  auto file = m_fileResourceManager->loadFile(path);
  if (!file) {
    return std::unexpected(
        std::make_unique<ShaderSourceErrorFileError>(std::move(file.error())));
  }
  detail::ShaderSourceControlBlock* controlBlock = m_pool.construct();
  

  return ShaderSource(controlBlock);
}

void ShaderSourceResourceManager::collectGarbage() {}

void ShaderSourceResourceManager::destroy() {}

}  // namespace strobe::resource
