#pragma once
#include "strb/apis/vulkan/vulkan.lib.hpp"
#include "strb/stl.hpp"

namespace strb::vulkan {

enum class VertexInputFormat {
  VEC2 = VK_FORMAT_R32G32_SFLOAT,
  VEC3 = VK_FORMAT_R32G32B32_SFLOAT,
  VEC4 = VK_FORMAT_R32G32B32A32_SFLOAT
};

constexpr static size_t sizeofVertexInputFormat(VertexInputFormat inputFormat) {
  switch (inputFormat) {
  case VertexInputFormat::VEC2:
    return sizeof(float) * 2;
  case VertexInputFormat::VEC3:
    return sizeof(float) * 3;
  case VertexInputFormat::VEC4:
    return sizeof(float) * 4;
  default:
    throw strb::runtime_exception("cannot determine sizeof VertexInputFormat");
  }
}

struct VertexAttributeLayout {
public:
  const uint32_t location;
  const uint32_t offset;
  const VertexInputFormat format;

  VertexAttributeLayout(uint32_t location, uint32_t offset,
                        VertexInputFormat format)
      : location(location), offset(offset), format(format) {}
  inline VkFormat getVkFormat() const { return static_cast<VkFormat>(format); }
};

struct VertexBindingLayout {
public:
  const uint32_t binding;
  uint32_t vertexSize;
  const strb::vector<VertexAttributeLayout> attribLayouts;
  strb::boolean instanced;

  VertexBindingLayout(uint32_t binding,
                      strb::vector<VertexAttributeLayout> attribLayouts,
                      strb::boolean instanced)
      : binding(binding), attribLayouts(attribLayouts), instanced(instanced) {
    vertexSize = 0;
    for (const VertexAttributeLayout &attrib : attribLayouts) {
      vertexSize += sizeofVertexInputFormat(attrib.format);
    }
  }
};

struct VertexLayout {
public:
  strb::vector<VertexBindingLayout> bindingLayouts;
  VertexLayout(const strb::vector<VertexBindingLayout> bindingLayouts)
      : bindingLayouts(bindingLayouts) {}
  const VertexBindingLayout &getVertexBufferLayout(uint32_t binding) {
    for (uint32_t i = 0; i < bindingLayouts.size(); i++) {
      if (bindingLayouts[i].binding == binding) {
        return bindingLayouts[i];
      }
    }
    throw strb::runtime_exception(
        "no VertexBindingLayout for that binding index");
  }
};

} // namespace strb::vulkan
