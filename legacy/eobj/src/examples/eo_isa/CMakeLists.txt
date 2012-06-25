LIST(APPEND EO_ISA_CC_SOURCES
   main.c
   simple.c
   complex.c
   interface.c
   mixin.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

add_executable(eo_isa ${EO_ISA_CC_SOURCES})

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(eo_isa
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(eo_isa eo)
