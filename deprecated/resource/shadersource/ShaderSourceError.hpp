#pragma once

#include "resource/file/FileError.hpp"
namespace strobe::resource {

class ShaderSourceErrorType {
 public:
  virtual ~ShaderSourceErrorType() = default;
  virtual std::string message() const = 0;
};

class ShaderSourceErrorFileError : public ShaderSourceErrorType {
 public:
  ShaderSourceErrorFileError(FileError fileError)
      : m_fileError(std::move(fileError)) {}

  std::string message() const { return m_fileError->message(); }

 private:
  FileError m_fileError;
};

using ShaderSourceError = std::unique_ptr<ShaderSourceErrorType>;

}  // namespace strobe::resource
