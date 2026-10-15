#include <strobe/flow/op_source.hpp>

namespace strobe::flow {

OpMetadata::OpMetadata()
    : title(allocator_ref{}), description(allocator_ref{}) {}

OpMetadata::OpMetadata(std::string_view title, std::string_view description,
                       OpSemantics semantics)
    : title(title, allocator_ref{}), description(description, allocator_ref{}),
      semantics(semantics) {}

ExternalOp external_op(std::string_view library_id, std::string_view symbol,
                       std::uint32_t abi_version, OpMetadata metadata) {
  return ExternalOp{
      .library_id = String<allocator_ref>(library_id, allocator_ref{}),
      .symbol = String<allocator_ref>(symbol, allocator_ref{}),
      .abi_version = abi_version,
      .metadata = std::move(metadata),
  };
}

} // namespace strobe::flow
