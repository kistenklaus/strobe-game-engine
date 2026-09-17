# -----------------------------------------------------------------------------
# Script mode: embed an existing SPIR-V binary into a C++ header.
# -----------------------------------------------------------------------------

if(DEFINED EMBED_SLANG_SPIRV_INPUT)
  if(NOT DEFINED EMBED_SLANG_SPIRV_OUTPUT)
    message(FATAL_ERROR "EMBED_SLANG_SPIRV_OUTPUT is required")
  endif()

  if(NOT DEFINED EMBED_SLANG_SPIRV_VARIABLE)
    message(FATAL_ERROR "EMBED_SLANG_SPIRV_VARIABLE is required")
  endif()

  file(READ "${EMBED_SLANG_SPIRV_INPUT}" DATA HEX)
  string(LENGTH "${DATA}" DATA_LENGTH)

  math(EXPR REMAINDER "${DATA_LENGTH} % 8")
  if(NOT REMAINDER EQUAL 0)
    message(FATAL_ERROR "SPIR-V size is not a multiple of 4 bytes")
  endif()

  # DATA contains two hex characters per byte, so one SPIR-V word
  # corresponds to 8 characters.
  math(EXPR WORD_COUNT "${DATA_LENGTH} / 8")

  file(WRITE "${EMBED_SLANG_SPIRV_OUTPUT}"
    "#pragma once\n"
    "\n"
    "#include <array>\n"
    "#include <cstdint>\n"
    "\n"
    "inline constexpr std::array<std::uint32_t, ${WORD_COUNT}> "
    "${EMBED_SLANG_SPIRV_VARIABLE} = {\n"
  )

  set(I 0)

  while(I LESS DATA_LENGTH)
    math(EXPR I1 "${I} + 2")
    math(EXPR I2 "${I} + 4")
    math(EXPR I3 "${I} + 6")

    string(SUBSTRING "${DATA}" ${I}  2 B0)
    string(SUBSTRING "${DATA}" ${I1} 2 B1)
    string(SUBSTRING "${DATA}" ${I2} 2 B2)
    string(SUBSTRING "${DATA}" ${I3} 2 B3)

    file(APPEND "${EMBED_SLANG_SPIRV_OUTPUT}"
      "  0x${B3}${B2}${B1}${B0}u,\n"
    )

    math(EXPR I "${I} + 8")
  endwhile()

  file(APPEND "${EMBED_SLANG_SPIRV_OUTPUT}"
    "};\n"
  )

  return()
endif()

# -----------------------------------------------------------------------------
# Configure mode: define embed_slang_spirv().
# -----------------------------------------------------------------------------

function(embed_slang_spirv TARGET)
  cmake_parse_arguments(
    ARG
    ""
    "PREFIX;SOURCE"
    "SLANG_FLAGS"
    ${ARGN}
  )

  if(NOT ARG_PREFIX)
    message(FATAL_ERROR
      "embed_slang_spirv(${TARGET}): PREFIX is required")
  endif()

  if(NOT ARG_SOURCE)
    message(FATAL_ERROR
      "embed_slang_spirv(${TARGET}): SOURCE is required")
  endif()

  find_program(SLANGC slangc REQUIRED)

  set(SOURCE
    "${ARG_PREFIX}/${ARG_SOURCE}"
  )

  set(GENERATED_ROOT
    "${CMAKE_CURRENT_BINARY_DIR}/generated/${TARGET}"
  )

  set(SPV
    "${GENERATED_ROOT}/${ARG_SOURCE}.spv"
  )

  set(HEADER
    "${GENERATED_ROOT}/${ARG_SOURCE}.spv.hpp"
  )

  set(DEPFILE
    "${SPV}.d"
  )

  get_filename_component(
    OUTPUT_DIR
    "${HEADER}"
    DIRECTORY
  )

  set(VARIABLE "${ARG_SOURCE}.spv")
  string(REGEX REPLACE
    "[^A-Za-z0-9_]"
    "_"
    VARIABLE
    "${VARIABLE}"
  )

  add_custom_command(
    OUTPUT "${HEADER}"

    COMMAND
      ${CMAKE_COMMAND} -E make_directory
      "${OUTPUT_DIR}"

    COMMAND
      "${SLANGC}"
      "${SOURCE}"
      ${ARG_SLANG_FLAGS}
      -depfile "${DEPFILE}"
      -o "${SPV}"

    COMMAND
      ${CMAKE_COMMAND}
      "-DEMBED_SLANG_SPIRV_INPUT=${SPV}"
      "-DEMBED_SLANG_SPIRV_OUTPUT=${HEADER}"
      "-DEMBED_SLANG_SPIRV_VARIABLE=${VARIABLE}"
      -P "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"

    DEPENDS
      "${SOURCE}"
      "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"

    BYPRODUCTS
      "${SPV}"

    DEPFILE
      "${DEPFILE}"

    COMMENT
      "Embedding Slang shader ${ARG_SOURCE}"

    VERBATIM
  )

  add_custom_target(
    "${TARGET}-generate"
    DEPENDS "${HEADER}"
  )

  add_library("${TARGET}" INTERFACE)

  add_dependencies(
    "${TARGET}"
    "${TARGET}-generate"
  )

  target_include_directories(
    "${TARGET}"
    INTERFACE
      "${GENERATED_ROOT}"
  )
endfunction()
