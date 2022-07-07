
#pragma once
#include <stdexcept>

namespace sge {

// TODO proper cpp implementation pls

class SourceNotFoundException : public std::exception {
 public:
  SourceNotFoundException(const std::string msg = "could not find source")
      : m_msg(msg) {}

  const char* what() const throw() { return m_msg.c_str(); }

 private:
  const std::string m_msg;
};

}  // namespace sge
