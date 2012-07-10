LIST(APPEND COMPOSITE_OBJECTS_CC_SOURCES
   main.c
   simple.c
   comp.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

add_executable(composite_objects ${COMPOSITE_OBJECTS_CC_SOURCES})

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(composite_objects
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(composite_objects eo)
add_dependencies(check composite_objects)

add_test(Test_composite_objects composite_objects)
