LIST(APPEND COMPOSITE_OBJECTS_CC_SOURCES
   main.c
   simple.c
   comp.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/lib
   )

add_executable(composite_objects ${COMPOSITE_OBJECTS_CC_SOURCES})

get_target_property(eobj_LIB_FILE eobj LOCATION)
target_link_libraries(composite_objects
   ${EINA_LIBRARIES}
   ${eobj_LIB_FILE}
   )

add_dependencies(composite_objects eobj)

add_test(Example_composite_objects composite_objects)
