#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/argument_group.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/type_id.hpp"

#include <strobe/core/containers/span.hpp>
#include <strobe/core/containers/string.hpp>
#include <strobe/core/containers/vector.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

namespace strobe::flow {

/**
 * \brief Describes semantic properties of an operation.
 * \code{.cpp}
 * enum class OpSemantics : std::uint32_t;
 * \endcode
 */
enum class OpSemantics : std::uint32_t {
  /**
   * \brief Declares no semantics beyond the deterministic operation contract.
   * \code{.cpp}
   * OpSemantics::none
   * \endcode
   */
  none = 0,

  /**
   * \brief Declares an observable effect outside the operation's resources.
   * \code{.cpp}
   * OpSemantics::side_effect
   * \endcode
   */
  side_effect = 1u << 0,
};

/**
 * \brief Combines operation semantic flags.
 * \code{.cpp}
 * constexpr OpSemantics operator|(OpSemantics left,
 *                                 OpSemantics right) noexcept;
 * \endcode
 *
 * \return Combined semantic flags.
 */
constexpr OpSemantics operator|(OpSemantics left, OpSemantics right) noexcept {
  return static_cast<OpSemantics>(static_cast<std::uint32_t>(left) |
                                  static_cast<std::uint32_t>(right));
}

/**
 * \brief Stores user-facing and semantic operation metadata.
 * \code{.cpp}
 * struct OpMetadata {
 *   String<allocator_ref> title;
 *   String<allocator_ref> description;
 *   OpSemantics semantics;
 * };
 * \endcode
 *
 * Compiler plugins must lower the operation and remove its intrinsic marker
 * before program emission. Compilation rejects any remaining intrinsic.
 */
struct OpMetadata {
  /**
   * \brief Constructs operation metadata.
   * \code{.cpp}
   * OpMetadata();
   * OpMetadata(std::string_view title,
   *            std::string_view description = {},
   *            OpSemantics semantics = OpSemantics::none);
   * \endcode
   *
   * \param title User-facing operation title.
   * \param description User-facing operation description.
   * \param semantics Semantic flags.
   * \attention 1 Operations are deterministic with respect to their declared
   * resource inputs and may affect only their declared outputs unless marked
   * with `OpSemantics::side_effect`.
   */
  OpMetadata();
  OpMetadata(std::string_view title, std::string_view description = {},
             OpSemantics semantics = OpSemantics::none);

  String<allocator_ref> title;
  String<allocator_ref> description;
  OpSemantics semantics = OpSemantics::none;
};

/**
 * \brief Identifies an operation supplied by an externally bound library.
 * \code{.cpp}
 * struct ExternalOp {
 *   String<allocator_ref> library_id;
 *   String<allocator_ref> symbol;
 *   std::uint32_t abi_version;
 *   OpMetadata metadata;
 * };
 * \endcode
 */
struct ExternalOp {
  String<allocator_ref> library_id;
  String<allocator_ref> symbol;
  std::uint32_t abi_version = 1;
  OpMetadata metadata;
};

/**
 * \brief Describes one typed immediate embedded in an intrinsic.
 * \code{.cpp}
 * struct IntrinsicImmediate {
 *   TypeId type;
 *   std::uint32_t offset;
 *   std::uint32_t size;
 *   std::uint32_t element_size;
 *   std::uint32_t element_count;
 * };
 * \endcode
 */
struct IntrinsicImmediate {
  TypeId type;
  std::uint32_t offset;
  std::uint32_t size;
  std::uint32_t element_size;
  std::uint32_t element_count;
};

/**
 * \brief Identifies an operation interpreted by compiler plugins.
 * \code{.cpp}
 * struct IntrinsicOp {
 *   String<allocator_ref> domain;
 *   String<allocator_ref> opcode;
 *   std::uint32_t version;
 *   Vector<IntrinsicImmediate, allocator_ref> immediates;
 *   Vector<std::byte, allocator_ref> immediate_data;
 * };
 * \endcode
 */
struct IntrinsicOp {
  /**
   * \brief Appends a typed immediate to the intrinsic.
   * \code{.cpp}
   * template<typename T>
   *   requires std::is_trivially_copyable_v<T>
   * IntrinsicOp& immediate(const T& value) &;
   *
   * template<typename T>
   *   requires std::is_trivially_copyable_v<T>
   * IntrinsicOp&& immediate(const T& value) &&;
   * \endcode
   *
   * \param value Value copied into the intrinsic.
   * \return This intrinsic for chained construction.
   */
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  IntrinsicOp &immediate(const T &value) & {
    append_immediate(value);
    return *this;
  }

  template <typename T>
    requires std::is_trivially_copyable_v<T>
  IntrinsicOp &&immediate(const T &value) && {
    append_immediate(value);
    return std::move(*this);
  }

