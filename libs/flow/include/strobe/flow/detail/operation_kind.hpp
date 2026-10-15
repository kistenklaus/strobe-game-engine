#pragma once

namespace strobe::flow::detail {

enum class OperationKind {
  callback,
  construct,
  cutoff,
  discard,
  acquire,
  yield,
  phi,
};

} // namespace strobe::flow::detail
