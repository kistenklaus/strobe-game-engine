#include "./FileResourceManager.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <print>

#include "logging/Logger.hpp"
#include "resource/file/FileError.hpp"

namespace strobe::resource {

std::expected<File, FileError> strobe::resource::FileResourceManager::loadFile(
    const Path& path) {
  std::lock_guard lck{m_cacheMutex};
  // Check if file is already cached.
  auto it = m_cache.find(path);
  bool isCached = it != m_cache.end();
  if (isCached) {
    it->second->referenceCounter.inc();
    STROBE_RESOURCES_INFO("Access File \"{}\"", *path);
    return File(it->second);
  }
  STROBE_RESOURCES_INFO("Loading File \"{}\"", *path);
  // Map file
  detail::FileControlBlock* controlBlock = m_pool.construct();
  controlBlock->path = path;

  const int fd = open(path->c_str(), O_RDONLY);
  if (fd == -1) {
    auto err = std::make_unique<FileErrorFailedToOpen>(path);
    STROBE_RESOURCES_ERROR("Loading File \"{}\" failed:\n{}", *path,
                           err->message());
    return std::unexpected(std::move(err));
  }

  struct stat st;
  if (fstat(fd, &st) == -1) {
    close(fd);
    auto err = std::make_unique<FileErrorFailedToStat>(path);
    STROBE_RESOURCES_ERROR("Loading File \"{}\" failed:\n{}", *path,
                           err->message());
    return std::unexpected(std::move(err));
  }
  controlBlock->size = static_cast<std::size_t>(st.st_size);

  if (controlBlock->size > 0) {
    const void* data =
        mmap(nullptr, controlBlock->size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (data == MAP_FAILED) {
      auto err = std::make_unique<FileErrorFailedToMap>(path);
      STROBE_RESOURCES_ERROR("Loading File \"{}\" failed:\n{}", *path,
                             err->message());
      return std::unexpected(std::move(err));
    }
    controlBlock->data = reinterpret_cast<const std::byte*>(data);
  }

  m_cache[path] = controlBlock;
  m_cacheSize += controlBlock->size;

  return File(controlBlock);
}

void FileResourceManager::destroyResourceAndControlBlock(
    detail::FileControlBlock* block) {
  STROBE_RESOURCES_INFO("Unload File {}", *block->path);
  if (block->data && block->size > 0) {
    ::munmap(const_cast<std::byte*>(block->data), block->size);
  }
  m_cacheSize -= block->size;
  m_pool.destruct(block);
}

void FileResourceManager::collectGarbage() {
  using namespace std::chrono;

  const auto now = high_resolution_clock::now();
  const auto expirationDelay = 1s;

  std::lock_guard lock{m_cacheMutex};

  for (auto it = m_cache.begin(); it != m_cache.end();) {
    auto& block = it->second;

    const auto lastUsed = block->lastUsed.load();

    bool isUnused = block->referenceCounter.useCount() == 0;
    bool isExpired = (now - lastUsed) > expirationDelay;

    if (isUnused && isExpired) {
      destroyResourceAndControlBlock(it->second);
      it = m_cache.erase(it);  // erase returns next iterator
    } else {
      ++it;
    }
  }
}

void FileResourceManager::destroy() {
  std::lock_guard lock{m_cacheMutex};
  for (const auto& [_, controlBlock] : m_cache) {
    destroyResourceAndControlBlock(controlBlock);
  }
  m_cache.clear();
}
}  // namespace strobe::resource
