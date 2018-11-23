set(DESCRIPTION "Ecore event abstraction library")

set(PUBLIC_LIBRARIES
  efl
  eina
  eo
)

set(LIBRARIES
  m
)

if(ENABLE_SYSTEMD)
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE libsystemd)
endif()

if(ENABLE_GLIB)
  list(APPEND PKG_CONFIG_REQUIRES_PRIVATE glib-2.0 gthread-2.0)
endif()

set(PUBLIC_EO_FILES
  ecore_exe.eo
  ecore_poller.eo
  ecore_types.eot
  efl_io_buffered_stream.eo
  efl_io_closer_fd.eo
  efl_io_copier.eo
  efl_io_file.eo
  efl_io_positioner_fd.eo
  efl_io_reader_fd.eo
  efl_io_sizer_fd.eo
  efl_io_stderr.eo
  efl_io_stdin.eo
  efl_io_stdout.eo
  efl_io_writer_fd.eo
  efl_loop.eo
  efl_loop_fd.eo
  efl_loop_timer.eo
  efl_loop_user.eo
  efl_promise.eo
  efl_linear_interpolator.eo
  efl_accelerate_interpolator.eo
  efl_decelerate_interpolator.eo
  efl_sinusoidal_interpolator.eo
  efl_divisor_interpolator.eo
  efl_bounce_interpolator.eo
  efl_spring_interpolator.eo
  efl_cubic_bezier_interpolator.eo
)

set(PUBLIC_HEADERS
  Ecore.h
  Ecore_Common.h
  Ecore_Eo.h
  Ecore_Getopt.h
  Ecore_Legacy.h
)

set(SOURCES
  ecore.c
  ecore_alloc.c
  ecore_anim.c
  ecore_app.c
  ecore_events.c
  ecore_exe.c
  ecore_exe_private.h
  ecore_getopt.c
  ecore_glib.c
  ecore_idle_enterer.c
  ecore_idle_exiter.c
  ecore_idler.c
  ecore_internal.h
  ecore_job.c
  ecore_main.c
  ecore_pipe.c
  ecore_poller.c
  ecore_private.h
  ecore_thread.c
  ecore_throttle.c
  ecore_time.c
  ecore_timer.c
  efl_io_buffered_stream.c
  efl_io_closer_fd.c
  efl_io_copier.c
  efl_io_file.c
  efl_io_positioner_fd.c
  efl_io_reader_fd.c
  efl_io_sizer_fd.c
  efl_io_stderr.c
  efl_io_stdin.c
  efl_io_stdout.c
  efl_io_writer_fd.c
  efl_loop_fd.c
  efl_loop_user.c
  efl_promise.c
)

# TODO: win32
# TODO: ps3

list(APPEND SOURCES
  ecore_exe_posix.c
  ecore_signal.c
)
