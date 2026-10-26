#pragma once

namespace strobe::rdg {

class Device {
public:
  explicit Device(rhi::Device device) noexcept : m_device(std::move(device)) {}

private:
  rhi::Device m_device;
};

} // namespace strobe::rdg
