LIST(APPEND EO_CC_SOURCES
   eo.c
   eo_base_class.c
   )

ADD_DEFINITIONS(-DEFL_EO_BUILD=1)

include_directories(
   ${EINA_INCLUDE_DIRS}
   )

add_library(eo SHARED ${EO_CC_SOURCES})
target_link_libraries(eo
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${EFL_COVERAGE_LIBS}
   )

set_target_properties(eo PROPERTIES
   COMPILE_FLAGS "${EFL_COVERAGE_CFLAGS}")
set_target_properties(eo PROPERTIES
   VERSION ${PACKAGE_VERSION} SOVERSION ${VMAJ})
set_target_properties(eo PROPERTIES OUTPUT_NAME "eo")

INSTALL(TARGETS eo DESTINATION "lib")
INSTALL(FILES  "${CMAKE_CURRENT_SOURCE_DIR}/Eo.h"
      DESTINATION "include")
