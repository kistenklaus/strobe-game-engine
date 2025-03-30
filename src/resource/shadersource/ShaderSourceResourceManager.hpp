#pragma once

#include <expected>
#include <mutex>
#include <unordered_map>

#include "core/memory/ObjectPool.hpp"
#include "resource/file/FileResourceManager.hpp"
#include "resource/file/Path.hpp"
#include "resource/shadersource/ShaderSource.hpp"
#include "resource/shadersource/ShaderSourceControlBlock.hpp"
#include "resource/shadersource/ShaderSourceError.hpp"

namespace strobe::resource {

class ShaderSourceResourceManager {
 public:
  std::expected<ShaderSource, ShaderSourceError> loadShaderSource(
      const Path& path);

  void collectGarbage();

  void destroy();

  FileResourceManager* m_fileResourceManager;

 private:
  std::mutex m_cacheMutex;
  std::unordered_map<Path, detail::ShaderSourceControlBlock*> m_cache;
  std::size_t m_cacheSize = 0;
  memory::ObjectPool<detail::ShaderSourceControlBlock> m_pool;
};

}  // namespace strobe::resource
