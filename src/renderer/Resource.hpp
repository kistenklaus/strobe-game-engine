#pragma once

namespace sge {

class Resource {
 public:
  Resource() {}
  virtual ~Resource() = 0;
  Resource(Resource&) = delete;
  Resource(Resource&&) = default;
  Resource& operator=(Resource&) = delete;
  Resource& operator=(Resource&&) = default;
};

}  // namespace sge
