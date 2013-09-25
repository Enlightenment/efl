#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#include <Eo.h>

EAPI Eo_Op EVAS_OUT_BASE_ID = EO_NOOP;
#define MY_CLASS EVAS_OUT_CLASS
typedef struct _Evas_Out_Public_Data Evas_Out_Public_Data;
struct _Evas_Out_Public_Data
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
   Evas_Object *eo_obj = eo_add(MY_CLASS, e);
   eo_unref(eo_obj);
   return eo_obj;
}

static void
_constructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Out_Public_Data *eo_dat = _pd;
   Eo *eo_parent;
   Evas_Public_Data *e;

   eo_do(eo_obj, eo_parent_get(&eo_parent));
   e = eo_data_scope_get(eo_parent, EVAS_CLASS);

   eo_do_super(eo_obj, MY_CLASS, eo_constructor());

   if (!e) return;
   e->outputs = eina_list_append(e->outputs, eo_obj);
   if (e->engine.func->info) eo_dat->info = e->engine.func->info(eo_parent);
   // XXX: context and output are currently held in the core engine and are
   // allocated by engine specific internal code. this all needs a new engine
   // api to make it work
}

EAPI void
evas_output_del(Evas_Out *evo)
{
   eo_unref(evo);
}

static void
_destructor(Eo *eo_obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Out_Public_Data *eo_dat = _pd;
   Eo *eo_parent;
   Evas_Public_Data *e;

   eo_do(eo_obj, eo_parent_get(&eo_parent));
   e = eo_data_scope_get(eo_parent, EVAS_CLASS);
   // XXX: need to free output and context one they get allocated one day   
   // e->engine.func->context_free(eo_dat->output, eo_dat->context);
   // e->engine.func->output_free(eo_dat->output);
   e->engine.func->info_free(eo_parent, eo_dat->info);
   if (e)
     {
        e->outputs = eina_list_remove(e->outputs, eo_obj);
     }
   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

EAPI void
evas_output_view_set(Evas_Out *evo, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   eo_do((Eo *)evo, evas_out_view_set(x, y, w, h));
}

static void
_output_view_set(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Out_Public_Data *eo_dat = _pd;
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   eo_dat->x = x;
   eo_dat->y = y;
   eo_dat->w = w;
   eo_dat->h = h;
   // XXX: tell engine about any output size etc. changes
   // XXX: tell evas to add damage if viewport loc/size changed
}

EAPI void
evas_output_view_get(const Evas_Out *evo, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   eo_do((Eo *)evo, evas_out_view_get(x, y, w, h));
}

static void
_output_view_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Out_Public_Data *eo_dat = _pd;
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   if (x) *x = eo_dat->x;
   if (y) *y = eo_dat->y;
   if (w) *w = eo_dat->w;
   if (h) *h = eo_dat->h;
}

EAPI Eina_Bool
evas_output_engine_info_set(Evas_Out *evo, Evas_Engine_Info *info)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do(evo, evas_out_engine_info_set(info, &ret));
   return ret;
}

static void
_output_engine_info_set(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Out_Public_Data *eo_dat = _pd;
   Evas_Engine_Info *info = va_arg(*list, Evas_Engine_Info *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (eo_dat->info != info)
     {
        *ret = EINA_FALSE;
        return;
     }
   // XXX: handle setting of engine info here
   if (ret) *ret = EINA_TRUE;
}

EAPI Evas_Engine_Info *
evas_output_engine_info_get(const Evas_Out *evo)
{
   Evas_Engine_Info *ret = NULL;
   eo_do((Eo *)evo, evas_out_engine_info_get(&ret));
   return ret;
}

static void
_output_engine_info_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Out_Public_Data *eo_dat = _pd;
   Evas_Engine_Info **ret = va_arg(*list, Evas_Engine_Info **);
   if (ret) *ret = eo_dat->info;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
      EO_OP_FUNC(EVAS_OUT_ID(EVAS_OUT_SUB_ID_VIEW_SET), _output_view_set),
      EO_OP_FUNC(EVAS_OUT_ID(EVAS_OUT_SUB_ID_VIEW_GET), _output_view_get),
      EO_OP_FUNC(EVAS_OUT_ID(EVAS_OUT_SUB_ID_ENGINE_INFO_SET), _output_engine_info_set),
      EO_OP_FUNC(EVAS_OUT_ID(EVAS_OUT_SUB_ID_ENGINE_INFO_GET), _output_engine_info_get),
      EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
   EO_OP_DESCRIPTION(EVAS_OUT_SUB_ID_VIEW_SET, "Set the canvas viewport region for the output"),
   EO_OP_DESCRIPTION(EVAS_OUT_SUB_ID_VIEW_GET, "Get the canvas viewport region for the output"),
   EO_OP_DESCRIPTION(EVAS_OUT_SUB_ID_ENGINE_INFO_SET, "Set engine specific information for rendering to the given output"),
   EO_OP_DESCRIPTION(EVAS_OUT_SUB_ID_ENGINE_INFO_GET, "Get engine specific information for rendering to the given output"),
   EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
   EO_VERSION,
   "Evas_Out",
   EO_CLASS_TYPE_REGULAR,
   EO_CLASS_DESCRIPTION_OPS(&EVAS_OUT_BASE_ID, op_desc, EVAS_OUT_SUB_ID_LAST),
   NULL,
   sizeof(Evas_Out_Public_Data),
   _class_constructor,
   NULL
};

EO_DEFINE_CLASS(evas_out_class_get, &class_desc, EO_BASE_CLASS, NULL);
