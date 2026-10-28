#pragma once

#include "strobe/flow/flow.hpp"
#include "strobe/flow/resource.hpp"
#include <memory>

namespace strobe::flow {

class boolean {
public:
  boolean(bool v) noexcept : m_runtime(resource<bool>()) {
    construct(m_runtime, [v](bool *ptr) { 
        ZoneScopedN("boolean::boolean");
        std::construct_at(ptr, v); 
        });
  }
  boolean(flow::Resource<bool> r) noexcept : m_runtime(r) {}

  friend boolean operator&&(const boolean &lhs, const boolean &rhs) noexcept {
    ZoneScopedN("flow::boolean::operator&&");
    auto res = flow::resource<bool>();
    flow::construct(res, read(lhs.m_runtime), read(rhs.m_runtime),
                    [](bool *out, const bool &lhs, const bool &rhs) {
                      std::construct_at(out, lhs && rhs);
                    });
    return boolean{res};
  }

  friend boolean operator||(const boolean &lhs, const boolean &rhs) noexcept {
    ZoneScopedN("flow::boolean::operator||");
    auto res = flow::resource<bool>();
    flow::construct(res, read(lhs.m_runtime), read(rhs.m_runtime),
                    [](bool *out, const bool &lhs, const bool &rhs) {
                      std::construct_at(out, lhs || rhs);
                    });
    return boolean{res};
  }

private:
  flow::Resource<bool> m_runtime;
};

} // namespace strobe::flow
