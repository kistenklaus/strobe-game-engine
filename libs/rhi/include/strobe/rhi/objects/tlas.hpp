#pragma once

#include "strobe/rhi/objects/object.hpp"

namespace strobe::rhi {

class Tlas : Object<Tlas> {
  public:
    explicit Tlas(void* handle) noexcept : Object(handle) {}
    Tlas() noexcept : Object(nullptr) {}
    Tlas(const Tlas &) noexcept;
    Tlas(Tlas &&) noexcept;
    Tlas &operator=(const Tlas &) noexcept;
    Tlas &operator=(Tlas &&) noexcept;
    ~Tlas() noexcept;
};

}

