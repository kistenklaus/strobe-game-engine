#include "materializer.hpp"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace strobe::rdg::detail {

void IntrinsicMaterializer::run() {
  using flow::AccessPurpose;

  for (auto &node : m_ir.nodes) {
    if (!node.intrinsic || node.intrinsic->domain.view() != m_domain ||
        node.intrinsic->version != m_version) {
      continue;
    }
    const auto definition =
        std::ranges::find_if(m_definitions, [&](const auto &candidate) {
          return candidate.opcode == node.intrinsic->opcode.view();
        });
    if (definition == m_definitions.end()) {
      continue;
    }
    if (!definition->operation) {
      throw std::logic_error("RDG materializer definition has no body");
    }
    if (node.intrinsic->immediates.size() != definition->immediates.size()) {
      throw std::logic_error("RDG intrinsic immediate count is invalid");
    }

    Vector<flow::detail::compiler::NormalizedAccess, flow::allocator_ref>
        arguments;
    Vector<flow::detail::compiler::NormalizedAccess, flow::allocator_ref>
        serialization;
    Vector<flow::ArgumentGroup, flow::allocator_ref> argument_groups;
    Vector<std::uint32_t, flow::allocator_ref> argument_elements;
    std::uint32_t recorded = 0;
    std::uint32_t recorded_group = 0;
    for (const auto &expected : definition->arguments) {
      if (expected.source == Source::global) {
        if (expected.resource >= m_ir.resources.size() ||
            m_ir.resources[expected.resource].type != expected.type) {
          throw std::logic_error("RDG materializer global is invalid");
        }
        arguments.push_back(
            {expected.resource, expected.mode, AccessPurpose::argument});
        const auto argument = static_cast<std::uint32_t>(arguments.size() - 1);
        const auto element =
            static_cast<std::uint32_t>(argument_elements.size());
        argument_groups.push_back({argument, 1, element, 1});
        argument_elements.push_back(argument);
        continue;
      }
      if (expected.source == Source::recorded &&
          recorded_group >= node.argument_groups.count) {
        throw std::logic_error("RDG intrinsic has too few argument groups");
      }
      const auto group = expected.source == Source::recorded
                             ? m_ir.argument_groups[
                                   node.argument_groups.offset + recorded_group++]
                             : flow::ArgumentGroup{recorded, 1, 0, 0};
      if ((expected.source == Source::recorded && !expected.span &&
           group.element_count != 1) ||
          (expected.source == Source::recorded && !expected.nullable &&
           group.count != group.element_count) ||
          group.offset != recorded ||
          group.count > node.accesses.count - recorded ||
          (expected.source == Source::recorded &&
           (group.element_offset > node.argument_elements.count ||
            group.element_count >
                node.argument_elements.count - group.element_offset))) {
        throw std::logic_error("RDG intrinsic argument group is invalid");
      }
      const auto output_offset = static_cast<std::uint32_t>(arguments.size());
      for (std::uint32_t element = 0; element < group.count; ++element) {
      if (recorded >= node.accesses.count) {
        throw std::logic_error("RDG intrinsic has too few accesses");
      }
      const auto access = m_ir.accesses[node.accesses.offset + recorded++];
      if (access.resource >= m_ir.resources.size() ||
          (expected.type.value != 0 &&
           m_ir.resources[access.resource].type != expected.type)) {
        throw std::logic_error("RDG intrinsic access type is invalid");
      }
      if (expected.source == Source::recorded) {
        if (access.purpose != AccessPurpose::argument ||
            access.mode != expected.mode) {
          throw std::logic_error("RDG intrinsic argument access is invalid");
        }
        arguments.push_back(access);
      } else {
        if (access.purpose != AccessPurpose::serialization) {
          throw std::logic_error(
              "RDG intrinsic serialization access is invalid");
        }
        serialization.push_back(access);
      }
      }
      if (expected.source == Source::recorded) {
        const auto output_element_offset =
            static_cast<std::uint32_t>(argument_elements.size());
        for (std::uint32_t element = 0; element < group.element_count;
             ++element) {
          const auto mapped = m_ir.argument_elements[
              node.argument_elements.offset + group.element_offset + element];
          if (mapped == flow::null_argument_element) {
            if (!expected.nullable) {
              throw std::logic_error("RDG intrinsic argument is null");
            }
            argument_elements.push_back(flow::null_argument_element);
          } else {
            if (mapped < group.offset || mapped >= group.offset + group.count) {
              throw std::logic_error(
                  "RDG intrinsic argument element is invalid");
            }
            argument_elements.push_back(output_offset + mapped - group.offset);
          }
        }
        argument_groups.push_back({output_offset, group.count,
                                   output_element_offset,
                                   group.element_count});
      }
    }
    if (recorded != node.accesses.count ||
        recorded_group != node.argument_groups.count) {
      throw std::logic_error("RDG intrinsic has unconsumed accesses");
    }

    const auto offset = m_ir.accesses.size();
    for (const auto &access : arguments) {
      m_ir.accesses.push_back(access);
    }
    for (const auto &access : serialization) {
      m_ir.accesses.push_back(access);
    }

    auto operation = definition->operation(*node.intrinsic);
    for (std::size_t index = 0; index < definition->immediates.size();
         ++index) {
      const auto &expected = definition->immediates[index];
      const auto &recorded = node.intrinsic->immediates[index];
      const auto expected_count =
          expected.span || expected.optional ? recorded.element_count : 1U;
      if (recorded.type != expected.type ||
          recorded.element_size != expected.size ||
          recorded.element_count != expected_count ||
          (expected.optional && recorded.element_count > 1) ||
          recorded.element_count >
              std::numeric_limits<std::uint32_t>::max() / expected.size ||
          recorded.size != expected.size * recorded.element_count) {
        throw std::logic_error("RDG intrinsic immediate type is invalid");
      }
      const auto offset =
          (operation.immediate_data.size() + expected.alignment - 1) &
          ~(expected.alignment - 1);
      operation.immediate_data.resize(offset + recorded.size);
      if (recorded.size != 0) {
        std::memcpy(operation.immediate_data.data() + offset,
                    node.intrinsic->immediate_data.data() + recorded.offset,
                    recorded.size);
      }
      operation.immediates.push_back({expected.type, offset, recorded.size,
                                      expected.alignment, expected.size,
                                      recorded.element_count});
    }
    for (const auto &argument : definition->arguments) {
      if (argument.source == Source::serialization) {
        continue;
      }
      const auto group = argument_groups[operation.argument_groups.size()];
      for (std::uint32_t element = 0; element < group.count; ++element) {
        operation.argument_types.push_back(argument.type);
        operation.argument_accesses.push_back(argument.mode);
      }
      operation.argument_groups.push_back(group);
    }
    operation.argument_elements = argument_elements;
    const auto group_offset = m_ir.argument_groups.size();
    for (const auto group : argument_groups) {
      m_ir.argument_groups.push_back(group);
    }
    const auto element_offset = m_ir.argument_elements.size();
    for (const auto element : argument_elements) {
      m_ir.argument_elements.push_back(element);
    }
    node.accesses = {.offset = static_cast<std::uint32_t>(offset),
                     .count = static_cast<std::uint32_t>(arguments.size() +
                                                         serialization.size())};
    node.argument_count = static_cast<std::uint32_t>(arguments.size());
    node.argument_groups = {
        .offset = static_cast<std::uint32_t>(group_offset),
        .count = static_cast<std::uint32_t>(argument_groups.size())};
    node.argument_elements = {
        .offset = static_cast<std::uint32_t>(element_offset),
        .count = static_cast<std::uint32_t>(argument_elements.size())};
    node.semantics = definition->metadata.semantics;
    node.resolved_operation = m_context.resolve_operation(std::move(operation));
    node.intrinsic.reset();
  }
}

} // namespace strobe::rdg::detail
