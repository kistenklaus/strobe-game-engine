#pragma once

#include "strobe/core/containers/vector_deque.hpp"
#include "strobe/flow/flow.hpp"

#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

namespace strobe::flow {

template <typename T> struct Channel;

template <typename T> Resource<T> recv(std::string_view tag);

template <typename T> struct Channel {
  friend Resource<T> recv<T>(std::string_view tag);

public:
  Channel(Program *prog, std::string_view name) noexcept
      : m_prog(prog), m_name(name) {
    assert(m_prog != nullptr);
    prog->bind_external(name, this);
  }
  Channel(const Channel &) = delete;
  Channel(Channel &&) = delete;

  void send(T value) {
    ZoneScopedN("Channel::send");
    {
      std::lock_guard lock{m_mutex};
      m_deque.push_front(std::move(value));
    }
    m_prog->release(m_name);
  }

private:
  Program *m_prog;
  std::string m_name;
  std::mutex m_mutex;
  VectorDeque<T> m_deque;
};

template <typename T> Resource<T> recv(std::string_view tag) {
  auto channel = external<Channel<T>>(tag);
  auto element = resource<T>();
  acquire(tag);
  construct(element,
            flow::OpMetadata{"recv", "", flow::OpSemantics::side_effect},
            read_write(channel), [](T *destination, Channel<T> &tx) {
              ZoneScopedN("flow-channel-recv");
              std::lock_guard lock{tx.m_mutex};
              std::construct_at(destination, std::move(tx.m_deque.back()));
              tx.m_deque.pop_back();
            });
  return element;
}

} // namespace strobe::flow
