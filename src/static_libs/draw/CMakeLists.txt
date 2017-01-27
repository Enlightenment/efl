set(LIBRARIES
  eina
)

set(SOURCES
  draw_main.c
  draw_main_neon.c
  draw_main_sse2.c
)

set(INCLUDE_DIRECTORIES
  ${CMAKE_BINARY_DIR}/src/lib
)

set(DEPENDENCIES
  efl-eo
)
