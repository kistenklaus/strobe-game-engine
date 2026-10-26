#pragma once

#include "strobe/flow/allocator.hpp"
#include "strobe/flow/argument_group.hpp"
#include "strobe/flow/detail/access_info.hpp"
#include "strobe/flow/detail/operation_kind.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/op_source.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/vector.hpp>

#include <fmt/format.h>

#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <string_view>

namespace strobe::flow::detail {

struct Operation {
  OperationKind kind = OperationKind::callback;
  OpMetadata metadata;
  std::optional<ExternalOp> external;
  std::optional<IntrinsicOp> intrinsic;
  Vector<TypeId, allocator_ref> argument_types;
  Vector<AccessMode, allocator_ref> argument_accesses;
  Vector<ArgumentGroup, allocator_ref> argument_groups;
  Vector<std::uint32_t, allocator_ref> argument_elements;
  Vector<AccessInfo, allocator_ref> accesses;
  Vector<std::size_t, allocator_ref> predecessors;
  std::move_only_function<void(Lifetime, span<void *const>)> invoke;
};

} // namespace strobe::flow::detail

namespace fmt {

/**
 * \brief Formats a recorded Flow operation for diagnostics.
 * \code{.cpp}
 * template<>
 * struct formatter<strobe::flow::detail::Operation>;
 * \endcode
 */
template <> struct formatter<strobe::flow::detail::Operation> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(const strobe::flow::detail::Operation &operation,
              FormatContext &context) const {
    using strobe::flow::OpSemantics;
    using strobe::flow::detail::OperationKind;

    const auto first_access =
        operation.kind == OperationKind::construct ? 1u : 0u;
    fmt::memory_buffer prefix;
    auto prefix_out = std::back_inserter(prefix);
    prefix_out = operation.kind == OperationKind::construct &&
                         !operation.accesses.empty()
                     ? fmt::format_to(prefix_out, "%{} = {}",
                                      operation.accesses[0].resource,
                                      operation.kind)
                     : fmt::format_to(prefix_out, "{}", operation.kind);

    if (first_access < operation.accesses.size()) {
      prefix_out = fmt::format_to(prefix_out, " ");
    }
    for (std::size_t index = first_access; index < operation.accesses.size();
         ++index) {
      if (index != first_access) {
        prefix_out = fmt::format_to(prefix_out, ", ");
      }
      prefix_out = fmt::format_to(prefix_out, "{}", operation.accesses[index]);
    }

    auto out = fmt::format_to(
        context.out(), "{:<50}",
        std::string_view(prefix.data(), prefix.size()));
    out = fmt::format_to(out, "(");
    if (operation.external) {
      out = fmt::format_to(out, "{}", *operation.external);
    } else if (operation.intrinsic) {
      out = fmt::format_to(out, "{}", *operation.intrinsic);
    } else {
      out = fmt::format_to(out, "<lambda>");
    }
    out = fmt::format_to(out, ")");
    if (!operation.metadata.title.empty() ||
        operation.metadata.semantics != OpSemantics::none) {
      out = fmt::format_to(out, " {}", operation.metadata);
    }
    return out;
  }
};

} // namespace fmt
