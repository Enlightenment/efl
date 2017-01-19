include_directories(
${PROJECT_SOURCE_DIR}/
${PROJECT_SOURCE_DIR}/src/lib/eina
${PROJECT_SOURCE_DIR}/src/lib/efl
${VG_INCLUDE_DIRS}
)

if (EINA_STATIC_BUILD_PASS_THROUGH)
    add_library(pass_through MODULE "mp/pass_through/eina_pass_through.c")
endif(EINA_STATIC_BUILD_PASS_THROUGH)


if (EINA_STATIC_BUILD_ONE_BIG)
    add_library(one_big MODULE mp/one_big/eina_one_big.c)
endif(EINA_STATIC_BUILD_ONE_BIG)


if (EINA_STATIC_BUILD_CHAINED_POOL)
    add_library(chained_pool MODULE mp/chained_pool/eina_chained_mempool.c)
endif(EINA_STATIC_BUILD_CHAINED_POOL)