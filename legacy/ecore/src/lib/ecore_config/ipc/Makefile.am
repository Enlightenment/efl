
## Process this file with automake to produce Makefile.in
AUTOMAKE_OPTIONS     = 1.4 foreign

# A list of all the files in the current directory which can be regenerated
MAINTAINERCLEANFILES = Makefile.in

LDFLAGS              = -module -avoid-version -rdynamic -shared
INCLUDES             = -I$(top_srcdir)/src\
		       -I$(top_srcdir)/src/lib/ecore_ipc \
		       -I../../ -I../ -I.

IPC_ECORE_L = ipc_ecore.la
EVSH_L =

lib_LTLIBRARIES               = ecore_config_ipc_ecore.la

ecore_config_ipc_ecore_la_DEPENDENCIES     = $(top_builddir)/config.h 
ecore_config_ipc_ecore_la_SOURCES          = ipc_ecore.c
ecore_config_ipc_ecore_la_LIBADD           = \
  $(top_builddir)/src/lib/ecore/libecore.la \
  $(top_builddir)/src/lib/ecore_ipc/libecore_ipc.la
ecore_config_ipc_ecore_la_LDFLAGS          = -no-undefined -module -avoid-version

