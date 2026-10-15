#pragma once

#include "strobe/flow/detail/compiler/resolved_ir.hpp"

#include <stdexcept>
#include <utility>

namespace strobe::flow::detail::compiler {

inline ResolvedIR resolve_phis(ValueIR values) {
  ResolvedIR result;
  result.values = std::move(values);
  result.phi_edges.reserve(result.values.phis.size() * 2);
  result.backedges.reserve(result.values.phis.size());

  for (const auto &phi : result.values.phis) {
    if (phi.node >= result.values.nodes.size() ||
        result.values.nodes[phi.node].kind != NodeKind::phi) {
      throw std::logic_error("phi node is invalid");
    }
    if (phi.initial >= result.values.values.size() ||
        phi.feedback >= result.values.values.size() ||
        phi.output >= result.values.values.size()) {
      throw std::logic_error("phi static value is out of range");
    }
    if (result.values.values[phi.output].producer != phi.node) {
      throw std::logic_error("phi output producer is inconsistent");
    }
    if (phi.condition != null_value_id) {
      if (phi.declaration_position >= phi.feedback_position) {
        throw std::logic_error("while interval is empty");
      }
      if (phi.condition >= result.values.values.size()) {
        throw std::logic_error("while condition is out of range");
      }
      const auto condition_resource =
          result.values.values[phi.condition].resource;
      if (condition_resource >= result.values.resources.size() ||
          result.values.resources[condition_resource].type != type_id<bool>()) {
        throw std::logic_error("while condition is not boolean");
      }
      const auto producer = result.values.values[phi.condition].producer;
      if (producer == null_node_id || producer >= result.values.nodes.size()) {
        throw std::logic_error(
            "while requires a condition produced in its body");
      }
      const auto position = result.values.nodes[producer].source_operation;
      if (position < phi.declaration_position ||
          position >= phi.feedback_position) {
        throw std::logic_error("while condition is produced outside its body");
      }
    }

    result.phi_edges.push_back(PhiEdge{
        .value = phi.initial,
        .phi = phi.node,
        .kind = PhiEdgeKind::initial,
    });
    result.phi_edges.push_back(PhiEdge{
        .value = phi.feedback,
        .phi = phi.node,
        .kind = PhiEdgeKind::feedback,
    });
    result.backedges.push_back(Backedge{
        .value = phi.feedback,
        .condition = phi.condition,
        .target = phi.node,
    });
  }

  for (std::size_t left = 0; left < result.values.phis.size(); ++left) {
    const auto &loop = result.values.phis[left];
    bool representative = true;
    for (std::size_t earlier = 0; earlier < left; ++earlier) {
      representative &= result.values.phis[earlier].declaration_position !=
                            loop.declaration_position ||
                        result.values.phis[earlier].feedback_position !=
                            loop.feedback_position;
    }
    if (!representative) {
      continue;
    }

    for (const auto &peer : result.values.phis) {
      if (peer.declaration_position != loop.declaration_position ||
          peer.feedback_position != loop.feedback_position) {
        continue;
      }
      if (peer.condition != loop.condition) {
        throw std::logic_error(
            "feedbacks for one loop must use the same condition");
      }
    }

    for (std::size_t right = left + 1; right < result.values.phis.size();
         ++right) {
      const auto &other = result.values.phis[right];
      if (other.declaration_position == loop.declaration_position &&
          other.feedback_position == loop.feedback_position) {
        continue;
      }
      if (other.declaration_position == loop.declaration_position ||
          other.feedback_position == loop.feedback_position) {
        throw std::logic_error(
            "nested feedback loops must have distinct boundaries");
      }
      const bool crosses =
          (loop.declaration_position < other.declaration_position &&
           other.declaration_position < loop.feedback_position &&
           loop.feedback_position < other.feedback_position) ||
          (other.declaration_position < loop.declaration_position &&
           loop.declaration_position < other.feedback_position &&
           other.feedback_position < loop.feedback_position);
      if (crosses) {
        throw std::logic_error("feedback loop intervals cannot overlap");
      }
    }
  }

  return result;
}

inline void validate_feedback_loops(const ResolvedIR &resolved) {
  const auto &values = resolved.values;
  for (std::size_t index = 0; index < values.phis.size(); ++index) {
    const auto &loop = values.phis[index];
    bool representative = true;
    for (std::size_t earlier = 0; earlier < index; ++earlier) {
      representative &=
          values.phis[earlier].declaration_position !=
              loop.declaration_position ||
          values.phis[earlier].feedback_position != loop.feedback_position;
    }
    if (!representative) {
      continue;
    }

    bool contains_acquire = false;
    bool contains_loop_acquire = false;
    bool contains_yield = false;
    for (const auto &node : values.nodes) {
      const bool inside = node.source_operation >= loop.declaration_position &&
                          node.source_operation < loop.feedback_position;
      contains_acquire |= node.kind == NodeKind::acquire &&
                          node.source_operation <= loop.declaration_position;
      contains_loop_acquire |= inside && node.kind == NodeKind::acquire;
      contains_yield |= inside && node.kind == NodeKind::yield;
    }
    if (loop.condition == null_value_id && !contains_acquire) {
      throw std::logic_error(
          "unconditional feedback loop requires an acquire operation");
    }
    if (loop.condition != null_value_id &&
        (contains_loop_acquire || contains_yield)) {
      throw std::logic_error("while body cannot contain a submission boundary");
    }
    if (loop.condition == null_value_id) {
      for (const auto &outer : values.phis) {
        if (outer.condition != null_value_id &&
            outer.declaration_position < loop.declaration_position &&
            loop.feedback_position < outer.feedback_position) {
          throw std::logic_error(
              "acquire-driven feedback cannot be nested in while");
        }
      }
    }
  }
}

} // namespace strobe::flow::detail::compiler
