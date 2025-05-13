#pragma once

#include <memory>
namespace strobe::mpsc {

namespace details {

template <typename T>
struct ChannelState {
};

};  // namespace details

template <typename T>
class Sender {
 public:
 private:
  std::shared_ptr<details::ChannelState<T>> m_state;
};

template <typename T>
class Receiver {
 public:
 private:
  std::shared_ptr<details::ChannelState<T>> m_state;
};

template <typename T, std::size_t Capacity>
static std::pair<Sender<T>, Receiver<T>> channel() {}

}  // namespace strobe::mpsc
