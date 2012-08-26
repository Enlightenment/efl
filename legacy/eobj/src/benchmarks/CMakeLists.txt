LIST(APPEND EO_SUITE_CC_SOURCES
   eo_bench.c
   eo_bench_eo_do.c
   class_simple.c
   )

add_executable(eo_bench ${EO_SUITE_CC_SOURCES})

include_directories(
   ${EINA_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/src/lib
   )

get_target_property(eo_LIB_FILE eo LOCATION)
target_link_libraries(eo_bench
   ${EINA_LIBRARIES}
   ${EINA_LDFLAGS_OTHER}
   ${eo_LIB_FILE}
   )

add_dependencies(eo_bench eo)

get_target_property(eo_bench_EXEC_FILE eo_bench LOCATION)
add_custom_target(benchmark COMMAND ${eo_bench_EXEC_FILE} eo_bench)

add_dependencies(benchmark eo_bench)
