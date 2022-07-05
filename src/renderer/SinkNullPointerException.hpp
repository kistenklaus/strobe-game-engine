#pragma once
#include <stdexcept>

namespace sge {

// TODO proper cpp implementation pls

class SinkNullPointerException : public std::exception {
 public:
  SinkNullPointerException(
      const std::string msg = "sink is null, but not nullable")
      : m_msg(msg) {}

  const char* what() const throw() { return m_msg.c_str(); }

 private:
  const std::string m_msg;
};

}  // namespace sge
