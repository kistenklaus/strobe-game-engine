#pragma once

namespace strobe::gpu {

class Queue {
  friend class Device;

public:
  Queue() : m_handle(nullptr) {}
  Queue(const Queue &) noexcept;
  Queue(Queue &&) noexcept;
  Queue &operator=(const Queue &) noexcept;
  Queue &operator=(Queue &&) noexcept;
  ~Queue() noexcept;

  void submit(/* ... */);
  void present(/* ... */);

  explicit operator bool() const { return m_handle != nullptr; }

private:
  explicit Queue(void *handle) : m_handle(handle) {}
  void *m_handle;
};

} // namespace strobe::gpu
