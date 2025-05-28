#pragma once

#include <cassert>
#include <ostream>
#include <print>
#include <string_view>

#include "core/AtomicRefCounter.hpp"
#include "renderer/resources.hpp"
#include "renderer/vks/resources/text/TextResource.hpp"
namespace strobe::renderer::vks {

class VksTextResourcePool {
 private:
  struct Allocation {
    VksTextResource resource;
    strobe::AtomicRefCounter<std::uint32_t> pinCounter;
  };

 public:
  // immediately creates a new resource.
  VksTextResource* create(std::string_view text) {
    auto* alloc = new Allocation;
    alloc->resource.text = text;
    alloc->pinCounter.reset();
    return &alloc->resource;
  }

  void pin(ResourceHandle resource) {
    const bool succ = reinterpret_cast<Allocation*>(resource)->pinCounter.inc();
    assert(succ);
  }

  void unpin(ResourceHandle resource) {
    Allocation* alloc = reinterpret_cast<Allocation*>(resource);
    if (alloc->pinCounter.dec()) {
      destroy(&alloc->resource);
    }
  }

  // queues resource for deletion.
  void release(ResourceHandle resource) {
    Allocation* alloc = reinterpret_cast<Allocation*>(resource);
    if (alloc->pinCounter.dec()) {
      destroy(&alloc->resource);
    }
    // enqueue somewhere for deletion.
    // maybe a seperate thread for cleaning up resources.
  }

 private:
  void destroy(VksTextResource* resource) { delete resource; }
};

}  // namespace strobe::renderer::vks
