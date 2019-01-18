#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_FOCUS_MANAGER_SUB_MIXIN
#define MY_DATA(o, p) Efl_Ui_Focus_Manager_Sub_Data *p = efl_data_scope_get(o, MY_CLASS);
typedef struct {
    Efl_Ui_Focus_Manager *manager;//the manager where current_border is currently registered
    Eina_Bool self_dirty;
    Eina_List *current_border; //the current set of widgets which is registered as borders
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
_register(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager *par_m, Efl_Ui_Focus_Object *node, Efl_Ui_Focus_Object *logical)
{
   if (par_m)
     efl_ui_focus_manager_calc_register(par_m, node, logical, obj);
}

static void
_unregister(Efl_Ui_Focus_Manager *obj EINA_UNUSED, Efl_Ui_Focus_Manager *par_m, Efl_Ui_Focus_Object *node)
{
   if (par_m)
     efl_ui_focus_manager_calc_unregister(par_m, node);
}

static void
_border_flush(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Eina_Iterator *borders;
   Eina_List *selection, *tmp;
   Efl_Ui_Focus_Object *node, *logical;
   Efl_Ui_Focus_Manager *manager;

   manager = efl_ui_focus_object_focus_manager_get(obj);
   logical = obj;
   borders = efl_ui_focus_manager_viewport_elements_get(obj, efl_gfx_entity_geometry_get(obj));

   selection = NULL;
   EINA_ITERATOR_FOREACH(borders, node)
     {
        selection = eina_list_append(selection, node);
     }
   eina_iterator_free(borders);

   //elements which are not in the current border elements
   tmp = _set_a_without_b(pd->current_border, selection);

   EINA_LIST_FREE(tmp, node)
     {
        if (node == obj) continue;
        _unregister(obj, manager, node);
     }

   //set of the elements which are new without those which are currently registered
   tmp = _set_a_without_b(selection, pd->current_border);

   EINA_LIST_FREE(tmp, node)
     {
        if (node == obj) continue;
        _register(obj, manager, node, logical);
     }

   eina_list_free(pd->current_border);
   pd->current_border = selection;
   pd->self_dirty = EINA_FALSE;
}

static void
_border_unregister(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Efl_Ui_Focus_Object *node;
   Efl_Ui_Focus_Manager *manager;

   manager = efl_ui_focus_object_focus_manager_get(obj);
   EINA_LIST_FREE(pd->current_border, node)
     {
        if (node == obj) continue;
        _unregister(obj, manager, node);
     }
}

static void
_parent_manager_pre_flush(void *data, const Efl_Event *ev EINA_UNUSED)
{
    MY_DATA(data, pd);

    if (!pd->self_dirty) return; //we are not interested

    _border_flush(data, pd);
}

static void
_redirect_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   //if (efl_ui_focus_manager_redirect_get(ev->object) != data) return;

   MY_DATA(data, pd);

   _border_flush(data, pd);
}

static void
_freeze_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   if (ev->info)
     {
        efl_ui_focus_manager_dirty_logic_freeze(data);
     }
   else
     {
        efl_ui_focus_manager_dirty_logic_unfreeze(data);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(parent_manager,
    {EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, _parent_manager_pre_flush},
    {EFL_UI_FOCUS_MANAGER_EVENT_REDIRECT_CHANGED, _redirect_changed_cb},
    {EFL_UI_FOCUS_MANAGER_EVENT_DIRTY_LOGIC_FREEZE_CHANGED, _freeze_changed_cb}
);

static void
_self_manager_dirty(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
    MY_DATA(data, pd);

    pd->self_dirty = EINA_TRUE;
}

static void
_logical_manager_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
   MY_DATA(data, pd);
   Eina_List *n;
   Efl_Ui_Focus_Object *b;
   Efl_Ui_Focus_Manager *manager;

   if (!ev->info) return;

   manager = efl_ui_focus_object_focus_manager_get(ev->object);
   EINA_LIST_FOREACH(pd->current_border, n, b)
     {
        if (b == ev->object) continue;
        efl_ui_focus_manager_calc_update_parent(manager, b, ev->info);
     }
}

static void
_flush_manager(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Efl_Ui_Focus_Manager *manager;
   Efl_Ui_Focus_Object *logical;
   Efl_Ui_Focus_Object *b;
   Eina_List *n;

   logical = efl_ui_focus_object_focus_parent_get(obj);
   manager = efl_ui_focus_object_focus_manager_get(obj);

   if (pd->manager == manager)
     return;

   //unregister from the old
   if (pd->manager) efl_event_callback_array_del(pd->manager, parent_manager(), obj);
   if (manager) efl_event_callback_array_add(manager, parent_manager(), obj);

   EINA_LIST_FOREACH(pd->current_border , n, b)
     {
        if (b == obj) continue;

        _unregister(obj, pd->manager, b);
        _register(obj, manager, b, logical);
     }
   //unregister the old manager, use the new
   pd->manager = manager;
}

static void
_manager_change(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA(data, pd);

   _flush_manager(data, pd);
}

EFL_CALLBACKS_ARRAY_DEFINE(self_manager,
    {EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, _self_manager_dirty},
    {EFL_UI_FOCUS_OBJECT_EVENT_LOGICAL_CHANGED, _logical_manager_change},
    {EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, _manager_change}
);

EOLIAN static Efl_Object*
_efl_ui_focus_manager_sub_efl_object_constructor(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_event_callback_array_add(obj, self_manager(), obj);

   pd->self_dirty = EINA_TRUE;

   return obj;
}

EOLIAN static void
_efl_ui_focus_manager_sub_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   _border_unregister(obj, pd);

   if (pd->manager) efl_event_callback_array_del(pd->manager, parent_manager(), obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_manager_sub_efl_ui_focus_manager_move(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Eo *target = efl_ui_focus_manager_move(efl_super(obj, MY_CLASS), direction);

   if (!target)
     _border_flush(obj, pd);

   return target;
}



#include "efl_ui_focus_manager_sub.eo.c"
