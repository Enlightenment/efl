macro(ENABLE_COVERAGE)
   if (CHECK_ENABLED)
      find_program(LCOV_BINARY lcov HINTS ${EFL_LCOV_PATH})
      find_program(GENHTML_BINARY genhtml HINTS ${EFL_LCOV_PATH})
      set(EFL_COVERAGE_CFLAGS "-fprofile-arcs -ftest-coverage")
      set(EFL_COVERAGE_LIBS "gcov")
   endif (CHECK_ENABLED)

   if (DEFINED LCOV_BINARY)
      set(EFL_COVERAGE_ENABLED true)
      add_custom_target(lcov-reset
         COMMAND rm -rf ${CMAKE_BINARY_DIR}/coverage
         COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
         COMMAND ${LCOV_BINARY} --zerocounters --directory ${CMAKE_BINARY_DIR}
         )

      add_custom_target(lcov-report
         COMMAND mkdir ${CMAKE_BINARY_DIR}/coverage
         COMMAND ${LCOV_BINARY} --capture --compat-libtool --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info --directory ${CMAKE_BINARY_DIR}
         COMMAND ${LCOV_BINARY} --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info '*.h' --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.cleaned.info
         COMMAND ${GENHTML_BINARY} -t "${PACKAGE}" -o "${CMAKE_BINARY_DIR}/coverage/html" "${CMAKE_BINARY_DIR}/coverage/coverage.cleaned.info"
         COMMAND echo "Coverage Report at ${CMAKE_BINARY_DIR}/coverage/html"
         )

      add_custom_target(coverage
         COMMAND ${CMAKE_MAKE_PROGRAM} lcov-reset
         COMMAND ${CMAKE_MAKE_PROGRAM} check
         COMMAND ${CMAKE_MAKE_PROGRAM} lcov-report
         )
   endif (DEFINED LCOV_BINARY)
endmacro(ENABLE_COVERAGE)
