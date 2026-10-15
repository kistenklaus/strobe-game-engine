#include <strobe/flow/compiler_plugins.hpp>

namespace strobe::flow {

CompilerPlugins::CompilerPlugins()
    : normalized(allocator_ref{}), values(allocator_ref{}),
      dependencies(allocator_ref{}), ordered(allocator_ref{}),
      graph(allocator_ref{}), storage(allocator_ref{}),
      lifetimes(allocator_ref{}), storage_layout(allocator_ref{}),
      activation_layout(allocator_ref{}), program(allocator_ref{}) {}

} // namespace strobe::flow
