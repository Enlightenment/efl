LIST(APPEND EOBJ_CC_SOURCES
   eobj.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   )

add_library(eobj SHARED ${EOBJ_CC_SOURCES})
target_link_libraries(eobj
   ${EINA_LIBRARIES}
   ${EFL_COVERAGE_LIBS}
   )

set_target_properties(eobj PROPERTIES
   COMPILE_FLAGS "${EFL_COVERAGE_CFLAGS}")
set_target_properties(eobj PROPERTIES
                         VERSION ${EOBJ_VERSION} SOVERSION ${EOBJ_VERSION_MAJOR})
set_target_properties(eobj PROPERTIES OUTPUT_NAME "eobj")

INSTALL(TARGETS eobj DESTINATION "lib")
INSTALL(FILES  "${CMAKE_CURRENT_SOURCE_DIR}/Eobj.h"
      DESTINATION "include")
