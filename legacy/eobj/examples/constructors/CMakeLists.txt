LIST(APPEND CONSTRUCTORS_CC_SOURCES
   main.c
   simple.c
   simple2.c
   simple3.c
   simple4.c
   simple5.c
   simple6.c
   mixin.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/lib
   )

add_executable(constructors ${CONSTRUCTORS_CC_SOURCES})

get_target_property(eobj_LIB_FILE eobj LOCATION)
target_link_libraries(constructors
   ${EINA_LIBRARIES}
   ${eobj_LIB_FILE}
   )

add_dependencies(constructors eobj)

add_test(Example_constructors constructors)
