if (CHECK_ENABLED)

   LIST(APPEND EOBJ_SUITE_CC_SOURCES
      eobj_suite.c
      eobj_test_init.c
      eobj_test_general.c
      )

   add_executable(eobj_suite ${EOBJ_SUITE_CC_SOURCES})

   include_directories(
      ${EINA_INCLUDE_DIRS}
      ${EVAS_INCLUDE_DIRS}
      ${ELEMENTARY_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/lib
      ${CHECK_INCLUDE_DIRS}
      )

   get_target_property(eobj_LIB_FILE eobj LOCATION)
   target_link_libraries(eobj_suite
      ${EINA_LIBRARIES}
      ${EVAS_LIBRARIES}
      ${ELEMENTARY_LIBRARIES}
      ${eobj_LIB_FILE}
      ${CHECK_LIBRARIES}
      )

   add_test(eobj_suite eobj_suite)
   add_dependencies(check eobj_suite)
endif (CHECK_ENABLED)
