#pragma once

#include "resource/file/Path.hpp"
namespace strobe::resource {

class FileErrorType {
 public:
 public:
  virtual ~FileErrorType() = default;

  virtual std::string message() const = 0;
};

class FileErrorFailedToOpen : public FileErrorType {
 public:
  FileErrorFailedToOpen(strobe::resource::Path path) : m_path(path) {}

  std::string message() const final override {
    return std::format("Failed to open file {}", m_path->c_str());
  }

 private:
  strobe::resource::Path m_path;
};

class FileErrorFailedToStat : public FileErrorType {
 public:
  FileErrorFailedToStat(strobe::resource::Path path) : m_path(path) {}

  std::string message() const final override {
    return std::format("Failed to call fstat on file {}", m_path->c_str());
  }

 private:
  strobe::resource::Path m_path;
};

class FileErrorFailedToMap : public FileErrorType {
 public:
  FileErrorFailedToMap(strobe::resource::Path path) : m_path(path) {}

  std::string message() const final override {
    return std::format("Failed to map memory of file {}", m_path->c_str());
  }

 private:
  strobe::resource::Path m_path;
};

using FileError = std::unique_ptr<FileErrorType>;

}  // namespace strobe::resource
