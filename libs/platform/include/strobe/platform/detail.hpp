#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>

namespace strobe::platform::detail {

void run(void (*fn)(void *userData), void *userData) noexcept;

// -----------------------------------------------------------------------------
// Result storage
// -----------------------------------------------------------------------------

template <typename R, typename Enable = void> class ResultSlot;

// Value result
template <typename R>
class ResultSlot<
    R, std::enable_if_t<!std::is_void_v<R> && !std::is_reference_v<R>>> {
private:
  using StoredType = std::remove_cv_t<R>;

public:
  ResultSlot() noexcept = default;

  ResultSlot(const ResultSlot &) = delete;
  ResultSlot &operator=(const ResultSlot &) = delete;

  ~ResultSlot() {
    if (m_engaged) {
      std::destroy_at(pointer());
    }
  }

  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    assert(!m_engaged);

    ::new (static_cast<void *>(m_storage))
        StoredType(std::invoke(std::forward<Producer>(producer)));

    m_engaged = true;
  }

  R take() noexcept {
    assert(m_engaged);

    if constexpr (std::is_move_constructible_v<StoredType>) {
      return std::move(*pointer());
    } else {
      static_assert(std::is_copy_constructible_v<StoredType>,
                    "The return value must be movable or copyable.");

      return *pointer();
    }
  }

private:
  StoredType *pointer() noexcept {
    return std::launder(reinterpret_cast<StoredType *>(m_storage));
  }

  alignas(StoredType) std::byte m_storage[sizeof(StoredType)];
  bool m_engaged = false;
};

// Lvalue-reference result
template <typename R>
class ResultSlot<R, std::enable_if_t<std::is_lvalue_reference_v<R>>> {
private:
  using ReferencedType = std::remove_reference_t<R>;

public:
  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    auto &&result = std::invoke(std::forward<Producer>(producer));
    m_value = std::addressof(result);
  }

  R take() const noexcept {
    assert(m_value);
    return *m_value;
  }

private:
  ReferencedType *m_value = nullptr;
};

// Rvalue-reference result
template <typename R>
class ResultSlot<R, std::enable_if_t<std::is_rvalue_reference_v<R>>> {
private:
  using ReferencedType = std::remove_reference_t<R>;

public:
  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    auto &&result = std::invoke(std::forward<Producer>(producer));
    m_value = std::addressof(result);
  }

  R take() const noexcept {
    assert(m_value);
    return static_cast<R>(*m_value);
  }

private:
  ReferencedType *m_value = nullptr;
};

// Void result
template <typename R> class ResultSlot<R, std::enable_if_t<std::is_void_v<R>>> {
public:
  template <typename Producer> void emplace_with(Producer &&producer) noexcept {
    std::invoke(std::forward<Producer>(producer));
  }

  void take() const noexcept {}
};

// -----------------------------------------------------------------------------
// Type-erased callable context
// -----------------------------------------------------------------------------

template <typename Fn, typename Result> struct CallableContext {
  using Callable = std::remove_reference_t<Fn>;

  Callable *callable;
  ResultSlot<Result> result;
};

template <typename Fn, typename Result>
void execute_callable(void *userdata) noexcept {
  auto &context = *static_cast<CallableContext<Fn, Result> *>(userdata);

  context.result.emplace_with([&]() -> decltype(auto) {
    if constexpr (std::is_lvalue_reference_v<Fn>) {
      return std::invoke(*context.callable);
    } else {
      return std::invoke(std::move(*context.callable));
    }
  });
}

} // namespace strobe::platform::detail
