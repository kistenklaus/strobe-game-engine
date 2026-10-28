#pragma once

#include "strobe/flow/flow.hpp"
namespace strobe::flow {

template <typename Fn> void loop(Fn &&fn, uint32_t max_in_flight = 1) {
  auto initial = resource<std::monostate>();
  construct(initial, flow::OpMetadata{"loop-inital"}, [](std::monostate *ptr) {
    ZoneScopedN("loop-inital");
    std::construct_at(ptr);
  });
  auto previous = phi(initial);
  auto next = resource<std::monostate>();
  construct(next, flow::OpMetadata{"loop-next"}, [](std::monostate *ptr) {
    ZoneScopedN("loop-next");
    std::construct_at(ptr);
  });
  fn();
  feedback(previous, next, {.max_in_flight = max_in_flight});
}

} // namespace strobe::flow
