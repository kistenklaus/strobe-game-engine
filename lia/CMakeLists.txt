

set(LIA_NAME "${PROJECT_NAME}_lia")

set(LIA_SRC 
  "src/mat.c"
  "src/mat.h"
  "src/vec.c"
  "src/vec.h"
)

add_library(${LIA_NAME} ${LIA_SRC})

target_include_directories(${LIA_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
