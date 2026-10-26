#include <strobe/flow/op_source.hpp>

#include <stdexcept>

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

IntrinsicOp intrinsic(std::string_view domain, std::string_view opcode,
                      std::uint32_t version) {
  if (domain.empty() || opcode.empty() || version == 0) {
    throw std::invalid_argument(
        "intrinsic requires a domain, opcode, and nonzero version");
  }
  return IntrinsicOp{
      .domain = String<allocator_ref>(domain, allocator_ref{}),
      .opcode = String<allocator_ref>(opcode, allocator_ref{}),
      .version = version,
      .immediates = Vector<IntrinsicImmediate, allocator_ref>(allocator_ref{}),
      .immediate_data = Vector<std::byte, allocator_ref>(allocator_ref{}),
  };
}

} // namespace strobe::flow
