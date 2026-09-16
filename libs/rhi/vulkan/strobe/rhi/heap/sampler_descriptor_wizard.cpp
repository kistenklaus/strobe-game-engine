#include "strobe/rhi/heap/sampler_descriptor_wizard.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include <limits>

namespace strobe::rhi {

SamplerDescriptorWizard::~SamplerDescriptorWizard() noexcept {
  if (m_index != std::numeric_limits<uint32_t>::max()) {
    object_handle_ptr<SamplerDescriptorHeapImpl>(m_heap)->layout.free_range(
        m_index, 1);
  }
}

} // namespace strobe::rhi
