LIST(APPEND MIXIN_CC_SOURCES
   main.c
   simple.c
   mixin.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/lib
   )

add_executable(mixin ${MIXIN_CC_SOURCES})

get_target_property(eobj_LIB_FILE eobj LOCATION)
target_link_libraries(mixin
   ${EINA_LIBRARIES}
   ${eobj_LIB_FILE}
   )

add_dependencies(mixin eobj)

add_test(Example_mixin mixin)
