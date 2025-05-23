eina_deps = [dl]
eina_pub_deps = [thread_dep, m]
eina_pc_deps = [m, rt, dl, thread_dep] #special case, this array is taken to drag in external deps to efl-one
eina_ext_deps = eina_pc_deps

if sys_windows == true
   eina_pub_deps += [evil]
endif

if sys_sun == true
   eina_deps += [socket]
endif

public_sub_headers = [
'eina_promise.h',
'eina_safety_checks.h',
'eina_error.h',
'eina_debug.h',
'eina_debug_private.h',
'eina_log.h',
'eina_inline_log.x',
'eina_fnmatch.h',
'eina_fp.h',
'eina_inline_f32p32.x',
'eina_inline_f16p16.x',
'eina_inline_f8p24.x',
'eina_inline_fp.x',
'eina_hash.h',
'eina_inline_hash.x',
'eina_lalloc.h',
'eina_clist.h',
'eina_inline_clist.x',
'eina_inarray.h',
'eina_inlist.h',
'eina_inline_inlist.x',
'eina_list.h',
'eina_file.h',
'eina_inline_file.x',
'eina_mempool.h',
'eina_module.h',
'eina_rectangle.h',
'eina_range.h',
'eina_types.h',
'eina_array.h',
'eina_counter.h',
'eina_inline_array.x',
'eina_magic.h',
'eina_stringshare.h',
'eina_binshare.h',
'eina_binbuf.h',
'eina_ustringshare.h',
'eina_inline_stringshare.x',
'eina_inline_ustringshare.x',
'eina_inline_list.x',
'eina_accessor.h',
'eina_convert.h',
'eina_rbtree.h',
'eina_benchmark.h',
'eina_inline_rbtree.x',
'eina_inline_mempool.x',
'eina_inline_rectangle.x',
'eina_inline_range.x',
'eina_inline_trash.x',
'eina_thread.h',
'eina_trash.h',
'eina_iterator.h',
'eina_main.h',
'eina_cpu.h',
'eina_inline_cpu.x',
'eina_tiler.h',
'eina_hamster.h',
'eina_matrixsparse.h',
'eina_inline_tiler.x',
'eina_str.h',
'eina_inline_str.x',
'eina_strbuf.h',
'eina_ustrbuf.h',
'eina_unicode.h',
'eina_quadtree.h',
'eina_simple_xml_parser.h',
'eina_lock.h',
'eina_prefix.h',
'eina_refcount.h',
'eina_mmap.h',
'eina_xattr.h',
'eina_value.h',
'eina_inline_value.x',
'eina_value_util.h',
'eina_inline_value_util.x',
'eina_inline_lock_barrier.x',
'eina_inline_lock_posix.x',
'eina_tmpstr.h',
'eina_alloca.h',
'eina_cow.h',
'eina_inline_unicode.x',
'eina_thread_queue.h',
'eina_matrix.h',
'eina_quad.h',
'eina_crc.h',
'eina_inline_crc.x',
'eina_evlog.h',
'eina_util.h',
'eina_inline_util.x',
'eina_quaternion.h',
'eina_vector.h',
'eina_inline_vector.x',
'eina_bezier.h',
'eina_safepointer.h',
'eina_inline_safepointer.x',
'eina_slice.h',
'eina_inline_slice.x',
'eina_inline_modinfo.x',
'eina_freeq.h',
'eina_slstr.h',
'eina_vpath.h',
'eina_abstract_content.h',
'eina_sha.h'
]

public_headers = [
  'Eina.h'
]

