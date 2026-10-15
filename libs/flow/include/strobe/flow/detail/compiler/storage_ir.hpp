#pragma once

#include "strobe/flow/detail/compiler/activation_ir.hpp"

#include <strobe/core/containers/vector.hpp>

#include <cstdint>
#include <utility>

namespace strobe::flow::detail::compiler {

enum class StorageClass : std::uint8_t {
  external,
  virtual_value,
  transient,
  persistent,
};

struct ValueStorageInfo {
  StorageClass storage;
  bool retained_by_feedback;
  bool retained_by_memoization;
  bool crosses_activation;
};

struct ResourceStorageInfo {
  StorageClass storage;
  std::size_t size;
  std::size_t alignment;
  void (*destroy)(void *) noexcept;
};

struct StorageIR {
  StorageIR(ActivationIR input, Vector<ValueStorageInfo, allocator_ref> values,
            Vector<ResourceStorageInfo, allocator_ref> resources)
      : input(std::move(input)), values(std::move(values)),
        resources(std::move(resources)) {}

  ActivationIR input;
  Vector<ValueStorageInfo, allocator_ref> values;
  Vector<ResourceStorageInfo, allocator_ref> resources;
};

} // namespace strobe::flow::detail::compiler
