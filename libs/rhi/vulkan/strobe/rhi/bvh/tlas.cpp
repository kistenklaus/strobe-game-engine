#include "strobe/rhi/objects/tlas.hpp"
#include "strobe/rhi/bvh/bvh_impl.hpp"
#include "strobe/rhi/handle.hpp"
#include <utility>

namespace strobe::rhi {

Tlas::Tlas(const Tlas & o) noexcept : Object(o.m_handle) {
  if (m_handle) {
    pin_void_handle<BvhImpl>(m_handle);
  }
}

Tlas::Tlas(Tlas && o) noexcept : Object(std::exchange(o.m_handle, nullptr)){

}

Tlas &Tlas::operator=(const Tlas & o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BvhImpl>(o.m_handle);
  }
  unpin_void_handle<BvhImpl>(m_handle);
  return *this;
}

Tlas &Tlas::operator=(Tlas && o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<BvhImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Tlas::~Tlas() noexcept {
  unpin_void_handle<BvhImpl>(m_handle); 
}


} // namespace strobe::rhi
