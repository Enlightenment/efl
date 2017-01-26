set(DESCRIPTION "Library for simplified serialization, compression and ciphering")

set(PKG_CONFIG_REQUIRES_PRIVATE
  zlib>=1.2.3
  libjpeg
)

set(LIBRARIES
  eina
  m
)

set(PUBLIC_HEADERS
  Emile.h
  emile_base64.h
  emile_cipher.h
  emile_compress.h
  emile_image.h
)

set(SOURCES
  emile_base64.c
  emile_compress.c
  emile_image.c
  emile_main.c
  emile_private.h
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/etc2_encoder.c
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/rg_etc1.c
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/rg_etc1.h
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc/rg_etc2.c
)

set(INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/src/static_libs/rg_etc
)

if(WITH_CRYPTO STREQUAL "gnutls")
  list(APPEND SOURCES emile_cipher_gnutls.c)
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE gnutls)
elseif(WITH_CRYPTO STREQUAL "openssl")
  list(APPEND SOURCES emile_cipher_openssl.c)
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE openssl)
else()
  list(APPEND SOURCES emile_cipher.c)
endif()

if(WITH_LZ4 STREQUAL "system")
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE liblz4)
else()
  list(APPEND SOURCES
    ${CMAKE_SOURCE_DIR}/src/static_libs/lz4/lz4.c
    ${CMAKE_SOURCE_DIR}/src/static_libs/lz4/lz4.h
    ${CMAKE_SOURCE_DIR}/src/static_libs/lz4/lz4hc.c
    ${CMAKE_SOURCE_DIR}/src/static_libs/lz4/lz4hc.h
  )
  list(APPEND INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/static_libs/lz4)
endif()
