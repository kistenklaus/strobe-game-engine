#include "strobe/rhi/objects/blas.hpp"
#include "strobe/rhi/memory/blas_impl.hpp"
#include "strobe/rhi/handle.hpp"

namespace strobe::rhi {

Blas::Blas(const Blas &o) noexcept : m_handle(o.m_handle) {
  if (m_handle) {
    pin_void_handle<BlasImpl>(m_handle);
  }
}

Blas::Blas(Blas &&o) noexcept : m_handle(std::exchange(o.m_handle, nullptr)) {}

Blas &Blas::operator=(const Blas &o) noexcept {
  if (this == &o) {
    return *this;
  }
  if (o.m_handle != nullptr) {
    pin_void_handle<BlasImpl>(o.m_handle);
  }
  unpin_void_handle<BlasImpl>(m_handle);
  return *this;
}

Blas &Blas::operator=(Blas &&o) noexcept {
  if (this == &o) {
    return *this;
  }
  unpin_void_handle<BlasImpl>(m_handle);
  m_handle = std::exchange(o.m_handle, nullptr);
  return *this;
}

Blas::~Blas() noexcept { 
  unpin_void_handle<BlasImpl>(m_handle); 
}

} // namespace strobe::rhi
