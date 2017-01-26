pkg_check_modules(LZ4 liblz4)

set(_choices "internal")
set(_defval "internal")
if(LZ4_FOUND)
  set(_choices system internal)
  set(_defval system)
endif()

EFL_OPTION(WITH_LZ4 "Choose if system or internal liblz4 should be used" ${_defval} CHOICE ${_choices})
