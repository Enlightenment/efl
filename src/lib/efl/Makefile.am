EOLIAN_FLAGS = -I$(srcdir)

include ../../Makefile_Eolian_Subbuild_Helper.am

# Even though libefl is new, it contains definitions used by legacy APIs
efl_eolian_legacy_files = \
interfaces/efl_gfx_fill.eo \
interfaces/efl_gfx.eo \
interfaces/efl_image.eo \
interfaces/efl_image_animated.eo \
$(NULL)

efl_eolian_files = \
interfaces/efl_control.eo \
interfaces/efl_file.eo \
interfaces/efl_image_load.eo \
interfaces/efl_player.eo \
interfaces/efl_text.eo \
interfaces/efl_text_properties.eo \
interfaces/efl_gfx_stack.eo \
interfaces/efl_gfx_view.eo \
interfaces/efl_gfx_buffer.eo \
interfaces/efl_gfx_shape.eo \
interfaces/efl_gfx_gradient.eo \
interfaces/efl_gfx_gradient_linear.eo \
interfaces/efl_gfx_gradient_radial.eo \
interfaces/efl_gfx_filter.eo \
interfaces/efl_gfx_size_hint.eo \
interfaces/efl_model.eo \
interfaces/efl_model_item.eo \
interfaces/efl_model_container.eo \
interfaces/efl_model_container_item.eo \
interfaces/efl_animator.eo \
interfaces/efl_orientation.eo \
interfaces/efl_flipable.eo \
interfaces/efl_vpath.eo \
interfaces/efl_vpath_manager.eo \
interfaces/efl_vpath_file.eo \
interfaces/efl_vpath_core.eo \
interfaces/efl_vpath_file_core.eo \
interfaces/efl_ui_spin.eo \
interfaces/efl_ui_progress.eo \
interfaces/efl_screen.eo \
$(efl_eolian_legacy_files) \
$(NULL)

efl_eolian_type_files = \
	interfaces/efl_gfx_types.eot \
	interfaces/efl_types.eot \
	$(NULL)

efl_eolian_files_h = $(efl_eolian_files:%.eo=%.eo.h) \
   $(efl_eolian_type_files:%.eot=%.eot.h)
efl_eolian_files_c = $(efl_eolian_files:%.eo=%.eo.c)
efl_eolian_legacy_files_h = $(efl_eolian_legacy_files:%.eo=%.eo.legacy.h)

BUILT_SOURCES = \
     $(efl_eolian_files_c) \
     $(efl_eolian_files_h) \
     $(efl_eolian_legacy_files_h)

efleolianfilesdir = $(datadir)/eolian/include/efl-@VMAJ@
efleolianfiles_DATA = $(efl_eolian_files) $(efl_eolian_type_files)

lib_LTLIBRARIES = libefl.la

libefl_la_SOURCES = \
interfaces/efl_interfaces_main.c \
interfaces/efl_model_common.c \
interfaces/efl_model_item.c \
interfaces/efl_model_container.c \
interfaces/efl_model_container_item.c \
interfaces/efl_gfx_shape.c \
interfaces/efl_vpath_file.c \
interfaces/efl_vpath_manager.c \
interfaces/efl_vpath_core.c \
interfaces/efl_vpath_file_core.c

libefl_la_CPPFLAGS = -I$(top_builddir)/src/lib/efl -I$(top_srcdir)/src/lib/efl @EFL_CFLAGS@ -DEFL_GFX_FILTER_BETA
libefl_la_LIBADD = @EFL_SUBBUILD_LIBS@
libefl_la_DEPENDENCIES = @EFL_SUBBUILD_INTERNAL_LIBS@
libefl_la_LDFLAGS = @EFL_LTLIBRARY_FLAGS@

installed_eflheadersdir = $(includedir)/efl-@VMAJ@
dist_installed_eflheaders_DATA = \
Efl_Config.h \
Efl_Model_Common.h \
Efl.h

installed_eflinterfacesdir = $(includedir)/efl-@VMAJ@/interfaces
nodist_installed_eflinterfaces_DATA = \
	$(efl_eolian_files_h) \
	$(efl_eolian_legacy_files_h)
