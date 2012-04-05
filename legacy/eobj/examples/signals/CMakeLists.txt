LIST(APPEND SIGNALS_CC_SOURCES
   main.c
   simple.c
   )

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${EVAS_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/lib
   )

add_executable(signals ${SIGNALS_CC_SOURCES})

get_target_property(eobj_LIB_FILE eobj LOCATION)
target_link_libraries(signals
   ${EINA_LIBRARIES}
   ${eobj_LIB_FILE}
   )

add_dependencies(signals eobj)

add_test(Example_signals signals)
