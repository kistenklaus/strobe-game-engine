#include <strobe/flow/flow.hpp>

#include "strobe/flow/detail/compile.hpp"

#include <strobe/core/memory/box.hpp>

namespace strobe::flow {

namespace detail {

struct Recording {
  Box<OpStream, allocator_ref> stream;
  CompileOptions options;
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

void begin(const CompileOptions &options) {
  if (detail::recording.active) {
    throw std::logic_error("flow recording is already active");
  }
  detail::recording.stream = make_box<detail::OpStream>(allocator_ref{});
  detail::recording.options = options;
  detail::recording.plugins = CompilerPlugins{};
  detail::recording.active = true;
}

void begin(const CompilerPlugins &plugins, const CompileOptions &options) {
  if (detail::recording.active) {
    throw std::logic_error("flow recording is already active");
  }
  detail::recording.stream = make_box<detail::OpStream>(allocator_ref{});
  detail::recording.options = options;
  detail::recording.plugins = plugins;
  detail::recording.active = true;
}

void begin(const CompileOptions &options, const CompilerPlugins &plugins) {
  begin(plugins, options);
}

Program end() {
  auto &stream = detail::active_stream();
  stream.validate();
  auto program = detail::compile(stream, detail::recording.options,
                                 detail::recording.plugins);
  detail::recording.active = false;
  return program;
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

} // namespace strobe::flow
