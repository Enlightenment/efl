pub_eo_file_target = []

pub_eo_files = [
  'eio_model.eo',
  'efl_io_manager.eo',
  'eio_sentry.eo'
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(
                           meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-gch', '@INPUT@'])
endforeach

pub_eo_types_files = []

eolian_include_directories += ['-I', meson.current_source_dir()]

eio_header_src = [
  'Eio.h',
  'Eio_Legacy.h',
  'Eio_Eo.h',
  'eio_inline_helper.x'
]

eio_src = [
  'eio_dir.c',
  'eio_eet.c',
  'eio_file.c',
  'efl_io_manager.c',
  'eio_sentry.c',
  'eio_main.c',
  'eio_map.c',
  'eio_monitor.c',
  'eio_monitor_poll.c',
  'eio_single.c',
  'eio_xattr.c',
  'eio_model.c',
  'eio_model_private.h',
  'eio_private.h',
  'eio_sentry_private.h'
]

linker_args = []

if target_machine.system() == 'windows'
  eio_src += ['eio_monitor_win32.c']
elif target_machine.system() == 'darwin'
  eio_src += ['eio_monitor_cocoa.c']
  linker_args = [['-framework', 'CoreServices']]
elif cc.has_header('sys/inotify.h')
  eio_src += ['eio_monitor_inotify.c']
elif target_machine.system() == 'bsd'
  eio_src += ['eio_monitor_kevent.c']
endif

eio_deps = [ecore, ecore_file, eet, eo, eina, efl, emile, efreet_mime]
eio_pub_deps = [eina, eet]

eio_lib = library('eio',
    eio_src, pub_eo_file_target,
    dependencies: [eina, eo, efl, m, eio_deps],
    include_directories : config_dir + [include_directories('.')],
    install: true,
    c_args : package_c_args,
    link_args : linker_args,
    version : meson.project_version()
)

eio = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: eio_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: eio_pub_deps,
)

install_data(pub_eo_files + pub_eo_types_files,
  install_dir: join_paths(eolian_include_dir, package_version_name)
)

install_headers(eio_header_src,
  install_dir : dir_package_include,
)
