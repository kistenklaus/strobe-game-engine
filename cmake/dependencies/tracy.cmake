include_guard(GLOBAL)

include(cmake/colorful.cmake)
include(FetchContent)

set(TRACY_ENABLE ${STROBE_TRACY} CACHE BOOL "" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
set(TRACY_ONLY_LOCALHOST ON CACHE BOOL "" FORCE)
set(TRACY_NO_BROADCAST ON CACHE BOOL "" FORCE)


FetchContent_Declare(
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG 05cceee0df3b8d7c6fa87e9638af311dbabc63cb # v0.13.1 (2026-06-16)
    EXCLUDE_FROM_ALL
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(tracy)

if (TARGET TracyClient)
  target_compile_options(
    TracyClient
    PRIVATE
      "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-w>"
      "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W0>"
  )

  set_property(
    TARGET TracyClient
    PROPERTY COMPILE_WARNING_AS_ERROR OFF
  )
endif()
# expose as strobe::tracy

add_library(strobe_tracy INTERFACE)
add_library(strobe::tracy ALIAS strobe_tracy)

target_link_libraries(
    strobe_tracy
    INTERFACE
        Tracy::TracyClient
)

if (STROBE_TRACY) 
  target_compile_options(
      strobe_tracy
      INTERFACE
          "$<$<AND:$<PLATFORM_ID:Linux>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-g;-fno-omit-frame-pointer>"
  )
  target_compile_definitions(
    strobe_tracy
    INTERFACE
      "$<$<BOOL:${STROBE_TRACY}>:STROBE_TRACY=1>"
  )
  target_link_options(
      strobe_tracy
      INTERFACE
          "$<$<AND:$<PLATFORM_ID:Linux>,$<LINK_LANG_AND_ID:CXX,GNU,Clang>>:-rdynamic>"
  )
endif()
