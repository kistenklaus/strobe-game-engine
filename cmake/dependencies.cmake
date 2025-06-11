include_guard(GLOBAL)  # or DIRECTORY

set(FETCHCONTENT_QUIET ON)

include(cmake/dependencies/glfw.cmake)
include(cmake/dependencies/fmt.cmake)
# include(cmake/dependencies/vulkan.cmake)
# include(cmake/dependencies/spdlog.cmake)
# include(cmake/dependencies/shaderc.cmake)

include(cmake/dependencies/googletest.cmake)