eina_src = files([
'eina_abi.c',
'eina_accessor.c',
'eina_array.c',
'eina_benchmark.c',
'eina_binbuf.c',
'eina_binshare.c',
'eina_convert.c',
'eina_counter.c',
'eina_cow.c',
'eina_cpu.c',
'eina_crc.c',
'eina_debug.c',
'eina_debug_bt.c',
'eina_debug_bt_file.c',
'eina_debug_chunk.c',
'eina_debug_thread.c',
'eina_debug_cpu.c',
'eina_debug_timer.c',
'eina_error.c',
'eina_evlog.c',
'eina_file_common.h',
'eina_file_common.c',
'eina_fnmatch.c',
'eina_fp.c',
'eina_hamster.c',
'eina_hash.c',
'eina_inarray.c',
'eina_inlist.c',
'eina_iterator.c',
'eina_lalloc.c',
'eina_list.c',
'eina_lock.c',
'eina_log.c',
'eina_magic.c',
'eina_main.c',
'eina_matrix.c',
'eina_matrixsparse.c',
'eina_mempool.c',
'eina_mmap.c',
'eina_module.c',
'eina_prefix.c',
'eina_promise.c',
'eina_promise_private.h',
'eina_quad.c',
'eina_quadtree.c',
'eina_rbtree.c',
'eina_rectangle.c',
'eina_safety_checks.c',
'eina_share_common.c',
'eina_simple_xml_parser.c',
'eina_str.c',
'eina_strbuf.c',
'eina_strbuf_common.c',
'eina_stringshare.c',
'eina_thread.c',
'eina_thread_queue.c',
'eina_tiler.c',
'eina_tmpstr.c',
'eina_unicode.c',
'eina_ustrbuf.c',
'eina_ustringshare.c',
'eina_util.c',
'eina_value.c',
'eina_value_util.c',
'eina_xattr.c',
'eina_private.h',
'eina_share_common.h',
'eina_strbuf_common.h',
'eina_quaternion.c',
'eina_bezier.c',
'eina_safepointer.c',
'eina_freeq.c',
'eina_slstr.c',
'eina_vpath.c',
'eina_vpath_xdg.c',
'eina_abstract_content.c',
'eina_sha.c',
]) + eina_mp_sources

if sys_windows == true
  eina_src += files('eina_file_win32.c', 'eina_win32_dllmain.c', 'eina_thread_win32.c')
else
  eina_src += files('eina_file_posix.c', 'eina_thread_posix.c')
endif

eina_config = configuration_data()

if get_option('buildtype') == 'debug'
     with_max_log_level=-1
     stringshare_usage=true
     use_valgrind=true
     debug_malloc=true
     debug_threads=true
     default_mempool=true
     want_cow_magic=true
elif get_option('buildtype') == 'release'
     with_max_log_level=3
     stringshare_usage=false
     use_valgrind=false
     debug_malloc=false
     debug_threads=false
     default_mempool=false
     want_cow_magic=false
 else
     with_max_log_level=-1
     stringshare_usage=false
     use_valgrind=true
     debug_malloc=false
     debug_threads=false
     default_mempool=false
     want_cow_magic=false
endif

if with_max_log_level != -1
   config_h.set('EINA_LOG_LEVEL_MAXIMUM', with_max_log_level)
endif

if stringshare_usage
   config_h.set('EINA_STRINGSHARE_USAGE', 1)
endif

if use_valgrind
   valgrind = dependency('valgrind', required: false)
   if valgrind.found() == false
     config_h.set('NVALGRIND', 1)
   else
     config_h.set('HAVE_VALGRIND', 1)
   endif
   eina_ext_deps += valgrind
else
   config_h.set('NVALGRIND', 1)
   valgrind = []
endif

if debug_malloc
   config_h.set('EINA_DEBUG_MALLOC', 1)
endif

if want_cow_magic
   config_h.set('EINA_COW_MAGIC_ON', 1)
endif

if get_option('eina-magic-debug')
   eina_config.set('EINA_MAGIC_DEBUG', '1')
endif

if default_mempool
   eina_config.set('EINA_DEFAULT_MEMPOOL', '1')
endif

if get_option('systemd') == true
  config_h.set('HAVE_SYSTEMD', '1')
endif


eina_config.set('EINA_SAFETY_CHECKS', '1')
eina_config.set('EINA_HAVE_THREADS', '1')


if cc.has_header_symbol('pthread.h', 'pthread_barrier_init')
   eina_config.set('EINA_HAVE_PTHREAD_BARRIER', '1')
endif

if sys_linux == true
   pthread_np_header_file = 'pthread.h'
   pthread_setname_np_api = 'pthread_setname_np'
   pthread_args = '-D_GNU_SOURCE'
