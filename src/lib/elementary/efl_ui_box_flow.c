#include "efl_ui_box_private.h"

#define MY_CLASS EFL_UI_BOX_FLOW_CLASS

typedef struct _Efl_Ui_Box_Flow_Data Efl_Ui_Box_Flow_Data;

struct _Efl_Ui_Box_Flow_Data
{
   Eina_Bool homogenous;
   Eina_Bool max_size;
};

EOLIAN static void
_efl_ui_box_flow_box_flow_homogenous_set(Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd, Eina_Bool val)
{
   pd->homogenous = val;
}

EOLIAN static Eina_Bool
_efl_ui_box_flow_box_flow_homogenous_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd)
{
   return pd->homogenous;
}

EOLIAN static void
_efl_ui_box_flow_box_flow_max_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd, Eina_Bool val)
{
   pd->max_size = val;
}

EOLIAN static Eina_Bool
_efl_ui_box_flow_box_flow_max_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Flow_Data *pd)
{
   return pd->max_size;
}

EOLIAN static void
_efl_ui_box_flow_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Box_Flow_Data *pd)
{
   void (*func)(Evas_Box *obj, Evas_Object_Box_Data *priv, void *data);
   Evas_Object_Box_Data *bd;
   Eina_Bool homo = EINA_FALSE, maxsize = EINA_FALSE;

   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(obj, EFL_UI_BOX_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   bd = efl_data_scope_get(wd->resize_obj, EVAS_BOX_CLASS);

   homo = pd->homogenous;
   maxsize = pd->max_size;

   // This makes it horizontal by default, as opposed to the standard box.
   if (efl_ui_dir_is_horizontal(efl_ui_direction_get(obj), EINA_TRUE))
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
