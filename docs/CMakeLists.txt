cmake_minimum_required(VERSION 3.15)

find_program(STROBE_DOCS_DOXYGEN_EXECUTABLE NAMES doxygen)
if(NOT STROBE_DOCS_DOXYGEN_EXECUTABLE)
  message(FATAL_ERROR
    "Doxygen is required for STROBE_BUILD_DOCS. Install the 'doxygen' package.")
endif()

if(NOT DEFINED STROBE_DOCS_TITLE)
  set(STROBE_DOCS_TITLE "Strobe Documentation")
endif()
if(NOT DEFINED STROBE_DOCS_TARGET)
  set(STROBE_DOCS_TARGET "strobe-docs")
endif()

# Full directory of the generated static site. Set this before
# add_subdirectory(docs), or change the default here, to emit it elsewhere.
if(NOT DEFINED STROBE_DOCS_HTML_OUTPUT)
  set(STROBE_DOCS_HTML_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/html")
endif()

# This deliberately contains only code roots. docs/ itself is always a
# Doxygen input, so native .dox pages need no CMake registration.
if(NOT DEFINED STROBE_DOCS_DOXYGEN_INPUTS)
  set(STROBE_DOCS_DOXYGEN_INPUTS
    "${PROJECT_SOURCE_DIR}/libs/rhi/include")
endif()

set(_strobe_docs_source_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(_strobe_docs_main_page "${_strobe_docs_source_dir}/main.dox")
set(_strobe_docs_doxyfile "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
set(_strobe_docs_layout "${_strobe_docs_source_dir}/DoxygenLayout.xml")
set(_strobe_docs_stylesheet "${_strobe_docs_source_dir}/html/strobe.css")

foreach(_required_file IN ITEMS
    "${_strobe_docs_main_page}"
    "${_strobe_docs_layout}"
    "${_strobe_docs_stylesheet}")
  if(NOT EXISTS "${_required_file}")
    message(FATAL_ERROR "Missing documentation input: ${_required_file}")
  endif()
endforeach()

if("${STROBE_DOCS_HTML_OUTPUT}" STREQUAL "")
  message(FATAL_ERROR "STROBE_DOCS_HTML_OUTPUT must not be empty.")
endif()
if(NOT IS_ABSOLUTE "${STROBE_DOCS_HTML_OUTPUT}")
  set(STROBE_DOCS_HTML_OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/${STROBE_DOCS_HTML_OUTPUT}")
endif()
get_filename_component(STROBE_DOCS_HTML_OUTPUT
  "${STROBE_DOCS_HTML_OUTPUT}" ABSOLUTE)
get_filename_component(_strobe_docs_html_parent
  "${STROBE_DOCS_HTML_OUTPUT}" DIRECTORY)
get_filename_component(_strobe_docs_html_name
  "${STROBE_DOCS_HTML_OUTPUT}" NAME)
if("${_strobe_docs_html_name}" STREQUAL "")
  message(FATAL_ERROR
    "STROBE_DOCS_HTML_OUTPUT must name a directory, got: ${STROBE_DOCS_HTML_OUTPUT}")
endif()

set(_strobe_docs_doxygen_inputs "")
set(_strobe_docs_doxygen_dependencies "")

function(_strobe_docs_append_doxy_input path)
  string(REPLACE "\\" "/" _doxy_path "${path}")
  string(APPEND _strobe_docs_doxygen_inputs " \"${_doxy_path}\"")
  set(_strobe_docs_doxygen_inputs "${_strobe_docs_doxygen_inputs}" PARENT_SCOPE)
endfunction()

# Native Doxygen pages are simply .dox files below docs/. No generated
# Markdown, page IDs, or CMake-maintained hierarchy is involved.
_strobe_docs_append_doxy_input("${_strobe_docs_source_dir}")
file(GLOB_RECURSE _strobe_docs_page_inputs CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE
  "${_strobe_docs_source_dir}/*.dox")
list(APPEND _strobe_docs_doxygen_dependencies ${_strobe_docs_page_inputs})

foreach(_input IN LISTS STROBE_DOCS_DOXYGEN_INPUTS)
  if(NOT IS_ABSOLUTE "${_input}")
    set(_input "${PROJECT_SOURCE_DIR}/${_input}")
  endif()
  get_filename_component(_input "${_input}" ABSOLUTE)
  if(NOT EXISTS "${_input}")
    message(FATAL_ERROR "Doxygen input does not exist: ${_input}")
  endif()

  _strobe_docs_append_doxy_input("${_input}")
  if(IS_DIRECTORY "${_input}")
    file(GLOB_RECURSE _input_files CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE
      "${_input}/*.h" "${_input}/*.hh" "${_input}/*.hpp" "${_input}/*.hxx"
      "${_input}/*.c" "${_input}/*.cc" "${_input}/*.cpp" "${_input}/*.cxx"
      "${_input}/*.inl" "${_input}/*.dox")
    list(APPEND _strobe_docs_doxygen_dependencies ${_input_files})
  else()
    list(APPEND _strobe_docs_doxygen_dependencies "${_input}")
  endif()
endforeach()
list(REMOVE_DUPLICATES _strobe_docs_doxygen_dependencies)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
  "${_strobe_docs_layout}"
  "${_strobe_docs_stylesheet}"
  ${_strobe_docs_doxygen_dependencies})

string(REPLACE "\\" "/" STROBE_DOCS_DOXYGEN_OUTPUT_DIRECTORY
  "${_strobe_docs_html_parent}")
set(STROBE_DOCS_DOXYGEN_HTML_OUTPUT "${_strobe_docs_html_name}")
set(STROBE_DOCS_DOXYGEN_INPUTS_DOXYFILE "${_strobe_docs_doxygen_inputs}")
string(REPLACE "\\" "/" STROBE_DOCS_DOXYGEN_LAYOUT "${_strobe_docs_layout}")
string(REPLACE "\\" "/" STROBE_DOCS_DOXYGEN_STYLESHEET "${_strobe_docs_stylesheet}")
string(REPLACE "\\" "/" STROBE_DOCS_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
  "${_strobe_docs_doxyfile}" @ONLY)

add_custom_command(
  OUTPUT "${STROBE_DOCS_HTML_OUTPUT}/index.html"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${_strobe_docs_html_parent}"
  COMMAND "${STROBE_DOCS_DOXYGEN_EXECUTABLE}" "${_strobe_docs_doxyfile}"
  DEPENDS
    "${_strobe_docs_doxyfile}"
    "${_strobe_docs_layout}"
    "${_strobe_docs_stylesheet}"
    ${_strobe_docs_doxygen_dependencies}
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  VERBATIM)

add_custom_target(${STROBE_DOCS_TARGET}
  DEPENDS "${STROBE_DOCS_HTML_OUTPUT}/index.html")
set(STROBE_DOCS_HTML "${STROBE_DOCS_HTML_OUTPUT}" PARENT_SCOPE)
