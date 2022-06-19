#pragma once

#include <memory>
#include <vector>

#include "renderer/Bindable.hpp"

namespace sge {

class Step {
 public:
  void addBindable(std::shared_ptr<Bindable> bindable);
  void removeBindable(std::shared_ptr<Bindable> bindable);

 private:
  std::vector<std::shared_ptr<Bindable>> m_bindables;
};

}  // namespace sge
