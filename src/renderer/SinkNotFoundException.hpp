#pragma once
#include <stdexcept>

namespace sge {

// TODO proper cpp implementation pls

class SinkNotFoundException : public std::exception {
 public:
  SinkNotFoundException(const std::string msg = "could not find sink")
      : m_msg(msg) {}

  const char* what() const throw() { return m_msg.c_str(); }

 private:
  const std::string m_msg;
};

}  // namespace sge
