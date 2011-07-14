MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = lib bin tests examples modules

if BUILD_ECORE_X_XCB
  MAINTAINERCLEANFILES += ecore_xcb_keysym_table.h ecore_xcb_keysym_table_h
  CLEANFILES = ecore_xcb_keysym_table.h ecore_xcb_keysym_table_h

  SUBDIRS += util

  BUILT_SOURCES=ecore_xcb_keysym_table.h

  #
  # Building ecore_xcb_keysym_table.h requires the makekeys utility
  #

  ecore_xcb_keysym_table.h: $(KEYSYMDEFS) $(top_builddir)/src/util/makekeys$(EXEEXT)
	  $(top_builddir)/src/util/makekeys $(KEYSYMDEFS) > ecore_xcb_keysym_table_h
	  mv -f ecore_xcb_keysym_table_h ./lib/ecore_x/xcb/$@

  $(top_builddir)/src/util/makekeys$(EXEEXT): force
	  cd util && $(MAKE)

  force:

endif
