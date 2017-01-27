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
  support-rg_etc
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
)

if(WITH_CRYPTO STREQUAL "gnutls")
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE gnutls)
elseif(WITH_CRYPTO STREQUAL "openssl")
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE openssl)
endif()
