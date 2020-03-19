#include "edje_private.h"
#include "edje_part_helper.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_PART_INVALID_CLASS

PROXY_IMPLEMENTATION(invalid, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

static void
_edje_part_invalid_call(const Eo *proxy, const char *function)
{
   PROXY_DATA_GET(proxy, pd);

   WRN("No such part '%s' in group '%s' in call to %s(). Ignored.",
       pd->part, pd->ed ? pd->ed->group : NULL, function);
   efl_event_callback_call(pd->obj, EFL_LAYOUT_EVENT_PART_INVALID, (void *) pd->part);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define EDJE_PART_INVALID_VOID(api, impl, ...) \
   EOLIAN static void impl(Eo *proxy, void *_pd EINA_UNUSED, ## __VA_ARGS__) { _edje_part_invalid_call(proxy, #api); }

#define EDJE_PART_INVALID_VOID_CONST(api, impl, ...) \
   EOLIAN static void impl(const Eo *proxy, void *_pd EINA_UNUSED, ## __VA_ARGS__) { _edje_part_invalid_call(proxy, #api); }

#define EDJE_PART_INVALID(type, ret, api, impl, ...) \
   EOLIAN static type impl(Eo *proxy, void *_pd EINA_UNUSED, ## __VA_ARGS__) { _edje_part_invalid_call(proxy, #api); return (type)ret; }

#define EDJE_PART_INVALID_CONST(type, ret, api, impl, ...) \
   EOLIAN static type impl(const Eo *proxy, void *_pd EINA_UNUSED, ## __VA_ARGS__) { _edje_part_invalid_call(proxy, #api); return (type)ret; }

EOLIAN static void
_efl_canvas_layout_part_invalid_efl_canvas_layout_part_state_get(const Eo *proxy, void *_pd EINA_UNUSED, const char **state, double *val)
{
   if (state) *state = "";
   if (val) *val = 0.0;
}

EOLIAN static Efl_Canvas_Layout_Part_Type
_efl_canvas_layout_part_invalid_efl_canvas_layout_part_type_provider_part_type_get(const Eo *proxy EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return EFL_CANVAS_LAYOUT_PART_TYPE_NONE;
}

EDJE_PART_INVALID_CONST(Eina_Rect, EINA_RECT_ZERO(), efl_gfx_entity_geometry_get, _efl_canvas_layout_part_invalid_efl_gfx_entity_geometry_get)
EDJE_PART_INVALID(Eina_Bool, 0, efl_ui_drag_value_set, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_value_set, double dx, double dy)
EDJE_PART_INVALID_CONST(Eina_Bool, 0, efl_ui_drag_value_get, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_value_get, double *dx, double *dy)
EDJE_PART_INVALID(Eina_Bool, 0, efl_ui_drag_size_set, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_size_set, double dw, double dh)
EDJE_PART_INVALID_CONST(Eina_Bool, 0, efl_ui_drag_size_get, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_size_get, double *dw, double *dh)
EDJE_PART_INVALID_CONST(Efl_Ui_Drag_Dir, 0, efl_ui_drag_dir_get, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_dir_get)
EDJE_PART_INVALID(Eina_Bool, 0, efl_ui_drag_step_set, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_step_set, double dx, double dy)
EDJE_PART_INVALID_CONST(Eina_Bool, 0, efl_ui_drag_step_get, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_step_get, double *dx, double *dy)
EDJE_PART_INVALID(Eina_Bool, 0, efl_ui_drag_step_move, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_step_move, double dx, double dy)
EDJE_PART_INVALID(Eina_Bool, 0, efl_ui_drag_page_set, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_page_set, double dx, double dy)
EDJE_PART_INVALID_CONST(Eina_Bool, 0, efl_ui_drag_page_get, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_page_get, double *dx, double *dy)
EDJE_PART_INVALID(Eina_Bool, 0, efl_ui_drag_page_move, _efl_canvas_layout_part_invalid_efl_ui_drag_drag_page_move, double dx, double dy)
EDJE_PART_INVALID(Eina_Bool, 0, efl_content_set, _efl_canvas_layout_part_invalid_efl_content_content_set, Efl_Gfx_Entity *content)
EDJE_PART_INVALID_CONST(Eo *, 0, efl_content_get, _efl_canvas_layout_part_invalid_efl_content_content_get)
EDJE_PART_INVALID(Eo *, 0, efl_content_unset, _efl_canvas_layout_part_invalid_efl_content_content_unset)
EDJE_PART_INVALID_VOID(efl_text_set, _efl_canvas_layout_part_invalid_efl_text_text_set, const char *text)
EDJE_PART_INVALID_CONST(const char *, 0, efl_text_get, _efl_canvas_layout_part_invalid_efl_text_text_get)
EDJE_PART_INVALID_VOID(efl_text_markup_set, _efl_canvas_layout_part_invalid_efl_text_markup_markup_set, const char *text)
EDJE_PART_INVALID_CONST(const char *, 0, efl_text_markup_get, _efl_canvas_layout_part_invalid_efl_text_markup_markup_get)
EDJE_PART_INVALID(Eina_Iterator *, 0, efl_content_iterate, _efl_canvas_layout_part_invalid_efl_container_content_iterate)
EDJE_PART_INVALID(int, 0, efl_content_count, _efl_canvas_layout_part_invalid_efl_container_content_count)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_clear, _efl_canvas_layout_part_invalid_efl_pack_pack_clear)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_unpack_all, _efl_canvas_layout_part_invalid_efl_pack_unpack_all)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_unpack, _efl_canvas_layout_part_invalid_efl_pack_unpack, Efl_Gfx_Entity *subobj)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack, _efl_canvas_layout_part_invalid_efl_pack_pack, Efl_Gfx_Entity *subobj)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_begin, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_begin, Efl_Gfx_Entity *subobj)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_end, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_end, Efl_Gfx_Entity *subobj)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_before, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_before, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_after, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_after, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_at, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_at, Efl_Gfx_Entity *subobj, int index)
EDJE_PART_INVALID(Efl_Gfx_Entity *, 0, efl_pack_content_get, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_content_get, int index)
EDJE_PART_INVALID(Efl_Gfx_Entity *, 0, efl_pack_unpack_at, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_unpack_at, int index)
EDJE_PART_INVALID(int, 0, efl_pack_index_get, _efl_canvas_layout_part_invalid_efl_pack_linear_pack_index_get, const Efl_Gfx_Entity *subobj)
EDJE_PART_INVALID_CONST(Efl_Ui_Layout_Orientation, 0, efl_ui_layout_orientation_get, _efl_canvas_layout_part_invalid_efl_ui_layout_orientable_orientation_get)
EDJE_PART_INVALID(Eina_Bool, 0, efl_pack_table, _efl_canvas_layout_part_invalid_efl_pack_table_pack_table, Efl_Gfx_Entity *subobj, int col, int row, int colspan, int rowspan)
EDJE_PART_INVALID(Efl_Gfx_Entity *, 0, efl_pack_table_content_get, _efl_canvas_layout_part_invalid_efl_pack_table_table_content_get, int col, int row)
EDJE_PART_INVALID(Eina_Iterator *, 0, efl_pack_table_contents_get, _efl_canvas_layout_part_invalid_efl_pack_table_table_contents_get, int col, int row, Eina_Bool below)
EDJE_PART_INVALID_CONST(Eina_Bool, 0, efl_pack_table_cell_column_get, _efl_canvas_layout_part_invalid_efl_pack_table_table_cell_column_get, Efl_Gfx_Entity *subobj, int *col, int *colspan)
EDJE_PART_INVALID_VOID(efl_pack_table_cell_column_set, _efl_canvas_layout_part_invalid_efl_pack_table_table_cell_column_set, Efl_Gfx_Entity *subobj, int col, int colspan)
EDJE_PART_INVALID_CONST(Eina_Bool, 0, efl_pack_table_cell_row_get, _efl_canvas_layout_part_invalid_efl_pack_table_table_cell_row_get, Efl_Gfx_Entity *subobj, int *row, int *rowspan)
EDJE_PART_INVALID_VOID(efl_pack_table_cell_row_set, _efl_canvas_layout_part_invalid_efl_pack_table_table_cell_row_set, Efl_Gfx_Entity *subobj, int row, int rowspan)
EDJE_PART_INVALID_VOID_CONST(efl_pack_table_size_get, _efl_canvas_layout_part_invalid_efl_pack_table_table_size_get, int *cols, int *rows)
EDJE_PART_INVALID_CONST(int, 0, efl_pack_table_columns_get, _efl_canvas_layout_part_invalid_efl_pack_table_table_columns_get)
EDJE_PART_INVALID_CONST(int, 0, efl_pack_table_rows_get, _efl_canvas_layout_part_invalid_efl_pack_table_table_rows_get)

#pragma GCC diagnostic pop

#include "efl_canvas_layout_part_invalid.eo.c"
