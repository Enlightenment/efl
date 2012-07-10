if (CHECK_ENABLED)

   LIST(APPEND EO_SUITE_CC_SOURCES
      eo_suite.c
      eo_test_init.c
      eo_test_general.c
      eo_test_class_errors.c
      class_simple.c
      )

   add_executable(eo_suite ${EO_SUITE_CC_SOURCES})

   include_directories(
      ${EINA_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/src/lib
      ${CHECK_INCLUDE_DIRS}
      )

   get_target_property(eo_LIB_FILE eo LOCATION)
   target_link_libraries(eo_suite
      ${EINA_LIBRARIES}
      ${EINA_LDFLAGS_OTHER}
      ${eo_LIB_FILE}
      ${CHECK_LIBRARIES}
      )

   add_test(eo_suite eo_suite)
   add_dependencies(eo_suite eo)
   add_dependencies(check eo_suite)
endif (CHECK_ENABLED)
