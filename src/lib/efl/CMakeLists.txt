set(DESCRIPTION "EFL's general purpose library")

set(LIBRARIES eo eina)

set(PUBLIC_EO_FILES
  interfaces/efl_animator.eo
  interfaces/efl_canvas.eo
  interfaces/efl_canvas_pointer.eo
  interfaces/efl_config.eo
  interfaces/efl_container.eo
  interfaces/efl_content.eo
  interfaces/efl_control.eo
  interfaces/efl_file.eo
  interfaces/efl_gfx.eo
  interfaces/efl_gfx_buffer.eo
  interfaces/efl_gfx_fill.eo
  interfaces/efl_gfx_filter.eo
  interfaces/efl_gfx_gradient.eo
  interfaces/efl_gfx_gradient_linear.eo
  interfaces/efl_gfx_gradient_radial.eo
  interfaces/efl_gfx_path.eo
  interfaces/efl_gfx_shape.eo
  interfaces/efl_gfx_size_hint.eo
  interfaces/efl_gfx_stack.eo
  interfaces/efl_gfx_types.eot
  interfaces/efl_gfx_view.eo
  interfaces/efl_image.eo
  interfaces/efl_image_animated.eo
  interfaces/efl_image_load.eo
  interfaces/efl_input_device.eo
  interfaces/efl_input_types.eot
  interfaces/efl_io_buffer.eo
  interfaces/efl_io_closer.eo
  interfaces/efl_io_positioner.eo
  interfaces/efl_io_queue.eo
  interfaces/efl_io_reader.eo
  interfaces/efl_io_sizer.eo
  interfaces/efl_io_writer.eo
  interfaces/efl_model.eo
  interfaces/efl_observable.eo
  interfaces/efl_observer.eo
  interfaces/efl_orientation.eo
  interfaces/efl_pack.eo
  interfaces/efl_pack_table.eo
  interfaces/efl_pack_layout.eo
  interfaces/efl_pack_linear.eo
  interfaces/efl_part.eo
  interfaces/efl_player.eo
  interfaces/efl_screen.eo
  interfaces/efl_text.eo
  interfaces/efl_text_types.eot
  interfaces/efl_types.eot
  interfaces/efl_ui_range.eo
  interfaces/efl_ui_autorepeat.eo
  interfaces/efl_vpath.eo
  interfaces/efl_vpath_core.eo
  interfaces/efl_vpath_file.eo
  interfaces/efl_vpath_file_core.eo
  interfaces/efl_vpath_manager.eo
)

set(PUBLIC_HEADERS
  Efl.h
  Efl_Config.h
  Efl_Model_Common.h
  interfaces/efl_common_internal.h
)

set(SOURCES
  interfaces/efl_interfaces_main.c
  interfaces/efl_model_common.c
  interfaces/efl_gfx_path.c
  interfaces/efl_gfx_shape.c
  interfaces/efl_vpath_file.c
  interfaces/efl_vpath_manager.c
  interfaces/efl_vpath_core.c
  interfaces/efl_vpath_file_core.c
  interfaces/efl_input_device.c
  interfaces/efl_io_closer.c
  interfaces/efl_io_positioner.c
  interfaces/efl_io_reader.c
  interfaces/efl_io_sizer.c
  interfaces/efl_io_writer.c
  interfaces/efl_io_buffer.c
  interfaces/efl_io_queue.c
  interfaces/efl_observer.c
)

set(DEFINITIONS -DEFL_GFX_FILTER_BETA)

# XXX HACK: interfaces/efl_canvas.eo refers to Efl.Input.Focus!
set(EOLIAN_EXTRA_PARAMS -I${CMAKE_SOURCE_DIR}/src/lib/evas/canvas)
