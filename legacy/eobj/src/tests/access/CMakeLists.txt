LIST(APPEND ACCESS_CC_SOURCES
   main.c
   simple.c
   inherit.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

add_executable(access ${ACCESS_CC_SOURCES})

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(access
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(access eo)
add_dependencies(check access)

add_test(Test_access access)
