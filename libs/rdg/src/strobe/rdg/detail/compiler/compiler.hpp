#pragma once

#include "strobe/flow/compile_context.hpp"
#include "strobe/flow/detail/compiler/normalized_ir.hpp"

namespace strobe::rdg::detail {

void define_globals(flow::detail::compiler::NormalizedIR &ir,
                    flow::CompileContext &context);
void collect_submissions(flow::detail::compiler::NormalizedIR &ir,
                         flow::CompileContext &context);
void collect_resource_states(flow::detail::compiler::NormalizedIR &ir,
                             flow::CompileContext &context);
void collect_resource_accesses(flow::detail::compiler::NormalizedIR &ir,
                               flow::CompileContext &context);
void derive_device_order(flow::detail::compiler::NormalizedIR &ir,
                         flow::CompileContext &context);
void place_image_barriers(flow::detail::compiler::NormalizedIR &ir,
                          flow::CompileContext &context);
void place_buffer_barriers(flow::detail::compiler::NormalizedIR &ir,
                           flow::CompileContext &context);
void remove_redundant_accesses(flow::detail::compiler::NormalizedIR &ir,
                               flow::CompileContext &context);
void materialize_operations(flow::detail::compiler::NormalizedIR &ir,
                            flow::CompileContext &context);
void lower_swapchain(flow::detail::compiler::NormalizedIR &ir,
                     flow::CompileContext &context);

} // namespace strobe::rdg::detail
