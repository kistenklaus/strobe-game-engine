#pragma once
#include <stdexcept>

namespace sge {

// TODO proper cpp implementation pls

class SinkNotLinkedException : public std::exception {
 public:
  SinkNotLinkedException(
      const std::string msg = "tried to access a unlinked sync")
      : m_msg(msg) {}

  const char* what() const throw() { return m_msg.c_str(); }

 private:
  const std::string m_msg;
};

}  // namespace sge
