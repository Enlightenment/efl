LIST(APPEND ACCESS_CC_SOURCES
   main.c
   simple.c
   inherit.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/lib
   )

add_executable(access ${ACCESS_CC_SOURCES})

get_target_property(eobj_LIB_FILE eobj LOCATION)
target_link_libraries(access
   ${EINA_LIBRARIES}
   ${eobj_LIB_FILE}
   )

add_dependencies(access eobj)

add_test(Example_access access)
