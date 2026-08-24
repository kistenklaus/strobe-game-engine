#pragma once

#include "strobe/core/containers/span.hpp"
#include "strobe/gpu/device/aabb.hpp"
#include "strobe/gpu/device/buffer.hpp"
#include "strobe/gpu/device/build_flags.hpp"
#include "strobe/gpu/device/format.hpp"
#include "strobe/gpu/device/geometry_flags.hpp"
#include "strobe/gpu/device/index_type.hpp"
#include <variant>

namespace strobe::gpu {

struct VertexPositionData {
  Buffer buffer{};
  uint64_t offset = 0;
  uint64_t stride = 0;
  Format format = Format::rgb32_float;
  // Accessible vertex range, not the current primitive count.
  uint32_t vertexCount = 0;
};

struct TransformData {
  Buffer buffer{};
  uint64_t offset = 0;
};

struct IndexData {
  Buffer buffer{};
  uint64_t offset = 0;
  IndexType type = IndexType::uint32;
};

struct TriangleGeometryData {
  GeometryFlags flags = GeometryFlags::none;
  uint32_t maxTriangles = 0;
  VertexPositionData positions{};
  std::optional<IndexData> indices = std::nullopt;
  std::optional<TransformData> transform = std::nullopt;
};

struct AabbGeometryData {
  GeometryFlags flags = GeometryFlags::none;
  uint32_t maxAabbs = 0;
  Buffer buffer{};
  uint64_t offset = 0;
  uint64_t stride = sizeof(Aabb);
};

struct BlasInfo {
  BuildFlags flags = BuildFlags::none;
  MemoryUsage memoryUsage = MemoryUsage::automatic;
  std::variant<span<const TriangleGeometryData>, span<const AabbGeometryData>>
      geometries = {};
};

struct BuildRangeInfo {
  uint32_t primitiveCount = 0;
  uint32_t primitiveOffset = 0;
  uint32_t firstVertex = 0;
  uint32_t transformOffset = 0;
};

struct BuildInfo {
  span<BuildRangeInfo> buildRanges = {};
};

struct Blas {
  friend class Device;
  friend class MemoryPool;
  friend class CommandBuffer;
  friend struct CommandBufferImpl;

public:
  Blas() noexcept : m_handle(nullptr) {}
  Blas(const Blas &) noexcept;
  Blas(Blas &&) noexcept;
  Blas &operator=(const Blas &) noexcept;
  Blas &operator=(Blas &&) noexcept;
  ~Blas() noexcept;
  explicit operator bool() const noexcept { return m_handle; }
  friend bool operator==(const Blas &lhs, const Blas &rhs) noexcept {
    return lhs.m_handle == rhs.m_handle;
  }
  friend bool operator!=(const Blas &lhs, const Blas &rhs) noexcept {
    return lhs.m_handle != rhs.m_handle;
  }

private:
  Blas(void *handle) noexcept : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
