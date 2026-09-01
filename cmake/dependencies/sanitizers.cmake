add_library(strobe_sanitizer_flags INTERFACE)
add_library(strobe_sanitizers INTERFACE)

if (STROBE_SAN)
  target_compile_options(
    strobe_sanitizer_flags
    INTERFACE
      "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
      "$<$<CONFIG:Debug>:-fno-omit-frame-pointer>"
  )

  target_compile_definitions(strobe_sanitizer_flags
    INTERFACE STROBE_SAN=1
  )

  target_link_options(
    strobe_sanitizer_flags
    INTERFACE
      "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
  )

  add_library(
    strobe_sanitizer_runtime
    OBJECT
      "${CMAKE_CURRENT_LIST_DIR}/suppressions.cpp"
  )

  target_link_libraries(
    strobe_sanitizer_runtime
    PRIVATE
      strobe_sanitizer_flags
  )

  target_link_libraries(
    strobe_sanitizers
    INTERFACE
      strobe_sanitizer_flags
  )

  target_sources(
    strobe_sanitizers
    INTERFACE
      $<TARGET_OBJECTS:strobe_sanitizer_runtime>
  )
endif()
