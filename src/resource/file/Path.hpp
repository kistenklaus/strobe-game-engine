#pragma once

#include <filesystem>
#include <string_view>
#include <utility>

namespace strobe::resource {

class Path {
 private:
  static constexpr std::string_view resourceDir = "./resources/";

 public:
  Path() : m_path() {}
  static Path toResource(std::string_view path) {
    std::filesystem::path resource = resourceDir;
    resource /= path;
    return Path(resource);
  }

  const std::string& operator*() const { return m_path; }

  const std::string* operator->() const { return &m_path; }

  friend auto operator<=>(const Path& a, const Path& b) {
    return a.m_path <=> b.m_path;
  }
  bool operator==(const Path& other) const = default;

 private:
  explicit Path(std::filesystem::path path) : m_path(std::move(path)) {}
  std::string m_path;
};

}  // namespace strobe::resource

namespace std {
template <>
struct hash<strobe::resource::Path> {
  std::size_t operator()(const strobe::resource::Path& p) const noexcept {
    return std::hash<std::filesystem::path>{}(*p);
  }
};
}  // namespace std
