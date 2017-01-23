set(INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/src/lib/eo/
)

set(LIBRARIES
    eina
    eolian
)

set(DEFINITIONS
  -DEOLIAN_GEN="${EOLIAN_BIN}")

set(SOURCES
  #for now no test suite
  eolian_generation.c
  eolian_parsing.c
  eolian_suite.c
  eolian_suite.h
)
