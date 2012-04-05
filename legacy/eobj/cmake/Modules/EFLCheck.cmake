include(CTest)
ENABLE_TESTING()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)

find_package(Check)
set (CHECK_ENABLED ${CHECK_FOUND})

set(EFL_COVERAGE false CACHE BOOL "Whether coverage support should be built.'")
if (EFL_COVERAGE)
   include(EFLlcov REQUIRED)
   ENABLE_COVERAGE()
endif (EFL_COVERAGE)
