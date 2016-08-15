#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#define MY_CLASS EVAS_OUT_CLASS
typedef struct _Evas_Out_Data Evas_Out_Data;
struct _Evas_Out_Data
{
   void *info;/*, *context, *output;*/
   Evas_Coord x, y, w, h;
};

EAPI Evas_Out *
evas_out_add(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = efl_add(MY_CLASS, e);
   return eo_obj;
}

EOLIAN static Eo *
_evas_out_efl_object_constructor(Eo *eo_obj, Evas_Out_Data *eo_dat)
{
   Eo *eo_parent = NULL;
   Evas_Public_Data *e;

   eo_parent = efl_parent_get(eo_obj);
   e = efl_data_scope_get(eo_parent, EVAS_CANVAS_CLASS);
   evas_canvas_async_block(e);

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   if (!e) return NULL;
   e->outputs = eina_list_append(e->outputs, eo_obj);
   if (e->engine.func->info) eo_dat->info = e->engine.func->info(eo_parent);
   // XXX: context and output are currently held in the core engine and are
   // allocated by engine specific internal code. this all needs a new engine
   // api to make it work

   return eo_obj;
}

EAPI void
evas_output_del(Evas_Out *evo)
{
   efl_unref(evo);
}

EOLIAN static void
_evas_out_efl_object_destructor(Eo *eo_obj, Evas_Out_Data *eo_dat)
{
   Eo *eo_parent = NULL;
   Evas_Public_Data *e;

   eo_parent = efl_parent_get(eo_obj);
   e = efl_data_scope_get(eo_parent, EVAS_CANVAS_CLASS);
   evas_canvas_async_block(e);
   if (!e) return;
   // XXX: need to free output and context one they get allocated one day
   // e->engine.func->context_free(eo_dat->output, eo_dat->context);
   // e->engine.func->output_free(eo_dat->output);
   e->engine.func->info_free(eo_parent, eo_dat->info);
   e->outputs = eina_list_remove(e->outputs, eo_obj);
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

EOLIAN static void
_evas_out_view_set(Eo *eo_e, Evas_Out_Data *eo_dat, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Eo *eo_parent = NULL;
   Evas_Public_Data *e;
   eo_parent = efl_parent_get(eo_e);
   e = efl_data_scope_get(eo_parent, EVAS_CANVAS_CLASS);
   evas_canvas_async_block(e);
   eo_dat->x = x;
   eo_dat->y = y;
   eo_dat->w = w;
   eo_dat->h = h;
   // XXX: tell engine about any output size etc. changes
   // XXX: tell evas to add damage if viewport loc/size changed
}

EOLIAN static void
_evas_out_view_get(Eo *eo_e EINA_UNUSED, Evas_Out_Data *eo_dat, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (x) *x = eo_dat->x;
   if (y) *y = eo_dat->y;
   if (w) *w = eo_dat->w;
   if (h) *h = eo_dat->h;
}

EOLIAN static Eina_Bool
_evas_out_engine_info_set(Eo *eo_e, Evas_Out_Data *eo_dat, Evas_Engine_Info *info)
{
   Eo *eo_parent = NULL;
   Evas_Public_Data *e;
   eo_parent = efl_parent_get(eo_e);
   e = efl_data_scope_get(eo_parent, EVAS_CANVAS_CLASS);
   evas_canvas_async_block(e);
   if (eo_dat->info != info) return EINA_FALSE;

   // XXX: handle setting of engine info here
  
   return EINA_TRUE;
}

EOLIAN static Evas_Engine_Info*
_evas_out_engine_info_get(Eo *eo_e EINA_UNUSED, Evas_Out_Data *eo_dat)
{
   return eo_dat->info;
}

#include "canvas/evas_out.eo.c"
