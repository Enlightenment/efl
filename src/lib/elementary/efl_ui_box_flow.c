#include "efl_ui_box_private.h"

#define MY_CLASS EFL_UI_BOX_FLOW_CLASS

typedef struct _Efl_Ui_Box_Flow_Data Efl_Ui_Box_Flow_Data;

struct _Efl_Ui_Box_Flow_Data
{
   Efl_Ui_Box_Flow_Params params;
};

EOLIAN static Eo *
_efl_ui_box_flow_eo_base_constructor(Eo *obj, Efl_Ui_Box_Flow_Data *pd)
{
   eo_constructor(eo_super(obj, MY_CLASS));
   efl_pack_layout_engine_set(obj, MY_CLASS, &pd->params);
   return obj;
}

EOLIAN static void
_efl_ui_box_flow_box_flow_homogenous_set(Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd, Eina_Bool val)
{
   pd->params.homogenous = val;
}

EOLIAN static Eina_Bool
_efl_ui_box_flow_box_flow_homogenous_get(Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd)
{
   return pd->params.homogenous;
}

EOLIAN static void
_efl_ui_box_flow_box_flow_max_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd, Eina_Bool val)
{
   pd->params.max_size = val;
}

EOLIAN static Eina_Bool
_efl_ui_box_flow_box_flow_max_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd)
{
   return pd->params.max_size;
}

EOLIAN static void
_efl_ui_box_flow_efl_pack_engine_layout_do(Eo_Class *klass EINA_UNUSED,
                                           void *_pd EINA_UNUSED,
                                           Eo *obj, const void *data)
{
   void (*func)(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data);
   const Efl_Ui_Box_Flow_Params *params = data;
   Evas_Object_Box_Data *bd;
   Efl_Ui_Box_Data *pd;
   Eina_Bool homo = EINA_FALSE, maxsize = EINA_FALSE;

   EINA_SAFETY_ON_FALSE_RETURN(eo_isa(obj, EFL_UI_BOX_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   bd = eo_data_scope_get(wd->resize_obj, EVAS_BOX_CLASS);
   pd = eo_data_scope_get(obj, EFL_UI_BOX_CLASS);

   if (params)
     {
        homo = params->homogenous;
        maxsize = params->max_size;
     }

   if (_horiz(pd->orient))
     {
        if (homo)
          {
             if (maxsize)
               func = evas_object_box_layout_homogeneous_max_size_horizontal;
             else
               func = evas_object_box_layout_homogeneous_horizontal;
          }
        else
          func = evas_object_box_layout_flow_horizontal;
     }
   else
     {
        if (homo)
          {
             if (maxsize)
               func = evas_object_box_layout_homogeneous_max_size_vertical;
             else
               func = evas_object_box_layout_homogeneous_vertical;
          }
        else
          func = evas_object_box_layout_flow_vertical;
     }

   func(wd->resize_obj, bd, NULL);
}

#include "efl_ui_box_flow.eo.c"
