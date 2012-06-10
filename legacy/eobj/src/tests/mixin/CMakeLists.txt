LIST(APPEND MIXIN_CC_SOURCES
   main.c
   simple.c
   mixin.c
   mixin2.c
   mixin3.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

add_executable(mixin ${MIXIN_CC_SOURCES})

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(mixin
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(mixin eo)

add_test(Example_mixin mixin)
