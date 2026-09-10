if(NOT DEFINED STROBE_DOCS_HTML_OUTPUT_DIR)
  message(FATAL_ERROR "STROBE_DOCS_HTML_OUTPUT_DIR is required")
endif()

if(NOT DEFINED STROBE_DOCS_SEARCH_SCRIPT_NAME)
  message(FATAL_ERROR "STROBE_DOCS_SEARCH_SCRIPT_NAME is required")
endif()

file(GLOB_RECURSE _strobe_docs_html_files
  LIST_DIRECTORIES FALSE
  "${STROBE_DOCS_HTML_OUTPUT_DIR}/*.html")

set(_strobe_docs_script_tag
  "<script src=\"${STROBE_DOCS_SEARCH_SCRIPT_NAME}\"></script>")

foreach(_html_file IN LISTS _strobe_docs_html_files)
  file(READ "${_html_file}" _html)

  string(FIND "${_html}" "${_strobe_docs_script_tag}" _already_injected)
  if(NOT _already_injected EQUAL -1)
    continue()
  endif()

  string(FIND "${_html}" "</head>" _head_end)
  if(_head_end EQUAL -1)
    message(WARNING "Could not find </head> in ${_html_file}")
    continue()
  endif()

  string(REPLACE "</head>"
    "${_strobe_docs_script_tag}\n</head>"
    _html "${_html}")
  file(WRITE "${_html_file}" "${_html}")
endforeach()
