#include <strobe/flow/detail/compiler/normalized_ir.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <string>
#include <string_view>

namespace strobe::flow::detail::compiler {

span<NormalizedAccess>
NormalizedIR::accesses_of(NormalizedNode &node) noexcept {
  assert(node.accesses.offset <= accesses.size() &&
         node.accesses.count <= accesses.size() - node.accesses.offset);
  return span<NormalizedAccess>(accesses.data(), accesses.size())
      .subspan(node.accesses.offset, node.accesses.count);
}

span<const NormalizedAccess>
NormalizedIR::accesses_of(const NormalizedNode &node) const noexcept {
  assert(node.accesses.offset <= accesses.size() &&
         node.accesses.count <= accesses.size() - node.accesses.offset);
  return span<const NormalizedAccess>(accesses.data(), accesses.size())
      .subspan(node.accesses.offset, node.accesses.count);
}

span<NormalizedAccess> NormalizedIR::accesses_of(NodeId node) noexcept {
  assert(node < nodes.size());
  return accesses_of(nodes[node]);
}

span<const NormalizedAccess>
NormalizedIR::accesses_of(NodeId node) const noexcept {
  assert(node < nodes.size());
  return accesses_of(nodes[node]);
}

span<ArgumentGroup>
NormalizedIR::argument_groups_of(NormalizedNode &node) noexcept {
  assert(node.argument_groups.offset <= argument_groups.size() &&
         node.argument_groups.count <=
             argument_groups.size() - node.argument_groups.offset);
  return span<ArgumentGroup>(argument_groups.data(), argument_groups.size())
      .subspan(node.argument_groups.offset, node.argument_groups.count);
}

span<const ArgumentGroup>
NormalizedIR::argument_groups_of(const NormalizedNode &node) const noexcept {
  assert(node.argument_groups.offset <= argument_groups.size() &&
         node.argument_groups.count <=
             argument_groups.size() - node.argument_groups.offset);
  return span<const ArgumentGroup>(argument_groups.data(),
                                   argument_groups.size())
      .subspan(node.argument_groups.offset, node.argument_groups.count);
}

span<ArgumentGroup> NormalizedIR::argument_groups_of(NodeId node) noexcept {
  assert(node < nodes.size());
  return argument_groups_of(nodes[node]);
}

span<const ArgumentGroup>
NormalizedIR::argument_groups_of(NodeId node) const noexcept {
  assert(node < nodes.size());
  return argument_groups_of(nodes[node]);
}

span<std::uint32_t>
NormalizedIR::argument_elements_of(NormalizedNode &node) noexcept {
  assert(node.argument_elements.offset <= argument_elements.size() &&
         node.argument_elements.count <=
             argument_elements.size() - node.argument_elements.offset);
  return span<std::uint32_t>(argument_elements.data(), argument_elements.size())
      .subspan(node.argument_elements.offset, node.argument_elements.count);
}

span<const std::uint32_t>
NormalizedIR::argument_elements_of(const NormalizedNode &node) const noexcept {
  assert(node.argument_elements.offset <= argument_elements.size() &&
         node.argument_elements.count <=
             argument_elements.size() - node.argument_elements.offset);
  return span<const std::uint32_t>(argument_elements.data(),
                                   argument_elements.size())
      .subspan(node.argument_elements.offset, node.argument_elements.count);
}

span<std::uint32_t> NormalizedIR::argument_elements_of(NodeId node) noexcept {
  assert(node < nodes.size());
  return argument_elements_of(nodes[node]);
}

span<const std::uint32_t>
NormalizedIR::argument_elements_of(NodeId node) const noexcept {
  assert(node < nodes.size());
  return argument_elements_of(nodes[node]);
}

namespace {

std::string_view name(NodeKind kind) {
  switch (kind) {
  case NodeKind::callback:
    return "op";
  case NodeKind::construct:
    return "construct";
  case NodeKind::cutoff:
    return "cutoff";
  case NodeKind::discard:
    return "discard";
  case NodeKind::acquire:
    return "acquire";
  case NodeKind::yield:
    return "yield";
  case NodeKind::phi:
    return "phi";
  }
  return "unknown";
}

void append_node(String<allocator_ref> &result, const NormalizedIR &ir,
                 const NormalizedNode &node) {
  const auto accesses = ir.accesses_of(node);
  const auto first_access = node.kind == NodeKind::construct ? 1U : 0U;

  std::string prefix;
  if (node.kind == NodeKind::construct && !accesses.empty()) {
    fmt::format_to(std::back_inserter(prefix), "%{} = {}",
                   accesses.front().resource, name(node.kind));
  } else {
    fmt::format_to(std::back_inserter(prefix), "{}", name(node.kind));
  }
  if (first_access < accesses.size()) {
    prefix.push_back(' ');
  }
  for (auto index = first_access; index < accesses.size(); ++index) {
    if (index != first_access) {
      prefix.append(", ");
    }
    const auto &access = accesses[index];
    if (access.purpose == AccessPurpose::serialization) {
      fmt::format_to(std::back_inserter(prefix), "s:%{}", access.resource);
    } else {
      fmt::format_to(std::back_inserter(prefix), "{}:%{}", access.mode,
                     access.resource);
    }
  }

  fmt::format_to(std::back_inserter(result), "{:<50}", prefix);
  if (node.intrinsic) {
    fmt::format_to(std::back_inserter(result), "({})", *node.intrinsic);
  } else {
    fmt::format_to(std::back_inserter(result), "(<source-op:{}>)",
                   node.source_operation);
  }
  if (node.semantics != OpSemantics::none) {
    fmt::format_to(std::back_inserter(result), " [\"\", {}]", node.semantics);
  }
}

} // namespace

String<allocator_ref> to_pretty(const NormalizedIR &ir) {
  String<allocator_ref> result(allocator_ref{});
  bool first_line = true;
  const auto begin_line = [&] {
    if (!first_line) {
      result.push_back('\n');
    }
    first_line = false;
  };

  for (const auto &external : ir.externals) {
    begin_line();
    fmt::format_to(std::back_inserter(result), "%{} = external #{} : {}",
                   external.resource, external.source_external,
                   ir.resources[external.resource].type_name);
  }

  SourceOperationId final_position = 0;
  for (const auto &node : ir.nodes) {
    if (node.source_operation != null_source_operation_id) {
      final_position =
          std::max(final_position,
                   static_cast<SourceOperationId>(node.source_operation + 1));
    }
  }
  for (const auto &phi : ir.phis) {
    final_position = std::max(final_position, phi.declaration_position);
    final_position = std::max(final_position, phi.feedback_position);
  }

  for (SourceOperationId position = 0; position <= final_position; ++position) {
    for (const auto &phi : ir.phis) {
      if (phi.feedback_position != position) {
        continue;
      }
      begin_line();
      auto prefix =
          fmt::format("feedback %{} <- %{}", phi.output, phi.feedback);
      if (phi.condition) {
        fmt::format_to(std::back_inserter(prefix), " if %{}", *phi.condition);
      }
      fmt::format_to(std::back_inserter(result), "{:<50}[max_in_flight={}]",
                     prefix, phi.max_in_flight);
    }

    for (const auto &phi : ir.phis) {
      if (phi.declaration_position != position) {
        continue;
      }
      begin_line();
      fmt::format_to(std::back_inserter(result), "%{} = phi %{}", phi.output,
                     phi.initial);
    }

    for (const auto &node : ir.nodes) {
      if (node.source_operation != position) {
        continue;
      }
      begin_line();
      append_node(result, ir, node);
    }
  }
  return result;
}

} // namespace strobe::flow::detail::compiler
