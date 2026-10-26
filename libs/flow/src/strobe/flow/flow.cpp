#include <strobe/flow/flow.hpp>

#include "strobe/flow/detail/compile.hpp"

#include <strobe/core/memory/box.hpp>

namespace strobe::flow {

namespace detail {

struct Recording {
  Box<OpStream, allocator_ref> stream;
  CompilerPlugins plugins;
  bool active = false;
};

thread_local Recording recording;

OpStream &active_stream() {
  if (!recording.active) {
    throw std::logic_error("flow operation requires begin() before end()");
  }
  return *recording.stream;
}

} // namespace detail

void begin() {
  if (detail::recording.active) {
    throw std::logic_error("flow recording is already active");
  }
  detail::recording.stream = make_box<detail::OpStream>(allocator_ref{});
  detail::recording.plugins = CompilerPlugins{};
  detail::recording.active = true;
}

void begin(const CompilerPlugins &plugins) {
  if (detail::recording.active) {
    throw std::logic_error("flow recording is already active");
  }
  detail::recording.stream = make_box<detail::OpStream>(allocator_ref{});
  detail::recording.plugins = plugins;
  detail::recording.active = true;
}

Program end() {
  auto &stream = detail::active_stream();
  try {
    stream.validate();
    auto program = detail::compile(stream, detail::recording.plugins);
    detail::recording.active = false;
    return program;
  } catch (...) {
    detail::recording.active = false;
    throw;
  }
}

std::size_t yield() { return detail::active_stream().yield(); }

std::size_t acquire(std::string_view name) {
  return detail::active_stream().acquire(name);
}

const Vector<detail::Operation, allocator_ref> &ops() {
  if (!detail::recording.stream) {
    throw std::logic_error("flow has no recording on this thread");
  }
  return detail::recording.stream->ops();
}

String<allocator_ref> to_pretty() {
  if (!detail::recording.stream) {
    throw std::logic_error("flow has no recording on this thread");
  }
  return detail::to_pretty(*detail::recording.stream);
}

} // namespace strobe::flow