  /**
   * \brief Appends a homogeneous span of typed immediates.
   * \code{.cpp}
   * template<typename T>
   *   requires std::is_trivially_copyable_v<T>
   * IntrinsicOp& immediate_span(span<const T> values) &;
   *
   * template<typename T>
   *   requires std::is_trivially_copyable_v<T>
   * IntrinsicOp&& immediate_span(span<const T> values) &&;
   * \endcode
   *
   * \param values Values copied into the intrinsic as one immediate entry.
   * \return This intrinsic for chained construction.
   */
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  IntrinsicOp &immediate_span(span<const T> values) & {
    append_immediate_span(values);
    return *this;
  }

  template <typename T>
    requires std::is_trivially_copyable_v<T>
  IntrinsicOp &&immediate_span(span<const T> values) && {
    append_immediate_span(values);
    return std::move(*this);
  }

  /**
   * \brief Returns a typed immediate by value.
   * \code{.cpp}
   * template<typename T>
   *   requires std::is_trivially_copyable_v<T>
   * T immediate(std::size_t index) const;
   * \endcode
   *
   * \param index Zero-based immediate index.
   * \return Copy of the stored immediate.
   * \throws std::out_of_range if the index is absent.
   * \throws std::invalid_argument if the stored type or size differs from T.
   */
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  T immediate(std::size_t index) const {
    if (index >= immediates.size()) {
      throw std::out_of_range("intrinsic immediate index is out of range");
    }
    const auto &entry = immediates[index];
    if (entry.type != type_id<T>() || entry.size != sizeof(T) ||
        entry.element_size != sizeof(T) || entry.element_count != 1 ||
        entry.offset > immediate_data.size() ||
        entry.size > immediate_data.size() - entry.offset) {
      throw std::invalid_argument("intrinsic immediate type does not match");
    }
    T result;
    std::memcpy(&result, immediate_data.data() + entry.offset, sizeof(T));
    return result;
  }

  String<allocator_ref> domain;
  String<allocator_ref> opcode;
  std::uint32_t version = 1;
  Vector<IntrinsicImmediate, allocator_ref> immediates;
  Vector<std::byte, allocator_ref> immediate_data;

private:
  template <typename T> void append_immediate(const T &value) {
    if (sizeof(T) > std::numeric_limits<std::uint32_t>::max() ||
        immediate_data.size() >
            std::numeric_limits<std::uint32_t>::max() - sizeof(T)) {
      throw std::length_error("intrinsic immediate storage is exhausted");
    }
    const auto offset = static_cast<std::uint32_t>(immediate_data.size());
    immediate_data.resize(immediate_data.size() + sizeof(T));
    std::memcpy(immediate_data.data() + offset, &value, sizeof(T));
    immediates.push_back(IntrinsicImmediate{
        .type = type_id<T>(),
        .offset = offset,
        .size = static_cast<std::uint32_t>(sizeof(T)),
        .element_size = static_cast<std::uint32_t>(sizeof(T)),
        .element_count = 1,
    });
  }

