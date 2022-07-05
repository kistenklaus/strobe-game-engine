#pragma once
#include <stdexcept>

namespace sge {

// TODO proper cpp implementation pls

class IncompatibleLinkageException : public std::exception {
 public:
  IncompatibleLinkageException(
      const std::string msg = "sink and source types do not match")
      : m_msg(msg) {}

  const char* what() const throw() { return m_msg.c_str(); }

 private:
  const std::string m_msg;
};

}  // namespace sge
