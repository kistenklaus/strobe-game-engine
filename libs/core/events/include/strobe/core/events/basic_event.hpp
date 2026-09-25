#pragma once
#include "event.hpp"
namespace strobe {

/**
 * \ingroup core
 * \brief Simple event carrying a payload without cancellation.
 *
 * \code{.cpp}
 * template<typename T> class BasicEvent;
 * \endcode
 */
template <typename T> class BasicEvent {
public:
  /**
   * \brief Constructs an event from a payload.
   * \code{.cpp}
   * explicit BasicEvent(T&& v);
   * explicit BasicEvent(const T& v);
   * \endcode
   *
   * \param v Payload to copy or move into the event.
   */
  explicit BasicEvent(T &&v) : m_payload(std::move(v)) {}
  explicit BasicEvent(const T &v) : m_payload(v) {}

  /**
   * \brief Type stored as the event payload.
   * \code{.cpp}
   * using payload_type = T;
   * \endcode
   */
  using payload_type = T;

  /**
   * \brief Public event payload.
   * \code{.cpp}
   * [[no_unique_address]] T m_payload;
   * \endcode
   *
   * The payload is initialized by a constructor and can be inspected or
   * modified directly.
   */
  [[no_unique_address]] T m_payload;

  /**
   * \brief Returns the event payload.
   * \code{.cpp}
   * const T& payload() const;
   * \endcode
   *
   * \return A const reference to the event payload.
   */
  const T &payload() const { return m_payload; }

  /**
   * \brief Reports whether the event is canceled; always false.
   * \code{.cpp}
   * constexpr bool canceled() const;
   * \endcode
   *
   * \return False.
   */
  constexpr bool canceled() const { return false; }
};
static_assert(events::Event<BasicEvent<int>>);

} // namespace strobe
