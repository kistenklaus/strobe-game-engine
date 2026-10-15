#include <strobe/flow/compile_context.hpp>

namespace strobe::flow {

CompileContext::CompileContext(CompileContext &&other) noexcept
    : m_entries(std::move(other.m_entries)) {}

CompileContext &CompileContext::operator=(CompileContext &&other) noexcept {
  if (this != &other) {
    clear();
    m_entries = std::move(other.m_entries);
  }
  return *this;
}

CompileContext::~CompileContext() { clear(); }

void CompileContext::clear() noexcept {
  for (auto &entry : m_entries) {
    entry.destroy(entry.value);
    allocator_ref{}.deallocate(entry.value, entry.size, entry.alignment);
  }
  m_entries.clear();
}

CompileContext::Entry *CompileContext::find(std::string_view id) {
  return const_cast<Entry *>(std::as_const(*this).find(id));
}

const CompileContext::Entry *CompileContext::find(std::string_view id) const {
  for (const auto &entry : m_entries) {
    if (entry.id.view() == id) {
      return &entry;
    }
  }
  return nullptr;
}

} // namespace strobe::flow
