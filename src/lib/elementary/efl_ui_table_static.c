#include "efl_ui_table_private.h"

#define MY_CLASS EFL_UI_TABLE_STATIC_CLASS
#define MY_CLASS_NAME "Efl.Ui.Table_Static"
#define MY_CLASS_NAME_LEGACY "elm_grid"

EOLIAN static Eo *
_efl_ui_table_static_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   Efl_Ui_Table_Data *gd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   gd = efl_data_scope_get(obj, EFL_UI_TABLE_CLASS);
   gd->req_cols = 100;
   gd->req_rows = 100;

   return obj;
}

EOLIAN static void
_efl_ui_table_static_efl_pack_layout_layout_update(Eo *obj, void *_pd EINA_UNUSED)
{
   Efl_Ui_Table_Data *gd;
   Table_Item *gi;
   Evas *e;
   long long xl, yl, wl, hl, vwl, vhl;
   Eina_Bool mirror;
   Eina_Rect r;

   gd = efl_data_scope_get(obj, EFL_UI_TABLE_CLASS);
   if (!gd->items) return;

   e = evas_object_evas_get(obj);
   efl_event_freeze(e);

   r = efl_gfx_entity_geometry_get(obj);
   xl = r.x;
   yl = r.y;
   wl = r.w;
   hl = r.h;
   mirror = efl_ui_mirrored_get(obj);

   if (!gd->req_cols || !gd->req_rows)
     {
        WRN("Table_Static size must be set before using! Default to 100x100.");
        efl_pack_table_size_set(obj, 100, 100);
        if (!gd->req_cols || !gd->req_rows) goto err;
     }
   vwl = gd->req_cols;
   vhl = gd->req_rows;

   EINA_INLIST_FOREACH(gd->items, gi)
     {
        long long x1, y1, x2, y2;

        if (!mirror)
          {
             x1 = xl + ((wl * (long long)gi->col) / vwl);
             x2 = xl + ((wl * (long long)(gi->col + gi->col_span)) / vwl);
          }
        else
          {
             x1 = xl + ((wl * (vwl - (long long)(gi->col + gi->col_span))) / vwl);
             x2 = xl + ((wl * (vwl - (long long)gi->col)) / vwl);
          }
        y1 = yl + ((hl * (long long)gi->row) / vhl);
        y2 = yl + ((hl * (long long)(gi->row + gi->row_span)) / vhl);
        efl_gfx_entity_geometry_set(gi->object, EINA_RECT(x1, y1, x2 - x1, y2 - y1));
     }
err:
   efl_event_thaw(e);
}

#include "efl_ui_table_static.eo.c"
