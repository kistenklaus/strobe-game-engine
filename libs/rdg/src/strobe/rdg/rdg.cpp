#include <strobe/rdg/rdg.hpp>

#include <strobe/flow/detail/compiler/ordered_ir.hpp>
#include <strobe/rhi/objects/buffer.hpp>
#include <strobe/rhi/objects/image.hpp>

#include <strobe/core/memory/arc.hpp>

#include <optional>
#include <stdexcept>
#include <utility>

namespace strobe::rdg {

namespace detail {

struct OperationScopes {
  explicit OperationScopes(std::uint32_t operation)
      : operation(operation), scopes(flow::allocator_ref{}) {}

  std::uint32_t operation;
  Vector<rhi::AccessScope, flow::allocator_ref> scopes;
};

struct ScopeTable {
  ScopeTable() : operations(flow::allocator_ref{}) {}

  Vector<OperationScopes, flow::allocator_ref> operations;
};

thread_local Arc<ScopeTable> scope_table;

std::optional<rhi::AccessScope>
scope_for(const ScopeTable *table,
          const flow::detail::compiler::ValueIR &values,
          flow::detail::NodeId node, flow::ResourceId resource) {
  if (table == nullptr || node >= values.nodes.size()) {
    return std::nullopt;
  }
  const auto &value_node = values.nodes[node];
  if (value_node.source_operation ==
      flow::detail::compiler::null_source_operation_id) {
    return std::nullopt;
  }
  const OperationScopes *record = nullptr;
  for (const auto &candidate : table->operations) {
    if (candidate.operation == value_node.source_operation) {
      record = &candidate;
      break;
    }
  }
  if (record == nullptr || record->scopes.size() != value_node.accesses.count) {
    return std::nullopt;
  }
  for (std::uint32_t index = 0; index < value_node.accesses.count; ++index) {
    if (values.accesses[value_node.accesses.offset + index].resource ==
        resource) {
      return record->scopes[index];
    }
  }
  return std::nullopt;
}

flow::CompilerPlugins make_compiler_plugins(flow::CompilerPlugins plugins,
                                            Arc<ScopeTable> scopes) {
  plugins.ordered.emplace_back([scopes = std::move(scopes)](
                                   flow::detail::compiler::OrderedIR &ir,
                                   flow::CompileContext &context) {
    auto &plan =
        context.emplace_plugin_data<BarrierPlan>(barrier_plan_plugin_id);
    const auto &value_ir = ir.dependencies.lowered.resolved.values;
    const auto &resources = value_ir.resources;
    const auto &values = value_ir.values;
    const auto append = [&](flow::detail::NodeId before,
                            flow::detail::NodeId after,
                            flow::ResourceId resource) {
      const auto type = resources[resource].type;
      if (type != flow::type_id<rhi::Buffer>() &&
          type != flow::type_id<rhi::Image>()) {
        return;
      }
      for (const auto &existing : plan.dependencies) {
        if (existing.before == before && existing.after == after &&
            existing.resource == resource) {
          return;
        }
      }
      const auto source = scope_for(scopes.get(), value_ir, before, resource);
      const auto destination =
          scope_for(scopes.get(), value_ir, after, resource);
      BarrierDependency dependency{
          .before = before,
          .after = after,
          .resource = resource,
          .type = type,
          .barrier = {},
      };
      if (source && destination) {
        dependency.source_scope = *source;
        dependency.destination_scope = *destination;
        dependency.barrier = rhi::MemoryBarrier{
            .srcStage = source->stage,
            .srcAccess = source->access,
            .dstStage = destination->stage,
            .dstAccess = destination->access,
        };
        dependency.materialized = true;
      }
      plan.dependencies.push_back(std::move(dependency));
    };
    for (const auto &edge : ir.dependencies.edges) {
      append(edge.producer, edge.consumer, values[edge.value].resource);
    }
    for (const auto &constraint : ir.resource_order) {
      if (constraint.resource != flow::null_resource_id) {
        append(constraint.before, constraint.after, constraint.resource);
      }
    }
  });
  plugins.program.emplace_back(
      [](flow::detail::ProgramData &program, flow::CompileContext &context) {
        auto &plan = context.plugin_data<BarrierPlan>(barrier_plan_plugin_id);
        const auto runtime_node = [&](flow::detail::NodeId source) {
          for (flow::detail::NodeId runtime = 0; runtime < program.nodes.size();
               ++runtime) {
            if (program.nodes[runtime].source == source) {
              return runtime;
            }
          }
          throw std::logic_error(
              "RDG barrier dependency source node was not emitted");
        };
        for (auto &dependency : plan.dependencies) {
          dependency.before = runtime_node(dependency.before);
          dependency.after = runtime_node(dependency.after);
        }
      });
  return plugins;
}

void record_scopes(std::size_t operation, span<const rhi::AccessScope> scopes) {
  if (!scope_table) {
    throw std::logic_error("RDG scoped operation requires rdg::begin()");
  }
  OperationScopes record(static_cast<std::uint32_t>(operation));
  record.scopes.reserve(scopes.size());
  for (const auto scope : scopes) {
    record.scopes.push_back(scope);
  }
  scope_table->operations.push_back(std::move(record));
}

} // namespace detail

BarrierPlan::BarrierPlan() : dependencies(flow::allocator_ref{}) {}

BarrierPlan &barrier_plan(flow::CompileContext &context) {
  return context.plugin_data<BarrierPlan>(barrier_plan_plugin_id);
}

const BarrierPlan &barrier_plan(const flow::CompileContext &context) {
  return context.plugin_data<BarrierPlan>(barrier_plan_plugin_id);
}

BarrierPlan &barrier_plan(flow::detail::ProgramData &program) {
  return barrier_plan(program.plugin_data);
}

const BarrierPlan &barrier_plan(const flow::detail::ProgramData &program) {
  return barrier_plan(program.plugin_data);
}

BarrierPlan &barrier_plan(flow::Program &program) {
  return barrier_plan(program.data());
}

const BarrierPlan &barrier_plan(const flow::Program &program) {
  return barrier_plan(program.data());
}

flow::CompilerPlugins compiler_plugins(flow::CompilerPlugins plugins) {
  return detail::make_compiler_plugins(std::move(plugins), {});
}

void begin(const CompileOptions &options) {
  auto scopes = make_arc<detail::ScopeTable>();
  flow::begin(detail::make_compiler_plugins({}, scopes), options);
  detail::scope_table = std::move(scopes);
}

void begin(const CompilerPlugins &plugins, const CompileOptions &options) {
  auto scopes = make_arc<detail::ScopeTable>();
  flow::begin(detail::make_compiler_plugins(plugins, scopes), options);
  detail::scope_table = std::move(scopes);
}

Program end() {
  try {
    auto program = flow::end();
    detail::scope_table.reset();
    return program;
  } catch (...) {
    detail::scope_table.reset();
    throw;
  }
}

} // namespace strobe::rdg
