#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#include "efl_ui_focus_composition_adapter.eo.h"

#define MY_CLASS EFL_UI_FOCUS_MANAGER_ROOT_FOCUS_CLASS

typedef struct {
   Efl_Ui_Focus_Object *root;

   Evas_Object *rect;
   Eina_Bool rect_registered;

   Eina_List *iterator_list;
} Efl_Ui_Focus_Manager_Root_Focus_Data;

static Efl_Ui_Focus_Object*
_trap(Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Object *obj)
{
   if (pd->rect == obj) return pd->root;
   return obj;
}

static void
_state_eval(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd)
{
   Efl_Ui_Focus_Object *root;
   Eina_Bool none_logical = EINA_FALSE, focused = EINA_FALSE;

   if (pd->rect_registered)
     {
        focused = efl_ui_focus_object_focus_get(pd->rect);
        efl_ui_focus_manager_calc_unregister(obj, pd->rect);
     }

   root = efl_ui_focus_manager_root_get(obj);
   none_logical = !!efl_ui_focus_manager_request_subchild(obj, root);

   if (none_logical)
     pd->rect_registered = EINA_FALSE;
   else
     {
        efl_ui_focus_manager_calc_register(obj, pd->rect, pd->root, NULL);
        pd->rect_registered = EINA_TRUE;

        if (focused)
          efl_ui_focus_manager_focus_set(obj, pd->rect);

     }
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_calc_register(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent, Efl_Ui_Focus_Manager *redirect)
{
   if (efl_ui_focus_manager_calc_register(efl_super(obj, MY_CLASS), child, parent, redirect))
     {
        if (child != pd->rect)
          _state_eval(obj, pd);

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_calc_register_logical(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Object *child, Efl_Ui_Focus_Object *parent, Efl_Ui_Focus_Manager *redirect)
{
   if (efl_ui_focus_manager_calc_register_logical(efl_super(obj, MY_CLASS), child, parent, redirect))
     {
        if (redirect)
          _state_eval(obj, pd);

        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_calc_unregister(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Object *child)
{
   efl_ui_focus_manager_calc_unregister(efl_super(obj, MY_CLASS), child);

   if (child != pd->rect)
     _state_eval(obj, pd);
}


EOLIAN static void
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_manager_focus_set(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Object *focus)
{
   EINA_SAFETY_ON_NULL_RETURN(focus);
   efl_ui_focus_manager_focus_set(efl_super(obj, MY_CLASS), _trap(pd, focus));
}


EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_manager_focus_get(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd)
{
   return _trap(pd, efl_ui_focus_manager_focus_get(efl_super(obj, MY_CLASS)));
}


EOLIAN static Efl_Ui_Focus_Relations *
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_fetch(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Object *child)
{
   return efl_ui_focus_manager_fetch(efl_super(obj, MY_CLASS), _trap(pd, child));
}


EOLIAN static Efl_Ui_Focus_Manager_Logical_End_Detail
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_logical_end(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd)
{
   Efl_Ui_Focus_Manager_Logical_End_Detail res;

   res = efl_ui_focus_manager_logical_end(efl_super(obj, MY_CLASS));

   res.element = _trap(pd, res.element);
   return res;
}

EOLIAN static Eina_Iterator *
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_border_elements_get(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd)
{
   if (pd->rect_registered)
     return eina_list_iterator_new(pd->iterator_list);

   return efl_ui_focus_manager_border_elements_get(efl_super(obj, MY_CLASS));
}


EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_request_move(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Direction direction)
{
   return _trap(pd, efl_ui_focus_manager_request_move(efl_super(obj, MY_CLASS), direction));
}


EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_root_focus_efl_ui_focus_manager_move(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd, Efl_Ui_Focus_Direction direction)
{
   return _trap(pd, efl_ui_focus_manager_move(efl_super(obj, MY_CLASS), direction));
}

EOLIAN static Efl_Object*
_efl_ui_focus_manager_root_focus_efl_object_finalize(Eo *obj, Efl_Ui_Focus_Manager_Root_Focus_Data *pd)
{
   Efl_Object *ret;

   ret = efl_finalize(efl_super(obj, MY_CLASS));

   pd->root = efl_ui_focus_manager_root_get(obj);

   pd->rect = efl_add(EFL_UI_FOCUS_COMPOSITION_ADAPTER_CLASS, pd->root);
   efl_ui_focus_composition_adapter_canvas_object_set(pd->rect, pd->root);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->rect, NULL);

   pd->iterator_list = eina_list_append(pd->iterator_list, pd->root);

   _state_eval(obj, pd);

   return ret;
}

#include "efl_ui_focus_manager_root_focus.eo.c"
