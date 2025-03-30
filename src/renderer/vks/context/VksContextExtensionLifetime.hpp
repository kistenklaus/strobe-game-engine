#pragma once

#include <vulkan/vulkan.hpp>

namespace strobe::renderer::vks {

class VksContextExtensionLifetime {
 public:
  virtual ~VksContextExtensionLifetime() = default;

  virtual void destroy(vk::Instance instance, const vk::detail::DispatchLoaderDynamic& dldi) = 0;
};

}  // namespace strobe::renderer::vks
