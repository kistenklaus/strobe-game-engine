find_package(Doxygen REQUIRED)

set(STROBE_DOXYFILE
    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
    "${STROBE_DOXYFILE}"
    @ONLY
)

add_custom_target(strobe-docs
    COMMAND Doxygen::doxygen "${STROBE_DOXYFILE}"
    COMMAND ${CMAKE_MAKE_PROGRAM}
            -C "${CMAKE_CURRENT_BINARY_DIR}/strobe/latex"
            refman.pdf
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Generating Strobe PDF documentation"
    VERBATIM
)
