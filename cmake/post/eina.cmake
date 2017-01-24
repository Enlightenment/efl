
# TODO: change code to avoid these
if(EINA_MODULE_TYPE_MP_CHAINED_POOL STREQUAL "STATIC")
  CHECK_APPEND_DEFINE(EINA_STATIC_BUILD_CHAINED_POOL 1)
else()
  CHECK_APPEND_DEFINE(EINA_STATIC_BUILD_CHAINED_POOL "")
endif()
if(EINA_MODULE_TYPE_MP_ONE_BIG STREQUAL "STATIC")
  CHECK_APPEND_DEFINE(EINA_STATIC_BUILD_ONE_BIG 1)
else()
  CHECK_APPEND_DEFINE(EINA_STATIC_BUILD_ONE_BIG "")
endif()
if(EINA_MODULE_TYPE_MP_PASS_THROUGH STREQUAL "STATIC")
  CHECK_APPEND_DEFINE(EINA_STATIC_BUILD_PASS_THROUGH 1)
else()
  CHECK_APPEND_DEFINE(EINA_STATIC_BUILD_PASS_THROUGH "")
endif()

# generate the file and close the scope started with CHECK_INIT(eina):
EFL_HEADER_CHECKS_FINALIZE(${CMAKE_CURRENT_BINARY_DIR}/src/lib/eina/eina_config_gen.h)

# TODO: when autotools is gone, rename this file in repository
# and remove this copy (using generate to skip @-subst)
file(GENERATE
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/lib/eina/eina_config.h
  INPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/eina/eina_config.h.cmake
)
