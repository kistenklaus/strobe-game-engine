Strobe documentation
====================

This is a native Doxygen HTML build. It contains no Markdown, Pandoc, TeX, or
Python.

Build on Arch:

  sudo pacman -S doxygen
  cmake -S . -B build -DSTROBE_BUILD_DOCS=ON
  cmake --build build --target strobe-docs
  xdg-open build/docs/html/index.html

The default output directory is build/docs/html. Set STROBE_DOCS_HTML_OUTPUT
before add_subdirectory(docs) to place the entire static site elsewhere:

  set(STROBE_DOCS_HTML_OUTPUT "${CMAKE_BINARY_DIR}/site")
  add_subdirectory(docs)

Writing documentation
---------------------

docs/main.dox is the native Doxygen main page. Add further prose pages as .dox
files anywhere below docs/. They are automatically parsed by Doxygen; CMake
does not maintain a page hierarchy.

For example:

  /**
   * \page rhi_design RHI design
   *
   * \section rhi_lifetimes Resource lifetimes
   *
   * Explanation here.
   */

Link a page from another Doxygen page with:

  \subpage rhi_design

API reference lives in normal C++ documentation comments. Define a group once,
then put each public type in it:

  /** \defgroup rhi RHI API Reference */

  /**
   * \ingroup rhi
   * \brief A device-owned buffer.
   */
  class Buffer;

Use \ref rhi "RHI API reference" to link that group from a .dox page. The
custom Doxygen layout deliberately removes global Topics, Classes, Namespaces,
and Files navigation. Group pages go directly to their documented types.

STROBE_DOCS_DOXYGEN_INPUTS in docs/CMakeLists.txt is a normal (non-cache)
CMake variable containing code roots. docs/ itself is always parsed for .dox
pages.
