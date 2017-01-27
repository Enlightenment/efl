set(LIBRARIES
  eina
)

set(SOURCES
  triangulator_simple.c
  triangulator_simple.h
  triangulator_stroker.c
  triangulator_stroker.h
)

set(INCLUDE_DIRECTORIES
  ${CMAKE_BINARY_DIR}/src/lib
)

set(DEPENDENCIES
  efl-eo
)
