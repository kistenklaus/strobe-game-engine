#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/compile_context.hpp"
#include "strobe/flow/detail/compiler/activation_layout_ir.hpp"
#include "strobe/flow/detail/compiler/dependency_ir.hpp"
#include "strobe/flow/detail/compiler/graph_ir.hpp"
#include "strobe/flow/detail/compiler/lifetime_ir.hpp"
#include "strobe/flow/detail/compiler/normalized_ir.hpp"
#include "strobe/flow/detail/compiler/ordered_ir.hpp"
#include "strobe/flow/detail/compiler/storage_ir.hpp"
#include "strobe/flow/detail/compiler/storage_layout_ir.hpp"
#include "strobe/flow/detail/compiler/value_ir.hpp"
#include "strobe/flow/detail/program_data.hpp"

#include <strobe/core/containers/vector.hpp>

#include <functional>

namespace strobe::flow {

/**
 * \brief Stores ordered extension passes for stable Flow compiler stages.
 * \code{.cpp}
 * struct CompilerPlugins {
 *   CompilerPlugins();
 *   Vector<std::function<void(detail::compiler::NormalizedIR&,
 * CompileContext&)>, allocator_ref> normalized;
 *   Vector<std::function<void(detail::compiler::ValueIR&, CompileContext&)>,
 *          allocator_ref> values;
 *   Vector<std::function<void(detail::compiler::DependencyIR&,
 * CompileContext&)>, allocator_ref> dependencies;
 *   Vector<std::function<void(detail::compiler::OrderedIR&, CompileContext&)>,
 *          allocator_ref> ordered;
 *   Vector<std::function<void(detail::compiler::GraphIR&, CompileContext&)>,
 *          allocator_ref> graph;
 *   Vector<std::function<void(detail::compiler::StorageIR&, CompileContext&)>,
 *          allocator_ref> storage;
 *   Vector<std::function<void(detail::compiler::LifetimeIR&, CompileContext&)>,
 *          allocator_ref>
 *       lifetimes;
 *   Vector<std::function<void(detail::compiler::StorageLayoutIR&,
 * CompileContext&)>, allocator_ref> storage_layout;
 *   Vector<std::function<void(detail::compiler::ActivationLayoutIR&,
 * CompileContext&)>, allocator_ref> activation_layout;
 *   Vector<std::function<void(detail::ProgramData&, CompileContext&)>,
 * allocator_ref> program;
 * };
 * \endcode
 *
 * Passes at each stage execute in insertion order after the built-in pass
 * produces that IR and before the next built-in pass consumes it.
 *
 * \attention 1 A pass must leave its IR valid for the following compiler
 * stage.
 * \attention 2 A plugin table and its captured state must not be modified
 * concurrently while compilation uses it.
 */
struct CompilerPlugins {
  /**
   * \brief Constructs an empty compiler plugin table.
   * \code{.cpp}
   * CompilerPlugins();
   * \endcode
   */
  CompilerPlugins();

  Vector<
      std::function<void(detail::compiler::NormalizedIR &, CompileContext &)>,
      allocator_ref>
      normalized;
  Vector<std::function<void(detail::compiler::ValueIR &, CompileContext &)>,
         allocator_ref>
      values;
  Vector<
      std::function<void(detail::compiler::DependencyIR &, CompileContext &)>,
      allocator_ref>
      dependencies;
  Vector<std::function<void(detail::compiler::OrderedIR &, CompileContext &)>,
         allocator_ref>
      ordered;
  Vector<std::function<void(detail::compiler::GraphIR &, CompileContext &)>,
         allocator_ref>
      graph;
  Vector<std::function<void(detail::compiler::StorageIR &, CompileContext &)>,
         allocator_ref>
      storage;
  Vector<std::function<void(detail::compiler::LifetimeIR &, CompileContext &)>,
         allocator_ref>
      lifetimes;
  Vector<std::function<void(detail::compiler::StorageLayoutIR &,
                            CompileContext &)>,
         allocator_ref>
      storage_layout;
  Vector<std::function<void(detail::compiler::ActivationLayoutIR &,
                            CompileContext &)>,
         allocator_ref>
      activation_layout;
  Vector<std::function<void(detail::ProgramData &, CompileContext &)>,
         allocator_ref>
      program;
};

} // namespace strobe::flow
