#include "strobe/core/fs/directory.hpp"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <stdexcept>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>

namespace strobe::fs {
namespace {

DIR *directory_of(void *handle) noexcept {
  return static_cast<DIR *>(handle);
}

bool is_dot_entry(const char *name) noexcept {
  return name[0] == '.' && (name[1] == '\0' ||
                            (name[1] == '.' && name[2] == '\0'));
}

Type entry_type(DIR *directory, const dirent *entry) {
  switch (entry->d_type) {
  case DT_REG:
    return Type::File;
  case DT_DIR:
    return Type::Directory;
  case DT_LNK:
    return Type::SymLink;
  case DT_UNKNOWN:
    break;
  default:
    throw std::runtime_error("Unsupported directory entry type");
  }

  struct stat status{};
  const int fd = dirfd(directory);
  assert(fd != -1);
  if (fstatat(fd, entry->d_name, &status, AT_SYMLINK_NOFOLLOW) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "Failed to determine directory entry type");
  }
  if (S_ISREG(status.st_mode)) {
    return Type::File;
  }
  if (S_ISDIR(status.st_mode)) {
    return Type::Directory;
  }
  if (S_ISLNK(status.st_mode)) {
    return Type::SymLink;
  }
  throw std::runtime_error("Unsupported directory entry type");
}

} // namespace

Directory::Directory(PathView path) {
  DIR *directory = ::opendir(path.c_str());
  if (directory == nullptr) {
    throw std::system_error(errno, std::generic_category(),
                            "Failed to open directory");
  }
  m_handle = directory;
  m_opened = true;
}

Directory &Directory::operator=(Directory &&other) noexcept {
  if (this == &other) {
    return *this;
  }
  close_platform();
  m_handle = std::exchange(other.m_handle, nullptr);
  m_opened = std::exchange(other.m_opened, false);
  m_platform_state = other.m_platform_state;
  m_name = other.m_name;
  return *this;
}

Directory::~Directory() noexcept { close_platform(); }

void Directory::close_platform() noexcept {
  if (m_opened && m_handle != nullptr) {
    ::closedir(directory_of(m_handle));
  }
  m_handle = nullptr;
  m_opened = false;
}

void Directory::close() {
  if (!m_opened) {
    return;
  }
  if (m_handle != nullptr && ::closedir(directory_of(m_handle)) != 0) {
    m_handle = nullptr;
    m_opened = false;
    throw std::system_error(errno, std::generic_category(),
                            "Failed to close directory");
  }
  m_handle = nullptr;
  m_opened = false;
}

bool Directory::advance(EntryView &entry) {
  if (!m_opened) {
    return false;
  }
  while (dirent *value = ::readdir(directory_of(m_handle))) {
    if (is_dot_entry(value->d_name)) {
      continue;
    }
    entry = EntryView{PathView(value->d_name),
                      entry_type(directory_of(m_handle), value)};
    return true;
  }
  return false;
}

Directory::iterator Directory::begin() {
  iterator result(this);
  result.m_end = !advance(result.m_entry);
  if (result.m_end) {
    result.m_directory = nullptr;
  }
  return result;
}

Directory::iterator &Directory::iterator::operator++() {
  if (m_end) {
    return *this;
  }
  m_end = !m_directory->advance(m_entry);
  if (m_end) {
    m_directory = nullptr;
  }
  return *this;
}

Directory ls(PathView path) { return Directory(path); }

} // namespace strobe::fs
