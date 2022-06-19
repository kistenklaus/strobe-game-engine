#pragma once

#include "renderer/Resource.hpp"

namespace sge {

class Bindable : public Resource {
 public:
  virtual ~Bindable() override = 0;
  virtual void bind() = 0;
};

}  // namespace sge
