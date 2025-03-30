#pragma once
#include <future>
#include <thread>
#include "resource/file/FileResourceManager.hpp"
#include "resource/shadersource/ShaderSourceResourceManager.hpp"

namespace strobe::resource::detail {

struct ResourceManagerControlBlock {
  FileResourceManager fileResourceManager;
  ShaderSourceResourceManager shaderSourceResourceManager;

  std::future<void> stopFuture;
  std::jthread gcThread;
  std::mutex cleanupMutex;
};

};
