LIST(APPEND INTERFACE_CC_SOURCES
   main.c
   simple.c
   interface.c
   interface2.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

add_executable(interface ${INTERFACE_CC_SOURCES})

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(interface
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(interface eo)
add_dependencies(check interface)

add_test(Test_interface interface)
