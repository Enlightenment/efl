#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

static Evas_Public_Data *
_efl_canvas_output_async_block(Efl_Canvas_Output *output)
{
   Evas_Public_Data *e;

   if (!output->canvas) return NULL;
   e = efl_data_scope_get(output->canvas, EVAS_CANVAS_CLASS);
   if (!e) return NULL;

   evas_canvas_async_block(e);

   return e;
}

void
efl_canvas_output_info_get(Evas_Public_Data *e, Efl_Canvas_Output *output)
{
   if (output->info) return;
   if (!e->engine.func->info_size)
     {
        CRI("Engine not up to date no info size provided.");
        return ;
     }

   output->info = calloc(1, e->engine.func->info_size);
   if (!output->info) return;
   output->info->magic = rand();
   output->info_magic = output->info->magic;

   if (e->engine.func->output_info_setup)
     e->engine.func->output_info_setup(output->info);
}

EAPI Efl_Canvas_Output *
efl_canvas_output_add(Evas *canvas)
{
   Efl_Canvas_Output *r;
   Evas_Public_Data *e;

   if (!efl_isa(canvas, EVAS_CANVAS_CLASS)) return NULL;

   r = calloc(1, sizeof (Efl_Canvas_Output));
   if (!r) return NULL;

   efl_wref_add(canvas, &r->canvas);
   r->changed = EINA_TRUE;

   e = _efl_canvas_output_async_block(r);
   if (!e)
     {
        efl_wref_del(canvas, &r->canvas);
        free(r);
        return NULL;
     }

   // Track this output in Evas
   e->outputs = eina_list_append(e->outputs, r);

   // The engine is already initialized, use it
   // right away to setup the info structure
   efl_canvas_output_info_get(e, r);

   return r;
}

EAPI void
efl_canvas_output_del(Efl_Canvas_Output *output)
{
   if (output->canvas)
     {
        Evas_Public_Data *e;

        e = _efl_canvas_output_async_block(output);
        if (!e) goto on_error;

        if (e->engine.func)
          {
             e->engine.func->output_free(_evas_engine_context(e),
                                         output->output);
             free(output->info);
             output->info = NULL;
          }
        e->outputs = eina_list_remove(e->outputs, output);

        efl_wref_del(output->canvas, &output->canvas);
     }

 on_error:
   free(output);
}

EAPI void
efl_canvas_output_view_set(Efl_Canvas_Output *output,
                           Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Public_Data *e;

   e = _efl_canvas_output_async_block(output);
   if (!e) return;

   if (output->geometry.x != x) goto changed;
   if (output->geometry.y != y) goto changed;
   if (output->geometry.w != w) goto changed;
   if (output->geometry.h != h) goto changed;
   return;

 changed:
   output->geometry.x = x;
   output->geometry.y = y;
   output->geometry.w = w;
   output->geometry.h = h;
   output->changed = EINA_TRUE;
   // XXX: tell evas to add damage if viewport loc/size changed
}

EAPI void
efl_canvas_output_view_get(Efl_Canvas_Output *output,
                           Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (x) *x = output->geometry.x;
   if (y) *y = output->geometry.y;
   if (w) *w = output->geometry.w;
   if (h) *h = output->geometry.h;
}

EAPI Eina_Bool
efl_canvas_output_engine_info_set(Efl_Canvas_Output *output,
                                  Evas_Engine_Info *info)
{
   Evas_Public_Data *e;

   e = _efl_canvas_output_async_block(output);
   if (!e) return EINA_FALSE;
   if (output->info != info) return EINA_FALSE;
   if (info->magic != output->info_magic) return EINA_FALSE;

   if (output->output)
     {
        if (e->engine.func->output_update)
          {
             e->engine.func->output_update(_evas_engine_context(e), output->output, info,
                                           output->geometry.w, output->geometry.h);
          }
        else
          {
             // For engine who do not provide an update function
             e->engine.func->output_free(_evas_engine_context(e),
                                         output->output);

             goto setup;
          }
     }
   else
     {
     setup:
        output->output = 
          e->engine.func->output_setup(_evas_engine_context(e), info,
                                       output->geometry.w, output->geometry.h);
     }

   return !!output->output;
}

EAPI Evas_Engine_Info*
efl_canvas_output_engine_info_get(Efl_Canvas_Output *output)
{
   Evas_Engine_Info *info = output->info;

   if (!info) return NULL;

   output->info_magic = info->magic;
   return output->info;
}

EAPI Eina_Bool
efl_canvas_output_lock(Efl_Canvas_Output *output)
{
   output->lock++;
   return EINA_TRUE;
}

EAPI Eina_Bool
efl_canvas_output_unlock(Efl_Canvas_Output *output)
{
   return !!(--output->lock);
}
