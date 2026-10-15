#pragma once

#include "strobe/flow/access_mode.hpp"
#include "strobe/flow/allocator.hpp"
#include "strobe/flow/lifetime.hpp"
#include "strobe/flow/type_id.hpp"

#include <strobe/core/containers/string.hpp>

#include <cstdint>
#include <string_view>
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
 *   void* const* arguments;
 *   const TypeId* argument_types;
 *   const AccessMode* argument_accesses;
 * };
 * \endcode
 */
struct ExternalOpContext {
  std::uint32_t abi_version;
  Lifetime lifetime;
  std::uint32_t argument_count;
  void *const *arguments;
  const TypeId *argument_types;
  const AccessMode *argument_accesses;
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
