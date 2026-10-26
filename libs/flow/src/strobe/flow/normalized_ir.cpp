#include <strobe/flow/detail/compiler/normalized_ir.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <string_view>

namespace strobe::flow::detail::compiler {
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
  const auto begin = static_cast<std::size_t>(node.accesses.offset);
  const auto end = begin + node.accesses.count;
  const auto first_access =
      node.kind == NodeKind::construct ? begin + 1 : begin;

  std::string prefix;
  if (node.kind == NodeKind::construct && begin < end) {
    fmt::format_to(std::back_inserter(prefix), "%{} = {}",
                   ir.accesses[begin].resource, name(node.kind));
  } else {
    fmt::format_to(std::back_inserter(prefix), "{}", name(node.kind));
  }
  if (first_access < end) {
    prefix.push_back(' ');
  }
  for (auto index = first_access; index < end; ++index) {
    if (index != first_access) {
      prefix.append(", ");
    }
    const auto &access = ir.accesses[index];
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
                   ir.resources[external.resource].type);
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
