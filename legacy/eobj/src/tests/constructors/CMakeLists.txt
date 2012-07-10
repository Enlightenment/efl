LIST(APPEND CONSTRUCTORS_CC_SOURCES
   main.c
   simple.c
   simple2.c
   simple3.c
   simple4.c
   simple5.c
   simple6.c
   simple7.c
   mixin.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

add_executable(constructors ${CONSTRUCTORS_CC_SOURCES})

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(constructors
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(constructors eo)
add_dependencies(check constructors)

add_test(Test_constructors constructors)
