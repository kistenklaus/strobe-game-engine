#pragma once

#include "util.hpp"

#include <strobe/flow/argument_span.hpp>

#include <strobe/core/containers/vector.hpp>

#include <cassert>
#include <concepts>
#include <functional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace strobe::rdg::detail {
namespace materializer_detail {

template <typename T> struct callable_arguments;

template <typename Result, typename Class, typename... Args>
struct callable_arguments<Result (Class::*)(Args...) const noexcept> {
  using tuple = std::tuple<Args...>;
};

template <typename Result, typename Class, typename... Args>
struct callable_arguments<Result (Class::*)(Args...) noexcept> {
  using tuple = std::tuple<Args...>;
};

template <typename Result, typename Class, typename... Args>
struct callable_arguments<Result (Class::*)(Args...) const> {
  using tuple = std::tuple<Args...>;
};

template <typename Result, typename Class, typename... Args>
struct callable_arguments<Result (Class::*)(Args...)> {
  using tuple = std::tuple<Args...>;
};

template <typename Parameter> decltype(auto) argument(void *storage) noexcept {
  using Value = std::remove_cv_t<
      std::remove_reference_t<std::remove_pointer_t<Parameter>>>;
  if constexpr (std::is_pointer_v<Parameter>) {
    return static_cast<Parameter>(static_cast<Value *>(storage));
  } else if constexpr (std::is_lvalue_reference_v<Parameter>) {
    return static_cast<Parameter>(*static_cast<Value *>(storage));
  } else {
    return Value(*static_cast<Value *>(storage));
  }
}

template <typename T> struct immediate_span_parameter : std::false_type {};

template <typename T>
struct immediate_span_parameter<span<const T>> : std::true_type {
  using element_type = T;
};

template <typename Parameter>
inline constexpr bool immediate_span_parameter_v =
    immediate_span_parameter<std::remove_cvref_t<Parameter>>::value;

template <typename T> struct argument_span_parameter : std::false_type {};

template <typename T>
struct argument_span_parameter<flow::ArgumentSpan<T>> : std::true_type {
  using element_type = T;
};

template <typename Parameter>
inline constexpr bool argument_span_parameter_v =
    argument_span_parameter<std::remove_cvref_t<Parameter>>::value;

template <typename Parameter>
decltype(auto) materialized_argument(std::size_t index,
                                     span<void *const> immediates,
                                     span<const std::uint32_t> counts,
                                     span<const flow::ArgumentGroup> groups,
                                     span<const std::uint32_t> elements,
                                     span<void *const> arguments) noexcept {
  if constexpr (immediate_span_parameter_v<Parameter>) {
    using Element = typename immediate_span_parameter<
        std::remove_cvref_t<Parameter>>::element_type;
    assert(index < immediates.size());
    return span<const Element>{static_cast<const Element *>(immediates[index]),
                               counts[index]};
  } else if constexpr (argument_span_parameter_v<Parameter>) {
    using Element = typename argument_span_parameter<
        std::remove_cvref_t<Parameter>>::element_type;
    const auto group = groups[index - immediates.size()];
    return flow::detail::ArgumentSpanAccess<Element>::make(
        arguments.data(), elements.data() + group.element_offset,
        group.element_count);
  } else {
    if (index < immediates.size()) {
      if (counts[index] == 0) {
        return argument<Parameter>(nullptr);
      }
      return argument<Parameter>(immediates[index]);
    }
    const auto group = groups[index - immediates.size()];
    if (group.count == 0) {
      return argument<Parameter>(nullptr);
    }
    assert(group.count == 1);
    auto *storage = arguments[group.offset];
    return argument<Parameter>(storage);
  }
}

template <typename Function, typename Tuple, std::size_t... Index>
void invoke(Function &function, span<void *const> immediates,
            span<const std::uint32_t> immediate_counts,
            span<const flow::ArgumentGroup> argument_groups,
            span<const std::uint32_t> argument_elements,
            span<void *const> arguments,
            std::index_sequence<Index...>) noexcept {
  assert(immediates.size() + argument_groups.size() == sizeof...(Index));
  assert(immediate_counts.size() == immediates.size());
  std::invoke(function,
              materialized_argument<std::tuple_element_t<Index, Tuple>>(
                  Index, immediates, immediate_counts, argument_groups,
                  argument_elements, arguments)...);
}

template <typename Parameter> constexpr flow::TypeId parameter_type() {
  using Raw = std::remove_cvref_t<Parameter>;
  if constexpr (immediate_span_parameter_v<Parameter>) {
    using Value = typename immediate_span_parameter<Raw>::element_type;
    return flow::type_id<Value>();
  } else if constexpr (argument_span_parameter_v<Parameter>) {
    using Value = typename argument_span_parameter<Raw>::element_type;
    return flow::type_id<std::remove_cv_t<Value>>();
  } else {
    using Value = std::remove_cv_t<std::remove_pointer_t<Raw>>;
    return flow::type_id<Value>();
  }
}

} // namespace materializer_detail

