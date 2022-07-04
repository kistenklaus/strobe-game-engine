#pragma once

#include <stdexcept>

namespace sge {

class RendergraphResourcesDeprecatedException : public std::exception {
 public:
  const char* what() const throw() {
    return "rendergraph resources are deprecated";
  }
};

}  // namespace sge
