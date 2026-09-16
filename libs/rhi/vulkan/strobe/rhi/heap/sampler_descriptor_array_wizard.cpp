#include "strobe/rhi/heap/sampler_descriptor_array_wizard.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/sampler_descriptor_heap_impl.hpp"
#include <limits>

namespace strobe::rhi {

SamplerDescriptorArrayWizard::~SamplerDescriptorArrayWizard() noexcept {
  if (m_index != std::numeric_limits<uint32_t>::max()) {
    auto *heap_impl = object_handle_ptr<SamplerDescriptorHeapImpl>(m_heap);
    heap_impl->layout.free_range(m_index, m_infos.size());
  }
}

} // namespace strobe::rhi
