#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_FOCUS_MANAGER_SUB_CLASS
#define MY_DATA(o, p) Efl_Ui_Focus_Manager_Sub_Data *pd = efl_data_scope_get(o, MY_CLASS);
typedef struct {
    Efl_Ui_Focus_Manager *manager;
    Efl_Ui_Focus_Manager *parent;
    Eina_Bool self_dirty;
    Eina_List *current_border;
} Efl_Ui_Focus_Manager_Sub_Data;

static Eina_List*
_set_a_without_b(Eina_List *a, Eina_List *b)
{
   Eina_List *a_out = NULL, *node;
   void *data;

   a_out = eina_list_clone(a);

   EINA_LIST_FOREACH(b, node, data)
     {
        a_out = eina_list_remove(a_out, data);
     }

   return a_out;
}

static void
_border_flush(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Eina_Iterator *borders;
   Eina_List *selection, *tmp;
   Efl_Ui_Focus_Object *node;

   borders = efl_ui_focus_manager_border_elements_get(obj);
   selection = efl_ui_focus_manager_sub_select_set(obj, borders);

   //elements which are not in the current border elements
   tmp = eina_list_clone(pd->current_border);
   tmp = _set_a_without_b(tmp , selection);

   EINA_LIST_FREE(tmp, node)
     {
        efl_ui_focus_manager_unregister(pd->manager, node);
     }

   //set of the elements which are new without those which are currently registered
   tmp = eina_list_clone(selection);
   tmp = _set_a_without_b(tmp, pd->current_border);

   EINA_LIST_FREE(tmp, node)
     {
        efl_ui_focus_manager_register(pd->manager, node, obj, obj);
     }

   eina_list_free(pd->current_border);
   pd->current_border = selection;
}

static void
_border_unregister(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Efl_Ui_Focus_Object *node;

   EINA_LIST_FREE(pd->current_border, node)
     {
        efl_ui_focus_manager_unregister(pd->manager, node);
     }

   pd->current_border = NULL;
}

static void
_parent_manager_pre_flush(void *data, const Efl_Event *ev EINA_UNUSED)
{
    MY_DATA(data, pd);

    if (!pd->self_dirty) return; //we are not interested

    _border_flush(data, pd);
}

EFL_CALLBACKS_ARRAY_DEFINE(parent_manager,
    {EFL_UI_FOCUS_MANAGER_EVENT_PRE_FLUSH, _parent_manager_pre_flush}
);

static void
_parent_set(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd, Efl_Ui_Focus_Manager *manager)
{
    if (pd->manager)
      {
         //remove ourself from the manager
         efl_ui_focus_manager_unregister(pd->manager, obj);


         efl_event_callback_array_del(pd->manager, parent_manager(), obj);
         _border_unregister(obj, pd);
      }

    pd->manager = manager;

    if (pd->manager)
      {
         //register our own root in the upper manager
         efl_ui_focus_manager_register(pd->manager, obj, pd->parent, obj);

         //listen to the manager
         efl_event_callback_array_add(pd->manager, parent_manager(), obj);
         _border_flush(obj, pd);
      }
}

static void
_self_parent_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
    MY_DATA(ev->object , pd);

    if (pd->manager == ev->info) return;

    _parent_set(ev->object, pd, ev->info);
}

static void
_self_manager_dirty(void *data EINA_UNUSED, const Efl_Event *ev)
{
    MY_DATA(ev->object , pd);

    pd->self_dirty = EINA_TRUE;
}

EFL_CALLBACKS_ARRAY_DEFINE(self_manager,
    {EFL_UI_FOCUS_USER_EVENT_MANAGER_CHANGED, _self_parent_change},
    {EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, _self_manager_dirty},
);
EOLIAN static void
_efl_ui_focus_manager_sub_parent_set(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd, Efl_Ui_Focus_Object *node)
{
   if (node == pd->parent) return;

   pd->parent = node;

   efl_ui_focus_manager_update_parent(pd->manager, obj, node);
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_sub_parent_get(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Manager_Sub_Data *pd EINA_UNUSED)
{
   return pd->parent;
}

EOLIAN static Efl_Object*
_efl_ui_focus_manager_sub_efl_object_constructor(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd EINA_UNUSED)
{
   efl_event_callback_array_add(obj, self_manager(), NULL);

   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_focus_manager_sub_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   efl_event_callback_array_del(obj, self_manager(), NULL);

   _parent_set(obj, pd, NULL);

   return efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object*
_efl_ui_focus_manager_sub_efl_object_finalize(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Efl_Ui_Focus_Manager *manager;

   manager = efl_ui_focus_user_manager_get(obj);

   _parent_set(obj, pd, manager);

   return efl_finalize(efl_super(obj, MY_CLASS));
}



#include "efl_ui_focus_manager_sub.eo.c"