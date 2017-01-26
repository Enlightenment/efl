set(DESCRIPTION "Library for speedy data storage, retrieval, and compression")

set(PKG_CONFIG_REQUIRES_PRIVATE
  zlib>=1.2.3
  libjpeg
)

set(PUBLIC_LIBRARIES
  eina
  emile
)

set(LIBRARIES
  m
)

set(PUBLIC_HEADERS
  Eet.h
)

set(SOURCES
  eet_alloc.c
  eet_cipher.c
  eet_connection.c
  eet_data.c
  eet_dictionary.c
  eet_image.c
  eet_lib.c
  eet_node.c
  Eet_private.h
  eet_utils.c
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/rg_etc1.c
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/rg_etc2.c
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/etc2_encoder.c
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/rg_etc1.h
)

set(INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc
)

if(WITH_CRYPTO STREQUAL "gnutls")
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE gnutls)
elseif(WITH_CRYPTO STREQUAL "openssl")
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE openssl)
endif()
