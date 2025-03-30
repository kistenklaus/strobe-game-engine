#pragma once

#include <future>
#include "resource/ResourceManagerControlBlock.hpp"
namespace strobe::resource {

class ResourceManager {
 public:
  static ResourceManager create();

  auto loadFile(const Path& path) {
    return m_controlBlock->fileResourceManager.loadFile(path);
  }

  std::future<void> destroy();

 private:
  ResourceManager(detail::ResourceManagerControlBlock* controlBlock)
      : m_controlBlock(controlBlock) {}

  detail::ResourceManagerControlBlock* m_controlBlock;
};

};  // namespace strobe::resource
