AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I. \
-I$(top_builddir) \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lib \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@ELEMENTARY_CFLAGS@

lib_LTLIBRARIES = libelementary.la

include_HEADERS = Elementary.h

libelementary_la_SOURCES = \
elm_priv.h \
elm_main.c \
elm_obj.c \
elm_callback.c \
elm_win.c \
elm_theme.c \
elm_widget.c \
elm_bg.c \
elm_scroller.c \
elm_label.c \
elm_box.c \
elm_table.c \
elm_button.c \
elm_icon.c \
elm_toggle.c \
elm_clock.c \
elm_frame.c \
elm_pad.c \
elm_contactlist.c \
\
els_pan.c \
els_pan.h \
els_scroller.c \
els_scroller.h \
els_box.c \
els_box.h \
els_table.c \
els_table.h \
els_icon.c \
els_icon.h

libelementary_la_CFLAGS = 
libelementary_la_LIBADD = @my_libs@ @ELEMENTARY_LIBS@
libelementary_la_LDFLAGS = -version-info @version_info@
libelementary_la_DEPENDENCIES = $(top_builddir)/elementary_config.h

clean-local:
	@rm -rf *.gcno
	
