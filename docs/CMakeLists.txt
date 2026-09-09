cmake_minimum_required(VERSION 3.20)

find_program(STROBE_DOCS_DOXYGEN_EXECUTABLE NAMES doxygen REQUIRED)
find_program(STROBE_DOCS_PANDOC_EXECUTABLE NAMES pandoc REQUIRED)
find_program(STROBE_DOCS_LATEXMK_EXECUTABLE NAMES latexmk REQUIRED)
find_program(STROBE_DOCS_LUALATEX_EXECUTABLE NAMES lualatex REQUIRED)

set(STROBE_DOCS_TITLE "Strobe Documentation")
# Full path of the final user-facing artefact, including its filename.
set(STROBE_DOCS_PDF_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/refman.pdf")
set(STROBE_DOCS_TARGET "strobe-docs")
set(STROBE_DOCS_DOXYGEN_INPUTS
  "${PROJECT_SOURCE_DIR}/libs/rhi/include")

set(_strobe_docs_source_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(_strobe_docs_binary_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(_strobe_docs_pages_dir "${_strobe_docs_binary_dir}/pages")
set(_strobe_docs_doxygen_dir "${_strobe_docs_binary_dir}/doxygen")
set(_strobe_docs_doxygen_latex_dir "${_strobe_docs_doxygen_dir}/latex")

file(MAKE_DIRECTORY "${_strobe_docs_binary_dir}")
file(MAKE_DIRECTORY "${_strobe_docs_pages_dir}")

# Do not make an ordinary code build depend on a documentation tool being
# installed.  Once STROBE_BUILD_DOCUMENTATION is ON all missing tools are hard
# configuration errors with the find_program calls above.

set(_strobe_docs_doxygen_inputs_doxyfile "")
set(_strobe_docs_doxygen_dependencies "")
foreach(_input IN LISTS STROBE_DOCS_DOXYGEN_INPUTS)
  if(NOT IS_ABSOLUTE "${_input}")
    set(_input "${PROJECT_SOURCE_DIR}/${_input}")
  endif()
  cmake_path(ABSOLUTE_PATH _input NORMALIZE OUTPUT_VARIABLE _input)
  if(NOT EXISTS "${_input}")
    message(FATAL_ERROR "Doxygen input does not exist: ${_input}")
  endif()

  string(REPLACE "\\" "/" _input_doxyfile "${_input}")
  string(APPEND _strobe_docs_doxygen_inputs_doxyfile " \"${_input_doxyfile}\"")

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

set(STROBE_DOCS_DOXYGEN_OUTPUT "${_strobe_docs_doxygen_dir}")
set(STROBE_DOCS_DOXYGEN_INPUTS_DOXYFILE "${_strobe_docs_doxygen_inputs_doxyfile}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
  "${_strobe_docs_binary_dir}/Doxyfile" @ONLY)

# Markdown files are the documentation's only hierarchy declaration.  The
# generated manual-structure.tex is intentionally a build artefact, never a
# second table of contents to keep in sync by hand.
file(GLOB_RECURSE _strobe_docs_markdown CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE
  RELATIVE "${_strobe_docs_source_dir}" "${_strobe_docs_source_dir}/*.md")
list(FILTER _strobe_docs_markdown EXCLUDE REGEX "^README\\.md$")
list(FILTER _strobe_docs_markdown EXCLUDE REGEX "^cmake/")
list(FILTER _strobe_docs_markdown EXCLUDE REGEX "^latex/")
list(FILTER _strobe_docs_markdown EXCLUDE REGEX "^pandoc/")
list(SORT _strobe_docs_markdown)

if(NOT _strobe_docs_markdown)
  message(FATAL_ERROR "No authored Markdown pages were found below ${_strobe_docs_source_dir}.")
endif()

# Changing a page title changes the generated TeX driver, so make CMake
# reconfigure when any authored page changes, not merely when files are added.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  ${_strobe_docs_markdown})

# Tree traversal order: a directory's index.md, then its ordinary pages, then
# its child directories.  Directories containing pages must have an index.md;
# this makes a missing structural chapter an immediate configuration error.
macro(_strobe_docs_visit _relative_directory)
  if("${_relative_directory}" STREQUAL "")
    set(_prefix "")
  else()
    set(_prefix "${_relative_directory}/")
  endif()

  set(_index_page "")
  set(_ordinary_pages "")
  set(_child_directories "")
  foreach(_page IN LISTS _strobe_docs_markdown)
    get_filename_component(_page_dir "${_page}" DIRECTORY)
    if("${_page_dir}" STREQUAL ".")
      set(_page_dir "")
    endif()
    get_filename_component(_page_name "${_page}" NAME)
    if("${_page_dir}" STREQUAL "${_relative_directory}")
      if("${_page_name}" STREQUAL "index.md")
        set(_index_page "${_page}")
      else()
        list(APPEND _ordinary_pages "${_page}")
      endif()
    endif()
    if("${_page}" MATCHES "^${_prefix}([^/]+)/")
      list(APPEND _child_directories "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  list(REMOVE_DUPLICATES _child_directories)
  list(SORT _ordinary_pages)
  list(SORT _child_directories)

  if(NOT "${_relative_directory}" STREQUAL "" AND
     (NOT "${_ordinary_pages}" STREQUAL "" OR NOT "${_child_directories}" STREQUAL "") AND
     "${_index_page}" STREQUAL "")
    message(FATAL_ERROR
      "Documentation directory '${_relative_directory}' contains pages but no index.md. "
      "An index.md defines every directory chapter/section.")
  endif()

  if(NOT "${_index_page}" STREQUAL "")
    list(APPEND _strobe_docs_order "${_index_page}")
  endif()
  list(APPEND _strobe_docs_order ${_ordinary_pages})
  foreach(_child IN LISTS _child_directories)
    if("${_relative_directory}" STREQUAL "")
      set(_child_path "${_child}")
    else()
      set(_child_path "${_relative_directory}/${_child}")
    endif()
    _strobe_docs_visit("${_child_path}")
  endforeach()
endmacro()

set(_strobe_docs_order "")
_strobe_docs_visit("")

function(_strobe_docs_title page out_title)
  file(STRINGS "${_strobe_docs_source_dir}/${page}" _lines LIMIT_COUNT 32)
  set(_title "")
  foreach(_line IN LISTS _lines)
    if(_line MATCHES "^title:[ \\t]*(.+)$")
      set(_title "${CMAKE_MATCH_1}")
      break()
    endif()
  endforeach()
  if("${_title}" STREQUAL "")
    message(FATAL_ERROR
      "${page} has no YAML 'title:' in its first 32 lines. "
      "The title is used for the heading generated from the docs tree.")
  endif()
  string(STRIP "${_title}" _title)
  if(_title MATCHES "^\"(.*)\"$")
    set(_title "${CMAKE_MATCH_1}")
  endif()
  if(_title MATCHES "^'(.*)'$")
    set(_title "${CMAKE_MATCH_1}")
  endif()
  set(${out_title} "${_title}" PARENT_SCOPE)
endfunction()

function(_strobe_docs_tex_escape input out_value)
  set(_value "${input}")
  string(REPLACE "\\" "\\textbackslash{}" _value "${_value}")
  string(REPLACE "&" "\\&" _value "${_value}")
  string(REPLACE "%" "\\%" _value "${_value}")
  string(REPLACE "$" "\\$" _value "${_value}")
  string(REPLACE "#" "\\#" _value "${_value}")
  string(REPLACE "_" "\\_" _value "${_value}")
  string(REPLACE "{" "\\{" _value "${_value}")
  string(REPLACE "}" "\\}" _value "${_value}")
  string(REPLACE "~" "\\textasciitilde{}" _value "${_value}")
  string(REPLACE "^" "\\textasciicircum{}" _value "${_value}")
  set(${out_value} "${_value}" PARENT_SCOPE)
endfunction()

set(_strobe_docs_structure "% Generated from the docs/ directory tree. Do not edit.\n")
set(_strobe_docs_page_outputs "")
set(_strobe_docs_graphic_paths "${_strobe_docs_source_dir}")
foreach(_page IN LISTS _strobe_docs_order)
  get_filename_component(_page_dir "${_page}" DIRECTORY)
  if("${_page_dir}" STREQUAL ".")
    set(_page_dir "")
  endif()
  get_filename_component(_page_name "${_page}" NAME)
  get_filename_component(_page_stem "${_page}" NAME_WE)

  if("${_page_dir}" STREQUAL "")
    set(_directory_depth 0)
  else()
    string(REPLACE "/" ";" _directory_parts "${_page_dir}")
    list(LENGTH _directory_parts _directory_depth)
  endif()

  if("${_page_dir}" STREQUAL "")
    # docs/overview.md and a hypothetical docs/index.md
    set(_level 0)
  elseif("${_page_name}" STREQUAL "index.md")
    # docs/libs/index.md -> chapter
    # docs/libs/rhi/index.md -> section
    math(EXPR _level "${_directory_depth} - 1")
  else()
    # docs/libs/foo.md -> section
    # docs/libs/rhi/barriers.md -> subsection
    set(_level "${_directory_depth}")
  endif()

  set(_heading_commands chapter section subsection subsubsection paragraph)
  list(GET _heading_commands ${_level} _heading_command)
  _strobe_docs_title("${_page}" _title)
  _strobe_docs_tex_escape("${_title}" _title_tex)

  string(REPLACE ".md" ".tex" _page_tex_rel "${_page}")
  set(_page_tex "${_strobe_docs_pages_dir}/${_page_tex_rel}")
  get_filename_component(_page_tex_dir "${_page_tex}" DIRECTORY)
  file(MAKE_DIRECTORY "${_page_tex_dir}")
  list(APPEND _strobe_docs_page_outputs "${_page_tex}")

  # A Markdown # heading is content beneath the generated structural heading:
  # chapter -> section, section -> subsection, and so on.
  add_custom_command(
    OUTPUT "${_page_tex}"
    COMMAND "${STROBE_DOCS_PANDOC_EXECUTABLE}"
      --from=markdown+yaml_metadata_block
      --to=latex
      --wrap=none
      "--shift-heading-level-by=${_level}"
      "--lua-filter=${CMAKE_CURRENT_SOURCE_DIR}/pandoc/doxygen.lua"
      "${_strobe_docs_source_dir}/${_page}"
      --output "${_page_tex}"
    DEPENDS "${_strobe_docs_source_dir}/${_page}"
      "${CMAKE_CURRENT_SOURCE_DIR}/pandoc/doxygen.lua"
    VERBATIM)

  string(REGEX REPLACE "[^A-Za-z0-9]+" "-" _label "${_page}")
  string(APPEND _strobe_docs_structure
    "\\clearpage\n\\${_heading_command}{${_title_tex}}\n"
    "\\label{doc:${_label}}\n"
    "\\input{pages/${_page_tex_rel}}\n")

  if(NOT "${_page_dir}" STREQUAL "")
    list(APPEND _strobe_docs_graphic_paths "${_strobe_docs_source_dir}/${_page_dir}")
  endif()
endforeach()
file(WRITE "${_strobe_docs_binary_dir}/manual-structure.tex" "${_strobe_docs_structure}")

list(REMOVE_DUPLICATES _strobe_docs_graphic_paths)
set(STROBE_DOCS_GRAPHIC_PATHS "")
foreach(_path IN LISTS _strobe_docs_graphic_paths)
  string(REPLACE "\\" "/" _path "${_path}")
  string(APPEND STROBE_DOCS_GRAPHIC_PATHS "{${_path}/}")
endforeach()
set(STROBE_DOCS_GENERATED_DIR "${_strobe_docs_binary_dir}")
set(STROBE_DOCS_DOXYGEN_LATEX_DIR "${_strobe_docs_doxygen_latex_dir}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/latex/manual.tex.in"
  "${_strobe_docs_binary_dir}/manual.tex" @ONLY)

add_custom_command(
  OUTPUT "${_strobe_docs_doxygen_latex_dir}/doxygen.sty"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${_strobe_docs_doxygen_dir}"
  COMMAND "${STROBE_DOCS_DOXYGEN_EXECUTABLE}" "${_strobe_docs_binary_dir}/Doxyfile"
  DEPENDS "${_strobe_docs_binary_dir}/Doxyfile" ${_strobe_docs_doxygen_dependencies}
  WORKING_DIRECTORY "${_strobe_docs_binary_dir}"
  VERBATIM)

set(_strobe_docs_pdf "${STROBE_DOCS_PDF_OUTPUT}")
get_filename_component(_strobe_docs_pdf_dir "${_strobe_docs_pdf}" DIRECTORY)
get_filename_component(_strobe_docs_pdf_name "${_strobe_docs_pdf}" NAME)
get_filename_component(_strobe_docs_pdf_stem "${_strobe_docs_pdf}" NAME_WE)
get_filename_component(_strobe_docs_pdf_extension "${_strobe_docs_pdf}" LAST_EXT)
if(NOT _strobe_docs_pdf_extension STREQUAL ".pdf")
  message(FATAL_ERROR
    "STROBE_DOCS_PDF_OUTPUT must name a PDF file, got: ${_strobe_docs_pdf}")
endif()
add_custom_command(
  OUTPUT "${_strobe_docs_pdf}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${_strobe_docs_pdf_dir}"
  COMMAND "${CMAKE_COMMAND}" -E env
    "TEXINPUTS=${_strobe_docs_doxygen_latex_dir}//:"
    "${STROBE_DOCS_LATEXMK_EXECUTABLE}"
      -g
      -lualatex -interaction=nonstopmode -halt-on-error -file-line-error
      "-jobname=${_strobe_docs_pdf_stem}"
      "-outdir=${_strobe_docs_pdf_dir}" manual.tex
  DEPENDS "${_strobe_docs_binary_dir}/manual.tex"
    "${_strobe_docs_binary_dir}/manual-structure.tex"
    ${_strobe_docs_page_outputs}
    "${_strobe_docs_doxygen_latex_dir}/doxygen.sty"
  WORKING_DIRECTORY "${_strobe_docs_binary_dir}"
  VERBATIM)

add_custom_target(${STROBE_DOCS_TARGET} DEPENDS "${_strobe_docs_pdf}")
set(STROBE_DOCS_PDF "${_strobe_docs_pdf}" PARENT_SCOPE)
