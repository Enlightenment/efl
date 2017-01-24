if(ENABLE_VALGRIND)
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE valgrind)
endif()

set(SOURCES eina_chained_mempool.c)
