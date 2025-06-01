#include "./pages.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

std::size_t strobe::page_size() {
#if defined(_WIN32)
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  static const std::size_t size = static_cast<std::size_t>(sysInfo.dwPageSize);
#else
  static const std::size_t size =
      static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
#endif
  return size;
}
