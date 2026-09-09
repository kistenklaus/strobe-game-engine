# Documentation build

This directory builds one single-sided PDF from authored Markdown and selected
Doxygen API-reference fragments. It requires no Python.

## One top-level integration line

Add this after the project's `project(...)` call:

```cmake
add_subdirectory(docs)
```

If the code to document is not under `include/` and `src/`, set the input list
immediately before that line:

```cmake
set(STROBE_DOCS_DOXYGEN_INPUTS
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/source")
add_subdirectory(docs)
```

Documentation is intentionally opt-in, so a normal code-only configuration
does not require documentation packages:

```sh
cmake -S . -B build -DSTROBE_BUILD_DOCUMENTATION=ON
cmake --build build --target strobe-docs
```

The resulting PDF is written to `build/docs/pdf/<project>-documentation.pdf`.

On Arch, install the tools with:

```sh
sudo pacman -S doxygen pandoc texlive-latexextra texlive-fontsextra
```

`latexmk` and LuaLaTeX are supplied by the TeX Live packages on current Arch
systems. Install any additional TeX package named by a LaTeX error if your
local TeX Live split differs.

## Authored hierarchy

The directory tree is the source of truth. `index.md` names a directory node;
ordinary Markdown pages appear immediately beneath it. Directories that
contain pages must have an `index.md`.

```text
docs/
  overview.md                  # chapter
  libraries/
    index.md                   # chapter: Libraries
    rhi/
      index.md                 # section: RHI
      descriptors.md           # subsection: Descriptors
  subsystems/
    index.md                   # chapter: Subsystems
```

Pages are ordered as: a directory's `index.md`, then its ordinary pages, then
its child directories; each set is alphabetical. Prefix names with `01-`,
`02-`, … when a non-alphabetical order matters. The YAML title controls the
printed name, so a numeric filename need not appear in the PDF.

Every page begins with a YAML title:

```markdown
---
title: Resource descriptor heaps
---

The paragraph starts here. A `# Heading` inside the page becomes a heading
*below* the one generated from the directory tree.
```

## Including code reference material

Use Doxygen groups as the API-reference boundary:

```cpp
/// \defgroup rhi Rendering hardware interface
/// @{ 

/// A device-local buffer.
class Buffer { /* ... */ };

/// @}
```

Then place an empty marker where the generated group reference belongs:

```markdown
## API reference

::: {.doxygen group="rhi"}
:::
```

The marker expands to Doxygen's `group__rhi.tex` fragment. Do not use a group
marker until that `\defgroup` exists; otherwise LaTeX correctly reports the
missing generated file.

The manual owns its table of contents and global structure. Doxygen's own
`refman.tex`, class index, file index, and namespace index are generated but
never included, so they cannot appear as surprise chapters in the final PDF.

## Owned styling

`latex/manual.tex.in` is the only global PDF styling file. It uses the
single-sided `report` class, keeps Doxygen's code/reference styling through the
generated `doxygen.sty`, and otherwise intentionally stays close to defaults.
Change it only when a genuine global typography decision is wanted.
