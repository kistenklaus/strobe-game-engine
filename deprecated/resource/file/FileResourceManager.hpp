#pragma once

#include <expected>
#include <mutex>
#include <unordered_map>

#include "core/memory/ObjectPool.hpp"
#include "resource/file/File.hpp"
#include "resource/file/FileControlBlock.hpp"
#include "resource/file/FileError.hpp"
#include "resource/file/Path.hpp"

namespace strobe::resource {

class FileResourceManager {
 public:
  FileResourceManager() : m_cacheMutex(), m_cache(), m_pool() {}

  std::expected<File, FileError> loadFile(const Path& path);

  void collectGarbage();

  void destroy();

 private:
  void destroyResourceAndControlBlock(detail::FileControlBlock* controlBlock);
  std::mutex m_cacheMutex;
  std::unordered_map<Path, detail::FileControlBlock*> m_cache;
  std::size_t m_cacheSize = 0;
  memory::ObjectPool<detail::FileControlBlock> m_pool;
};

}  // namespace strobe::resource
