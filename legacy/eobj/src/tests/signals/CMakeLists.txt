LIST(APPEND SIGNALS_CC_SOURCES
   main.c
   simple.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

add_executable(signals ${SIGNALS_CC_SOURCES})

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(signals
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(signals eo)
add_dependencies(check signals)

add_test(Test_signals signals)
