INCLUDE (CheckSymbolExists)
INCLUDE (CheckIncludeFile)
INCLUDE (CheckTypeSize)
INCLUDE (FindPkgConfig)
find_package(Threads REQUIRED)

#Configure depending on the profile
set(EINA_SAFETY_CHECKS 1)
if ("${BUILD_PROFILE}" STREQUAL "dev")
    set(EINA_STRINGSHARE_USAGE ON)
    set(CHECK_VALGRIND ON)
elseif ("${BUILD_PROFILE}" STREQUAL "debug")
    set(EINA_DEBUG_MALLOC ON)
    set(EINA_COW_MAGIC_ON ON)
    set(EINA_DEFAULT_MEMPOOL ON)
    set(EINA_DEBUG_MALLOC ON)
    set(EINA_DEBUG_THREADS ON)
    set(VALGRIND ON)
elseif ("${BUILD_PROFILE}" STREQUAL "release")
    set(EINA_LOG_LEVEL_MAXIMUM 3)
endif ("${BUILD_PROFILE}" STREQUAL "dev")

#options of that submodule
option(EINA_MAGIC_DEBUG "magic debug of eina structure" ON)
option(EINA_DEBUG_THREADS "debugging of eina threads" ${EINA_DEBUG_THREADS})
option(VALGRIND "valgrind support" ${VALGRIND})

#check for symbols in pthread
#TODO Make the definitions depending on the platform
set(CMAKE_REQUIRED_FLAGS "${CMAKE_THREAD_LIBS_INIT}")
set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE=1")
CHECK_SYMBOL_EXISTS(pthread_barrier_init pthread.h EINA_HAVE_PTHREAD_BARRIER)
CHECK_SYMBOL_EXISTS(pthread_attr_setaffinity_np pthread.h EINA_HAVE_PTHREAD_AFFINITY)
CHECK_SYMBOL_EXISTS(pthread_setname_np pthread.h EINA_HAVE_PTHREAD_SETNAME)
CHECK_SYMBOL_EXISTS(pthread_spin_init pthread.h EINA_HAVE_POSIX_SPINLOCK)

#check for eina header files that are required
CHECK_INCLUDE_FILE(alloca.h EINA_HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE(byteswap.h EINA_HAVE_BYTESWAP_H)
CHECK_INCLUDE_FILE(fnmatch.h EINA_HAVE_FNMATCH_H)
#only enable log when fnmatch is there
if (EINA_HAVE_FNMATCH_H)
  set(EINA_ENABLE_LOG 1)
endif (EINA_HAVE_FNMATCH_H)
CHECK_INCLUDE_FILE(dirent.h EINA_HAVE_DIRENT_H)

CHECK_TYPE_SIZE("wchar_t" EINA_SIZEOF_WCHAR_T)
CHECK_TYPE_SIZE("uintptr_t" EINA_SIZEOF_UINTPTR_T)
CHECK_TYPE_SIZE("wchar_t" EINA_SIZEOF_WCHAR_T)

#check for swap16/32/64
CHECK_SYMBOL_EXISTS(bswap_16 byteswap.h EINA_HAVE_BSWAP16)
CHECK_SYMBOL_EXISTS(bswap_32 byteswap.h EINA_HAVE_BSWAP32)
CHECK_SYMBOL_EXISTS(bswap_64 byteswap.h EINA_HAVE_BSWAP64)

#TODO really check if we wanted to have valgrind support or not
pkg_check_modules(VG REQUIRED valgrind)

#Check if there is the alloca header

configure_file(eina_config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/eina_config.h)

include_directories(
    ${PROJECT_SOURCE_DIR}/
    ${VG_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/lib/efl
    ${PROJECT_SOURCE_DIR}/src/lib/eina
)

LIST(APPEND eina_source
    Eina.h
    eina_abi.c
    eina_accessor.c
    eina_accessor.h
    eina_alloca.h
    eina_array.c
    eina_array.h
    eina_benchmark.c
    eina_benchmark.h
    eina_bezier.c
    eina_bezier.h
    eina_binbuf.c
    eina_binbuf.h
    eina_binshare.c
    eina_binshare.h
    eina_clist.h
    eina_config.h
    eina_convert.c
    eina_convert.h
    eina_counter.c
    eina_counter.h
    eina_cow.c
    eina_cow.h
    eina_cpu.c
    eina_cpu.h
    eina_crc.c
    eina_crc.h
    eina_debug.c
    eina_debug.h
    eina_debug_bt.c
    eina_debug_bt_file.c
    eina_debug_chunk.c
    eina_debug_monitor.c
    eina_debug_proto.c
    eina_debug_thread.c
    eina_error.c
    eina_error.h
    eina_evlog.c
    eina_evlog.h
    eina_file.c
    eina_file.h
    eina_file_common.c
    eina_file_common.h
    eina_fp.c
    eina_fp.h
    eina_freeq.c
    eina_freeq.h
    eina_hamster.c
    eina_hamster.h
    eina_hash.c
    eina_hash.h
    eina_inarray.c
    eina_inarray.h
    eina_inline_private.h
    eina_inlist.c
    eina_inlist.h
    eina_iterator.c
    eina_iterator.h
    eina_lalloc.c
    eina_lalloc.h
    eina_list.c
    eina_list.h
    eina_lock.c
    eina_lock.h
    eina_log.c
    eina_log.h
    eina_magic.c
    eina_magic.h
    eina_main.c
    eina_main.h
    eina_matrix.c
    eina_matrix.h
    eina_matrixsparse.c
    eina_matrixsparse.h
    eina_mempool.c
    eina_mempool.h
    eina_mmap.c
    eina_mmap.h
    eina_module.c
    eina_module.h
    eina_prefix.c
    eina_prefix.h
    eina_private.h
    eina_quad.c
    eina_quad.h
    eina_quadtree.c
    eina_quadtree.h
    eina_quaternion.c
    eina_quaternion.h
    eina_rbtree.c
    eina_rbtree.h
    eina_rectangle.c
    eina_rectangle.h
    eina_refcount.h
    eina_safepointer.c
    eina_safepointer.h
    eina_safety_checks.c
    eina_safety_checks.h
    eina_sched.c
    eina_sched.h
    eina_share_common.c
    eina_share_common.h
    eina_simple_xml_parser.c
    eina_simple_xml_parser.h
    eina_slice.h
    eina_slstr.c
    eina_slstr.h
    eina_str.c
    eina_str.h
    eina_strbuf.c
    eina_strbuf.h
    eina_strbuf_common.c
    eina_strbuf_common.h
    eina_strbuf_template_c.x
    eina_stringshare.c
    eina_stringshare.h
    eina_thread.c
    eina_thread.h
    eina_thread_queue.c
    eina_thread_queue.h
    eina_tiler.c
    eina_tiler.h
    eina_tmpstr.c
    eina_tmpstr.h
    eina_trash.h
    eina_types.h
    eina_unicode.c
    eina_unicode.h
    eina_ustrbuf.c
    eina_ustrbuf.h
    eina_ustringshare.c
    eina_ustringshare.h
    eina_util.c
    eina_util.h
    eina_value.c
    eina_value.h
    eina_value_util.c
    eina_value_util.h
    eina_vector.h
    eina_xattr.c
    eina_xattr.h
)

add_library(eina SHARED ${eina_source})