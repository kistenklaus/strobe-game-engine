#include "strobe/rhi/heap/resource_descriptor_wizard.hpp"
#include "strobe/rhi/handle.hpp"
#include "strobe/rhi/heap/resource_descriptor_heap_impl.hpp"

namespace strobe::rhi {

ResourceDescriptorWizard::~ResourceDescriptorWizard() noexcept {
  if (m_index != std::numeric_limits<uint32_t>::max()) {
    object_handle_ptr<ResourceDescriptorHeapImpl>(m_heap)->layout.free_range(
        m_index, 1);
  }
}
} // namespace strobe::rhi
