AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib bin imported

if BUILD_MODULES
SUBDIRS += modules edje_externals
endif

SUBDIRS += examples

if ELEMENTARY_ENABLE_TESTS
SUBDIRS += tests
endif