class IntrinsicMaterializer {
private:
  enum class Source : std::uint8_t { recorded, serialization, global };

  struct Argument {
    Source source;
    flow::TypeId type;
    flow::AccessMode mode;
    flow::ResourceId resource = flow::null_resource_id;
    bool span = false;
    bool nullable = true;
  };

  struct Immediate {
    flow::TypeId type;
    std::size_t size;
    std::size_t alignment;
    bool span;
    bool optional;
  };

  struct Definition {
    std::string_view opcode;
    flow::OpMetadata metadata;
    Vector<Immediate, flow::allocator_ref> immediates;
    Vector<Argument, flow::allocator_ref> arguments;
    std::function<flow::detail::ResolvedOperation(const flow::IntrinsicOp &)>
        operation;
    bool has_global = false;
  };

public:
  class Builder {
  public:
    Builder &metadata(std::string_view title,
                      flow::OpSemantics semantics = flow::OpSemantics::none) {
      m_definition.metadata = flow::OpMetadata(title, {}, semantics);
      return *this;
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    Builder &immediate() {
      if (!m_definition.arguments.empty()) {
        throw std::logic_error(
            "RDG materializer immediates follow resource arguments");
      }
      m_definition.immediates.push_back(Immediate{
          .type = flow::type_id<T>(),
          .size = sizeof(T),
          .alignment = alignof(T),
          .span = false,
          .optional = false,
      });
      return *this;
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    Builder &immediate_span() {
      if (!m_definition.arguments.empty()) {
        throw std::logic_error(
            "RDG materializer immediates follow resource arguments");
      }
      m_definition.immediates.push_back(Immediate{
          .type = flow::type_id<T>(),
          .size = sizeof(T),
          .alignment = alignof(T),
          .span = true,
          .optional = false,
      });
      return *this;
    }

    template <typename T>
      requires std::is_trivially_copyable_v<T>
    Builder &immediate_optional() {
      if (!m_definition.arguments.empty()) {
        throw std::logic_error(
            "RDG materializer immediates follow resource arguments");
      }
      m_definition.immediates.push_back(Immediate{
          .type = flow::type_id<T>(),
          .size = sizeof(T),
          .alignment = alignof(T),
          .span = false,
          .optional = true,
      });
      return *this;
    }

    template <typename T> Builder &arg(flow::AccessMode mode) {
      if (m_definition.has_global) {
        throw std::logic_error("RDG materializer arguments follow globals");
      }
      m_definition.arguments.push_back(
          {Source::recorded, flow::type_id<T>(), mode});
      return *this;
    }

    template <typename T> Builder &arg_nonnull(flow::AccessMode mode) {
      arg<T>(mode);
      m_definition.arguments.back().nullable = false;
      return *this;
    }

    template <typename T> Builder &arg_span(flow::AccessMode mode) {
      if (m_definition.has_global) {
        throw std::logic_error("RDG materializer arguments follow globals");
      }
      m_definition.arguments.push_back(
          {Source::recorded, flow::type_id<T>(), mode,
           flow::null_resource_id, true});
      return *this;
    }

    template <typename T> Builder &arg_span_nonnull(flow::AccessMode mode) {
      arg_span<T>(mode);
      m_definition.arguments.back().nullable = false;
      return *this;
    }

    template <typename T> Builder &serialize() {
      if (m_definition.has_global) {
        throw std::logic_error(
            "RDG materializer serialization follows globals");
      }
      m_definition.arguments.push_back({Source::serialization,
                                        flow::type_id<T>(),
                                        flow::AccessMode::read_write,
                                        flow::null_resource_id, false, false});
      return *this;
    }

    Builder &serialize() {
      if (m_definition.has_global) {
        throw std::logic_error(
            "RDG materializer serialization follows globals");
      }
      m_definition.arguments.push_back({Source::serialization, flow::TypeId{},
                                        flow::AccessMode::read_write,
                                        flow::null_resource_id, false, false});
      return *this;
    }

    template <typename T>
    Builder &global(flow::ResourceId resource, flow::AccessMode mode) {
      m_definition.has_global = true;
      m_definition.arguments.push_back(
          {Source::global, flow::type_id<T>(), mode, resource, false, false});
      return *this;
    }

    template <typename Function>
      requires std::copy_constructible<std::decay_t<Function>>
    void body(Function function) {
      using Stored = std::decay_t<Function>;
      using Tuple = typename materializer_detail::callable_arguments<
          decltype(&Stored::operator())>::tuple;
      validate_body<Tuple>(
          std::make_index_sequence<std::tuple_size_v<Tuple>>{});

      const auto metadata = m_definition.metadata;
      m_definition.operation = [metadata,
                                function = Stored(std::move(function))](
                                   const flow::IntrinsicOp &) mutable {
        flow::detail::ResolvedOperation operation;
        operation.metadata = metadata;
        operation.runtime =
            [function](flow::Lifetime, span<void *const> immediates,
                       span<const std::uint32_t> immediate_counts,
                       span<const flow::ArgumentGroup> argument_groups,
                       span<const std::uint32_t> argument_elements,
                       span<void *const> arguments) mutable noexcept {
              materializer_detail::invoke<Stored, Tuple>(
                  function, immediates, immediate_counts, argument_groups,
                  argument_elements, arguments,
                  std::make_index_sequence<std::tuple_size_v<Tuple>>{});
            };
        return operation;
      };
    }

    void body(flow::ExternalOp external) {
      const auto metadata = m_definition.metadata;
      m_definition.operation = [metadata, external = std::move(external)](
                                   const flow::IntrinsicOp &) mutable {
        flow::detail::ResolvedOperation operation;
        operation.metadata = metadata;
        operation.external = external;
        operation.external->metadata = metadata;
        return operation;
      };
    }

  private:
    friend class IntrinsicMaterializer;
    explicit Builder(Definition &definition) : m_definition(definition) {}

    template <typename Tuple, std::size_t... Index>
    void validate_body(std::index_sequence<Index...>) const {
      Vector<flow::TypeId, flow::allocator_ref> types;
      Vector<bool, flow::allocator_ref> spans;
      Vector<bool, flow::allocator_ref> optionals;
      for (const auto &immediate : m_definition.immediates) {
        types.push_back(immediate.type);
        spans.push_back(immediate.span);
        optionals.push_back(immediate.optional);
      }
      for (const auto &argument : m_definition.arguments) {
        if (argument.source != Source::serialization) {
          types.push_back(argument.type);
          spans.push_back(argument.span);
          optionals.push_back(argument.nullable);
        }
      }
      if (types.size() != sizeof...(Index)) {
        throw std::logic_error(
            "RDG materializer body signature does not match arguments");
      }
      const bool matches =
          ((types[Index] == materializer_detail::parameter_type<
                                std::tuple_element_t<Index, Tuple>>() &&
            spans[Index] ==
                (materializer_detail::immediate_span_parameter_v<
                     std::tuple_element_t<Index, Tuple>> ||
                 materializer_detail::argument_span_parameter_v<
                     std::tuple_element_t<Index, Tuple>>) &&
            (!optionals[Index] || spans[Index] ||
             std::is_pointer_v<std::remove_cvref_t<
                 std::tuple_element_t<Index, Tuple>>>)) &&
           ...);
      if (!matches) {
        throw std::logic_error(
            "RDG materializer body signature does not match arguments");
      }
    }

    Definition &m_definition;
  };

  IntrinsicMaterializer(flow::detail::compiler::NormalizedIR &ir,
                        flow::CompileContext &context, std::string_view domain,
                        std::uint32_t version)
      : m_ir(ir), m_context(context), m_domain(domain), m_version(version) {}

  Builder define(std::string_view opcode) {
    m_definitions.push_back(Definition{
        .opcode = opcode,
        .metadata = flow::OpMetadata{},
        .immediates = Vector<Immediate, flow::allocator_ref>{},
        .arguments = Vector<Argument, flow::allocator_ref>{},
        .operation = {},
        .has_global = false,
    });
    return Builder(m_definitions.back());
  }

  void run();

private:
  flow::detail::compiler::NormalizedIR &m_ir;
  flow::CompileContext &m_context;
  std::string_view m_domain;
  std::uint32_t m_version;
  Vector<Definition, flow::allocator_ref> m_definitions;
};

} // namespace strobe::rdg::detail
