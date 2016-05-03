EOLIAN_FLAGS = -I$(srcdir)

include ../../Makefile_Eolian_Subbuild_Helper.am

### Library

ecore_eolian_files_legacy = \
	efl_timer.eo \
	ecore_poller.eo \
	ecore_job.eo \
	ecore_exe.eo \
	ecore_idler.eo \
	ecore_idle_enterer.eo \
	ecore_idle_exiter.eo \
	ecore_animator.eo

ecore_eolian_files = \
	$(ecore_eolian_files_legacy) \
	ecore_mainloop.eo \
	ecore_parent.eo

ecore_eolian_type_files = \
	ecore_types.eot

ecore_eolian_c = $(ecore_eolian_files:%.eo=%.eo.c)
ecore_eolian_h = $(ecore_eolian_files:%.eo=%.eo.h) \
                 $(ecore_eolian_files_legacy:%.eo=%.eo.legacy.h) \
                 $(ecore_eolian_type_files:%.eot=%.eot.h)

BUILT_SOURCES = $(ecore_eolian_c) \
                $(ecore_eolian_h)

ecoreeolianfilesdir = $(datadir)/eolian/include/ecore-@VMAJ@
ecoreeolianfiles_DATA = $(ecore_eolian_files) \
                        $(ecore_eolian_type_files)

lib_LTLIBRARIES = libecore.la

installed_ecoremainheadersdir = $(includedir)/ecore-@VMAJ@
dist_installed_ecoremainheaders_DATA = \
Ecore.h \
Ecore_Common.h \
Ecore_Legacy.h \
Ecore_Eo.h \
Ecore_Getopt.h

nodist_installed_ecoremainheaders_DATA = $(ecore_eolian_h)

libecore_la_SOURCES = \
ecore.c \
ecore_alloc.c \
ecore_anim.c \
ecore_app.c \
ecore_events.c \
ecore_getopt.c \
ecore_glib.c \
ecore_idle_enterer.c \
ecore_idle_exiter.c \
ecore_idler.c \
ecore_job.c \
ecore_main.c \
ecore_pipe.c \
ecore_poller.c \
ecore_time.c \
ecore_timer.c \
ecore_thread.c \
ecore_thread_promise.c \
ecore_throttle.c \
ecore_exe.c \
ecore_exe_private.h \
ecore_private.h

if HAVE_WIN32
libecore_la_SOURCES += ecore_exe_win32.c
else
libecore_la_SOURCES += ecore_signal.c ecore_exe_posix.c
endif

libecore_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl \
-DPACKAGE_BIN_DIR=\"$(bindir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_DATA_DIR=\"$(datadir)/ecore\" \
-DPACKAGE_BUILD_DIR=\"$(abs_top_builddir)\" \
@ECORE_CFLAGS@
libecore_la_LIBADD = @ECORE_SUBBUILD_LIBS@ @EVIL_SUBBUILD_LIBS@
libecore_la_DEPENDENCIES = @ECORE_SUBBUILD_INTERNAL_LIBS@
libecore_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@