else
   pthread_np_header_file = 'pthread_np.h'
   pthread_setname_np_api = 'pthread_set_name_np'
   pthread_args = []
endif

if cc.has_header_symbol(pthread_np_header_file, pthread_setname_np_api, args : pthread_args)
   eina_config.set('EINA_HAVE_PTHREAD_SETNAME', '1')
endif

if cc.has_header_symbol(pthread_np_header_file, 'pthread_attr_setaffinity_np', args : pthread_args)
   eina_config.set('EINA_HAVE_PTHREAD_AFFINITY', '1')
endif

if debug_threads and get_option('debug-threads')
   if cc.has_header_symbol('execinfo.h', 'backtrace')
      eina_config.set('EINA_HAVE_DEBUG_THREADS', '1')
   endif
endif

eina_config.set('EINA_SIZEOF_WCHAR_T', cc.sizeof('wchar_t',  prefix : '#include<wchar.h>'))

uintptr_size = cc.sizeof('uintptr_t',  prefix : '#include<stdint.h>')

eina_config.set('EINA_SIZEOF_UINTPTR_T', uintptr_size)
config_h.set('SIZEOF_UINTPTR_T', uintptr_size)

if cc.has_header('dirent.h')
   eina_config.set('EINA_CONFIGURE_HAVE_DIRENT_H', '1')
endif

eina_config.set('EINA_ENABLE_LOG', '1')

if cc.has_header_symbol('alloca.h', 'alloca')
   eina_config.set('EINA_HAVE_ALLOCA_H', '1')
endif

if cc.has_header('byteswap.h')
   eina_config.set('EINA_HAVE_BYTESWAP_H', '1')
endif

if cc.has_header_symbol('pthread.h', 'pthread_spin_init')
   eina_config.set('EINA_HAVE_POSIX_SPINLOCK', '1')
endif

if sys_osx == true
   if cc.has_header_symbol('libkern/OSAtomic.h', 'OSSpinLockTry')
     eina_config.set('EINA_HAVE_OSX_SPINLOCK', 1)
   endif
   if cc.has_header_symbol('mach/task.h', 'semaphore_create')
     eina_config.set('EINA_HAVE_OSX_SEMAPHORE', 1)
   endif
endif

if host_machine.endian() == 'big'
   eina_config.set('EINA_HAVE_WORDS_BIGENDIAN', '1')
endif

eina_config_file = configure_file(
  output: 'eina_config.h',
  configuration: eina_config,
  install_dir: dir_package_include)

public_headers += eina_config_file

unwind = dependency('libunwind-generic', required: false)
if unwind.found()
   config_h.set('HAVE_UNWIND', 1)
   eina_ext_deps += unwind
   config_h.set('EINA_LOG_BACKTRACE_ENABLE', 1)
endif

#for the case that the iconv library is not part of libc but rather libiconv or smth. like that
iconv = dependency('iconv', required: false)
if iconv.found() == false
  iconv = cc.find_library('iconv', required: false)
endif
eina_ext_deps += [iconv]

execinfo = cc.find_library('execinfo', required: false)
eina_ext_deps += [execinfo]

eina_c_args = [package_c_args, '-DEINA_BUILD']
if sys_windows == true
   bcrypt = cc.find_library('bcrypt', required: true)
   eina_ext_deps += [bcrypt]
   if iconv.found()
     eina_c_args += '-DWINICONV_CONST='
   endif
endif

eina_lib = library('eina', eina_src,
  c_args : eina_c_args,
  include_directories : config_dir,
  dependencies: eina_ext_deps + [eina_deps, evil],
  install: true,
  version : meson.project_version()
)


eina = declare_dependency(
  include_directories: [include_directories('.')] + config_dir,
  dependencies : eina_pub_deps + eina_deps,
  link_with: eina_lib,
)

install_headers(public_headers,
  install_dir : dir_package_include,
)

install_headers(public_sub_headers,
  install_dir : join_paths(dir_package_include, 'eina')
)

package_header_subdirs += 'eina'
#special case here to support -lm -ldl -threads
eina_pub_deps += eina_pc_deps
