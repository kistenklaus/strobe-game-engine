#include <strobe/flow/detail/compiler/value_ir.hpp>

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

void append_value(auto out, const ValueIR &ir, ValueId value) {
  const auto &info = ir.values[value];
  fmt::format_to(out, "%{}.{}", info.resource, info.version);
}

void append_access(std::string &prefix, const ValueIR &ir,
                   const ValueAccess &access) {
  auto out = std::back_inserter(prefix);
  if (access.purpose == AccessPurpose::serialization) {
    fmt::format_to(out, "s:%{}", access.resource);
    return;
  }
  if (access.mode == AccessMode::read) {
    fmt::format_to(out, "r:");
    append_value(out, ir, access.input);
    return;
  }
  if (access.mode == AccessMode::consume) {
    fmt::format_to(out, "c:");
    append_value(out, ir, access.input);
    return;
  }

  fmt::format_to(out, "{}:", access.mode);
  if (access.input == null_value_id) {
    fmt::format_to(out, "_");
  } else {
    append_value(out, ir, access.input);
  }
  fmt::format_to(out, "->");
  append_value(out, ir, access.output);
}

void append_node(String<allocator_ref> &result, const ValueIR &ir,
                 const ValueNode &node) {
  const auto begin = static_cast<std::size_t>(node.accesses.offset);
  const auto end = begin + node.accesses.count;
  const auto first_access =
      node.kind == NodeKind::construct ? begin + 1 : begin;

  std::string prefix;
  if (node.kind == NodeKind::discard && begin < end) {
    fmt::format_to(std::back_inserter(prefix), "discard %{}",
                   ir.accesses[begin].resource);
  } else if (node.kind == NodeKind::construct && begin < end) {
    append_value(std::back_inserter(prefix), ir, ir.accesses[begin].output);
    fmt::format_to(std::back_inserter(prefix), " = {}", name(node.kind));
  } else {
    fmt::format_to(std::back_inserter(prefix), "{}", name(node.kind));
  }
  if (node.kind != NodeKind::discard && first_access < end) {
    prefix.push_back(' ');
  }
  for (auto index = node.kind == NodeKind::discard ? end : first_access;
       index < end; ++index) {
    if (index != first_access) {
      prefix.append(", ");
    }
    append_access(prefix, ir, ir.accesses[index]);
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

String<allocator_ref> to_pretty(const ValueIR &ir) {
  String<allocator_ref> result(allocator_ref{});
  bool first_line = true;
  const auto begin_line = [&] {
    if (!first_line) {
      result.push_back('\n');
    }
    first_line = false;
  };

  for (const auto &external : ir.externals) {
    const auto value = std::find_if(
        ir.values.begin(), ir.values.end(), [&](const StaticValue &candidate) {
          return candidate.resource == external.resource &&
                 candidate.flags == StaticValueFlags::external;
        });
    if (value == ir.values.end()) {
      continue;
    }
    begin_line();
    fmt::format_to(std::back_inserter(result), "%{}.{} = external #{} : {}",
                   value->resource, value->version, external.source_external,
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
      std::string prefix = "feedback ";
      append_value(std::back_inserter(prefix), ir, phi.output);
      prefix.append(" <- ");
      append_value(std::back_inserter(prefix), ir, phi.feedback);
      if (phi.condition != null_value_id) {
        prefix.append(" if ");
        append_value(std::back_inserter(prefix), ir, phi.condition);
      }
      fmt::format_to(std::back_inserter(result), "{:<50}[max_in_flight={}]",
                     prefix, phi.max_in_flight);
    }

    for (const auto &phi : ir.phis) {
      if (phi.declaration_position != position) {
        continue;
      }
      begin_line();
      append_value(std::back_inserter(result), ir, phi.output);
      fmt::format_to(std::back_inserter(result), " = phi ");
      append_value(std::back_inserter(result), ir, phi.initial);
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