  template <typename T> void append_immediate_span(span<const T> values) {
    if (values.size() > std::numeric_limits<std::uint32_t>::max() ||
        values.size() > std::numeric_limits<std::uint32_t>::max() / sizeof(T)) {
      throw std::length_error("intrinsic immediate span is too large");
    }
    const auto size = values.size() * sizeof(T);
    if (immediate_data.size() >
        std::numeric_limits<std::uint32_t>::max() - size) {
      throw std::length_error("intrinsic immediate storage is exhausted");
    }
    const auto offset = static_cast<std::uint32_t>(immediate_data.size());
    immediate_data.resize(immediate_data.size() + size);
    if (!values.empty()) {
      std::memcpy(immediate_data.data() + offset, values.data(), size);
    }
    immediates.push_back(IntrinsicImmediate{
        .type = type_id<T>(),
        .offset = offset,
        .size = static_cast<std::uint32_t>(size),
        .element_size = static_cast<std::uint32_t>(sizeof(T)),
        .element_count = static_cast<std::uint32_t>(values.size()),
    });
  }

};

/**
 * \brief Creates a symbolic compiler intrinsic operation.
 * \code{.cpp}
 * IntrinsicOp intrinsic(std::string_view domain,
 *                       std::string_view opcode,
 *                       std::uint32_t version = 1);
 * \endcode
 *
 * \param domain Namespace of the compiler extension interpreting the operation.
 * \param opcode Operation identifier within the extension domain.
 * \param version Version of the intrinsic contract.
 * \return Symbolic intrinsic operation source.
 * \attention 1 The domain and opcode must not be empty.
 * \attention 2 The version must be greater than zero.
 * \attention 3 A compiler plugin must lower the intrinsic and remove its
 * marker before program emission.
 */
IntrinsicOp intrinsic(std::string_view domain, std::string_view opcode,
                      std::uint32_t version = 1);

/**
 * \brief Creates a symbolic external operation reference.
 * \code{.cpp}
 * ExternalOp external_op(std::string_view library_id,
 *                        std::string_view symbol,
 *                        std::uint32_t abi_version = 1,
 *                        OpMetadata metadata = {});
 * \endcode
 *
 * \param library_id Identifier supplied later to Program::bind_library.
 * \param symbol Symbol resolved from the bound library.
 * \param abi_version Required external operation ABI version.
 * \param metadata Operation metadata.
 * \return External operation reference.
 */
ExternalOp external_op(std::string_view library_id, std::string_view symbol,
                       std::uint32_t abi_version = 1,
                       OpMetadata metadata = OpMetadata{});

/**
 * \brief Describes one invocation through the external operation ABI.
 * \code{.cpp}
 * struct ExternalOpContext {
 *   std::uint32_t abi_version;
 *   Lifetime lifetime;
 *   std::uint32_t argument_count;
 *   std::uint32_t argument_group_count;
 *   std::uint32_t argument_element_count;
 *   std::uint32_t immediate_count;
 *   void* const* arguments;
 *   void* const* immediates;
 *   const TypeId* immediate_types;
 *   const std::uint32_t* immediate_counts;
 *   const TypeId* argument_types;
 *   const AccessMode* argument_accesses;
 *   const ArgumentGroup* argument_groups;
 *   const std::uint32_t* argument_elements;
 * };
 * \endcode
 */
struct ExternalOpContext {
  std::uint32_t abi_version;
  Lifetime lifetime;
  std::uint32_t argument_count;
  std::uint32_t argument_group_count;
  std::uint32_t argument_element_count;
  std::uint32_t immediate_count;
  void *const *arguments;
  void *const *immediates;
  const TypeId *immediate_types;
  const std::uint32_t *immediate_counts;
  const TypeId *argument_types;
  const AccessMode *argument_accesses;
  const ArgumentGroup *argument_groups;
  const std::uint32_t *argument_elements;
};

/**
 * \brief Function signature exported by an external operation library.
 * \code{.cpp}
 * using ExternalOpFunction = void (*)(const ExternalOpContext&) noexcept;
 * \endcode
 */
using ExternalOpFunction = void (*)(const ExternalOpContext &) noexcept;

/**
 * \brief Provides symbol resolution for one externally owned library.
 * \code{.cpp}
 * struct Library {
 *   void* state;
 *   ExternalOpFunction (*resolve)(void* state,
 *                                 std::string_view symbol,
 *                                 std::uint32_t abi_version) noexcept;
 * };
 * \endcode
 *
 * \attention 1 The state and resolver must remain valid until all submitted
 * work using the binding has completed.
 */
struct Library {
  void *state = nullptr;
  ExternalOpFunction (*resolve)(void *, std::string_view,
                                std::uint32_t) noexcept = nullptr;
};

} // namespace strobe::flow

namespace fmt {

/**
 * \brief Formats Flow operation semantic flags.
 * \code{.cpp}
 * template<>
 * struct formatter<strobe::flow::OpSemantics>;
 * \endcode
 */
template <> struct formatter<strobe::flow::OpSemantics> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(strobe::flow::OpSemantics semantics,
              FormatContext &context) const {
    using strobe::flow::OpSemantics;
    if (semantics == OpSemantics::none) {
      return fmt::format_to(context.out(), "none");
    }
    if (semantics == OpSemantics::side_effect) {
      return fmt::format_to(context.out(), "side_effect");
    }
    return fmt::format_to(context.out(), "{}",
                          static_cast<std::uint32_t>(semantics));
  }
};

/**
 * \brief Formats Flow operation metadata for diagnostics.
 * \code{.cpp}
 * template<>
 * struct formatter<strobe::flow::OpMetadata>;
 * \endcode
 */
template <> struct formatter<strobe::flow::OpMetadata> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(const strobe::flow::OpMetadata &metadata,
              FormatContext &context) const {
    if (metadata.semantics == strobe::flow::OpSemantics::none) {
      return fmt::format_to(context.out(), "[\"{}\"]", metadata.title);
    }
    return fmt::format_to(context.out(), "[\"{}\", {}]", metadata.title,
                          metadata.semantics);
  }
};

/**
 * \brief Formats an external Flow operation identity.
 * \code{.cpp}
 * template<>
 * struct formatter<strobe::flow::ExternalOp>;
 * \endcode
 */
template <> struct formatter<strobe::flow::ExternalOp> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(const strobe::flow::ExternalOp &operation,
              FormatContext &context) const {
    return fmt::format_to(context.out(), "{}::{}", operation.library_id,
                          operation.symbol);
  }
};

/**
 * \brief Formats a Flow intrinsic identity.
 * \code{.cpp}
 * template<>
 * struct formatter<strobe::flow::IntrinsicOp>;
 * \endcode
 */
template <> struct formatter<strobe::flow::IntrinsicOp> {
  constexpr auto parse(format_parse_context &context) {
    return context.begin();
  }

  template <typename FormatContext>
  auto format(const strobe::flow::IntrinsicOp &operation,
              FormatContext &context) const {
    return fmt::format_to(context.out(), "{}/{}", operation.domain,
                          operation.opcode);
  }
};

} // namespace fmt
