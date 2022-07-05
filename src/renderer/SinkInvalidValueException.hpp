#pragma once
#include <stdexcept>

namespace sge {

// TODO proper cpp implementation pls

class SinkInvalidValueException : public std::exception {
 public:
  SinkInvalidValueException(const std::string msg = "sink value is invalid")
      : m_msg(msg) {}

  const char* what() const throw() { return m_msg.c_str(); }

 private:
  const std::string m_msg;
};

}  // namespace sge
