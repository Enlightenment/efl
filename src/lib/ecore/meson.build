ecore_deps = [intl]
ecore_pub_deps = [eina, eo, efl]

pub_legacy_eo_files = [
  'ecore_exe.eo',
  'ecore_event_message.eo',
  'ecore_event_message_handler.eo',
  'efl_loop_timer.eo'
]

pub_eo_file_target = []
foreach eo_file : pub_legacy_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
  pub_eo_file_target += custom_target('eolian_gen_legacy_' + eo_file,
    input : eo_file,
    output : [eo_file + '.legacy.h'],
    depfile : eo_file + '.legacy.d',
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'l:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.h'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.legacy.d'),
                           '-gld', '@INPUT@'])
endforeach

pub_eo_files = [
  'efl_app.eo',
  'efl_loop.eo',
  'efl_loop_consumer.eo',
  'efl_loop_fd.eo',
  'efl_loop_handler.eo',
  'efl_loop_message.eo',
  'efl_loop_message_handler.eo',
  'efl_exe.eo',
  'efl_thread.eo',
  'efl_threadio.eo',
  'efl_appthread.eo',
  'efl_task.eo',
  'efl_io_closer_fd.eo',
  'efl_io_positioner_fd.eo',
  'efl_io_reader_fd.eo',
  'efl_io_sizer_fd.eo',
  'efl_io_writer_fd.eo',
  'efl_io_stdin.eo',
  'efl_io_stdout.eo',
  'efl_io_stderr.eo',
  'efl_io_file.eo',
  'efl_io_copier.eo',
  'efl_io_buffered_stream.eo',
  'efl_interpolator.eo',
  'efl_interpolator_linear.eo',
  'efl_interpolator_accelerate.eo',
  'efl_interpolator_decelerate.eo',
  'efl_interpolator_sinusoidal.eo',
  'efl_interpolator_divisor.eo',
  'efl_interpolator_bounce.eo',
  'efl_interpolator_spring.eo',
  'efl_interpolator_cubic_bezier.eo',
  'efl_loop_message_future_handler.eo',
  'efl_loop_message_future.eo',
  'efl_model_item.eo',
  'efl_model_container.eo',
  'efl_model_container_item.eo',
  'efl_model_composite_boolean.eo',
  'efl_model_composite_boolean_children.eo',
  'efl_model_composite_selection.eo',
  'efl_model_composite_selection_children.eo',
  'efl_model_composite.eo'
]

foreach eo_file : pub_eo_files
  pub_eo_file_target += custom_target('eolian_gen_' + eo_file,
    input : eo_file,
    output : [eo_file + '.h'],
    depfile : eo_file + '.d',
    install : true,
    install_dir : dir_package_include,
    command : [eolian_gen, '-I', meson.current_source_dir(), eolian_include_directories,
                           '-o', 'h:' + join_paths(meson.current_build_dir(), eo_file + '.h'),
                           '-o', 'c:' + join_paths(meson.current_build_dir(), eo_file + '.c'),
                           '-o', 'd:' + join_paths(meson.current_build_dir(), eo_file + '.d'),
                           '-gchd', '@INPUT@'])
endforeach

# special handling, because this is already eo API and legacy API
pub_eo_files += ['efl_loop_timer.eo']

pub_eo_types_files = []

eolian_include_directories += ['-I', meson.current_source_dir()]

ecore_header_src = [
  'Ecore.h',
  'Ecore_Common.h',
  'Ecore_Legacy.h',
  'Ecore_Eo.h',
  'Efl_Core.h',
  'efl_general.h',
  'Ecore_Getopt.h'
]

ecore_src = [
  'efl_app.c',
  'ecore.c',
  'ecore_alloc.c',
  'ecore_anim.c',
  'ecore_app.c',
  'ecore_events.c',
  'ecore_getopt.c',
  'ecore_glib.c',
  'ecore_idle_enterer.c',
  'ecore_idle_exiter.c',
  'ecore_idler.c',
  'ecore_job.c',
  'ecore_main.c',
  'ecore_event_message.c',
  'ecore_event_message_handler.c',
  'efl_loop.c',
  'efl_loop_consumer.c',
  'efl_loop_fd.c',
  'efl_loop_handler.c',
  'efl_loop_message.c',
  'efl_loop_message_future.c',
  'efl_loop_message_handler.c',
  'efl_loop_message_future_handler.c',
  'efl_task.c',
  'efl_io_closer_fd.c',
  'efl_io_positioner_fd.c',
  'efl_io_reader_fd.c',
  'efl_io_sizer_fd.c',
  'efl_io_writer_fd.c',
  'efl_io_stdin.c',
  'efl_io_stdout.c',
  'efl_io_stderr.c',
  'efl_io_file.c',
  'efl_io_copier.c',
  'efl_io_buffered_stream.c',
  'efl_model_item.c',
  'efl_model_container.c',
  'efl_model_container_item.c',
  'efl_model_container_private.h',
  'efl_model_composite.c',
  'efl_model_composite_boolean.c',
  'efl_model_composite_selection.c',
  'efl_model_accessor_view.c',
  'efl_model_accessor_view_private.h',
  'efl_interpolator.c',
  'efl_interpolator_linear.c',
  'efl_interpolator_accelerate.c',
  'efl_interpolator_decelerate.c',
  'efl_interpolator_sinusoidal.c',
  'efl_interpolator_divisor.c',
  'efl_interpolator_bounce.c',
  'efl_interpolator_spring.c',
  'efl_interpolator_cubic_bezier.c',
  'ecore_main_timechanges.c',
  'ecore_pipe.c',
  'ecore_poller.c',
  'ecore_time.c',
  'ecore_timer.c',
  'ecore_thread.c',
  'ecore_throttle.c',
  'ecore_exe.c',
  'ecore_exe_private.h',
  'ecore_private.h',
  'ecore_internal.h',
  'ecore_main_common.h'
]

if sys_windows == true
  #nothing for now ... needs testing
else
  ecore_src += ['ecore_signal.c', 'ecore_exe_posix.c']
endif

if get_option('glib') == true
  ecore_deps += dependency('glib-2.0')
  ecore_deps += dependency('gthread-2.0')
  config_h.set('GLIB_INTEGRATION_ALWAYS', '1')
  config_h.set('HAVE_GLIB', '1')
endif

if get_option('g-mainloop') == true
  config_h.set('USE_G_MAIN_LOOP', '1')
endif

if get_option('gstreamer') == true
  gstreamer = dependency('gstreamer-1.0')
  ecore_deps += gstreamer
endif

if get_option('systemd') == true
  ecore_deps += systemd
endif

ecore_lib = library('ecore',
    ecore_src, pub_eo_file_target,
    dependencies: ecore_pub_deps + [m, buildsystem, ecore_deps],
    include_directories : config_dir + [include_directories(join_paths('..','..'))],
    install: true,
    c_args : package_c_args,
    version : meson.project_version()
)

ecore = declare_dependency(
  include_directories: [include_directories('.')],
  link_with: ecore_lib,
  sources : pub_eo_file_target + priv_eo_file_target,
  dependencies: ecore_pub_deps,
)

eolian_ecore_dir = join_paths(eolian_include_dir, package_version_name)

install_data(pub_eo_files + pub_eo_types_files,
  install_dir: eolian_ecore_dir
)

install_headers(ecore_header_src,
  install_dir : dir_package_include,
)

#TODO:
# HAVE_TIZEN_CONFIGUREATION_MANAGER
# avahi
