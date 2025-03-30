#include "./ResourceManager.hpp"

#include <condition_variable>
#include <print>

#include "resource/ResourceManagerControlBlock.hpp"

namespace strobe::resource {

using namespace std::chrono_literals;

static constexpr std::chrono::duration GC_INTERVAL = 5s;

static void gcThread(std::stop_token stopToken,
                     detail::ResourceManagerControlBlock* resourceManager,
                     std::promise<void> stopPromise) {
  using namespace std::chrono_literals;

  std::mutex dummyMutex;
  std::condition_variable_any cv;

  while (!stopToken.stop_requested()) {
    std::lock_guard lck{resourceManager->cleanupMutex};
    resourceManager->fileResourceManager.collectGarbage();

    // Stop-aware sleep for up to 1s
    std::unique_lock lock(dummyMutex);
    cv.wait_for(lock, stopToken, 1s,
                [&]() { return stopToken.stop_requested(); });
  }

  resourceManager->shaderSourceResourceManager.destroy();
  resourceManager->fileResourceManager.destroy();

  delete resourceManager;

  stopPromise.set_value();
}

ResourceManager ResourceManager::create() {
  detail::ResourceManagerControlBlock* controlBlock =
      new detail::ResourceManagerControlBlock;

  controlBlock->shaderSourceResourceManager.m_fileResourceManager =
      &controlBlock->fileResourceManager;

  std::promise<void> stopPromise;
  controlBlock->stopFuture = stopPromise.get_future();

  controlBlock->gcThread =
      std::jthread(gcThread, controlBlock, std::move(stopPromise));

  return ResourceManager(controlBlock);
}

std::future<void> ResourceManager::destroy() {
  std::future<void> stopFuture = std::move(m_controlBlock->stopFuture);
  m_controlBlock->gcThread.request_stop();
  m_controlBlock->gcThread.detach();
  return std::move(stopFuture);
}

}  // namespace strobe::resource
