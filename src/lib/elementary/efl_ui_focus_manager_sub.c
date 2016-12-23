#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_FOCUS_MANAGER_SUB_CLASS
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
_focus_changed(void *data, const Efl_Event *event)
{
   Efl_Ui_Focus_Manager_Sub *m = data;
   Efl_Ui_Focus_Manager *manager;
   Elm_Widget *elem;

   elem = efl_parent_get(m);
   manager = efl_ui_focus_user_manager_get(elem);

   //only do this when we are getting focus
   if (!event->info) return;

   //if there is already the redirect, ignore this focus request
   if (efl_ui_focus_manager_redirect_get(manager)) return;

   efl_ui_focus_manager_focus(m, event->object);
}

static void
_register(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager *par_m, Efl_Ui_Focus_Object *node, Efl_Ui_Focus_Object *logical)
{
   efl_ui_focus_manager_register(par_m, node, logical, obj);
   efl_event_callback_add(node, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED, _focus_changed, obj);
}

static void
_unregister(Efl_Ui_Focus_Manager *obj, Efl_Ui_Focus_Manager *par_m, Efl_Ui_Focus_Object *node)
{
   efl_ui_focus_manager_unregister(par_m, node);
   efl_event_callback_del(node, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED, _focus_changed, obj);
}

static void
_border_flush(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Eina_Iterator *borders;
   Eina_List *selection, *tmp;
   Efl_Ui_Focus_Object *node, *logical;
   Efl_Ui_Focus_Manager *manager;
   Efl_Ui_Focus_Manager *elem;

   elem = efl_parent_get(obj);
   manager = efl_ui_focus_user_manager_get(elem);
   logical = elem;
   borders = efl_ui_focus_manager_border_elements_get(obj);

   selection = NULL;
   EINA_ITERATOR_FOREACH(borders, node)
     {
        selection = eina_list_append(selection, node);
     }
   eina_iterator_free(borders);

   //elements which are not in the current border elements
   tmp = eina_list_clone(pd->current_border);
   tmp = _set_a_without_b(tmp , selection);

   EINA_LIST_FREE(tmp, node)
     {
        if (node == elem) continue;
        _unregister(obj, manager, node);
     }

   //set of the elements which are new without those which are currently registered
   tmp = eina_list_clone(selection);
   tmp = _set_a_without_b(tmp, pd->current_border);

   EINA_LIST_FREE(tmp, node)
     {
        if (node == elem) continue;
        _register(obj, manager, node, logical);
     }

   eina_list_free(pd->current_border);
   pd->current_border = selection;
}

static void
_border_unregister(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   Efl_Ui_Focus_Object *node;
   Efl_Ui_Focus_Manager *manager;
   Efl_Ui_Focus_Manager *elem;

   elem = efl_parent_get(obj);

   manager = efl_ui_focus_user_manager_get(elem);
   EINA_LIST_FREE(pd->current_border, node)
     {
        if (node == elem) continue;
        _unregister(obj, manager, node);
     }
}

static void
_parent_manager_pre_flush(void *data, const Efl_Event *ev EINA_UNUSED)
{
    MY_DATA(data, pd);

    //if (!pd->self_dirty) return; //we are not interested

    _border_flush(data, pd);
}

static void
_redirect_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   //if (efl_ui_focus_manager_redirect_get(ev->object) != data) return;

   MY_DATA(data, pd);
   _border_flush(data, pd);
}

EFL_CALLBACKS_ARRAY_DEFINE(parent_manager,
    {EFL_UI_FOCUS_MANAGER_EVENT_PRE_FLUSH, _parent_manager_pre_flush},
    {EFL_UI_FOCUS_MANAGER_EVENT_REDIRECT_CHANGED, _redirect_changed_cb}
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
   Efl_Ui_Focus_User *b;
   Efl_Ui_Focus_Manager *manager;
   manager = efl_ui_focus_user_manager_get(ev->object);

   if (!ev->info) return;

   EINA_LIST_FOREACH(pd->current_border, n, b)
     {
        if (b == ev->object) continue;
        efl_ui_focus_manager_update_parent(manager, b, ev->info);
     }
}

static void
_manager_change(void *data, const Efl_Event *ev)
{
   MY_DATA(data, pd);

   Efl_Ui_Focus_Manager *manager;
   Efl_Ui_Focus_Object *logical;
   Efl_Ui_Focus_User *b;
   Eina_List *n;

   logical = efl_ui_focus_user_parent_get(ev->object);
   manager = efl_ui_focus_user_manager_get(ev->object);

   //unregister from the old
   efl_event_callback_array_del(pd->manager, parent_manager(), data);
   efl_event_callback_array_add(manager, parent_manager(), data);

   EINA_LIST_FOREACH(pd->current_border , n, b)
     {
        if (b == ev->object) continue;

        _unregister(data, manager, b);
        _register(data, manager, b, logical);
     }
   //unregister the old manager, use the new
   pd->manager = manager;
}

EFL_CALLBACKS_ARRAY_DEFINE(self_manager,
    {EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, _self_manager_dirty},
    {EFL_UI_FOCUS_USER_EVENT_LOGICAL_CHANGED, _logical_manager_change},
    {EFL_UI_FOCUS_USER_EVENT_MANAGER_CHANGED, _manager_change}
);

EOLIAN static void
_efl_ui_focus_manager_sub_efl_object_parent_set(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd EINA_UNUSED, Efl_Object *parent)
{
   Eo *old_parent, *new_parent;

   old_parent = efl_parent_get(obj);

   _efl_ui_focus_manager_redirect_events_del(obj, old_parent);
   efl_event_callback_array_del(old_parent, self_manager(), obj);

   efl_parent_set(efl_super(obj, MY_CLASS), parent);

   new_parent = efl_parent_get(obj);
   _efl_ui_focus_manager_redirect_events_add(obj, new_parent);
   efl_event_callback_array_add(new_parent, self_manager(), obj);
}

EOLIAN static Efl_Object*
_efl_ui_focus_manager_sub_efl_object_constructor(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd EINA_UNUSED)
{
   return efl_constructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_focus_manager_sub_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Manager_Sub_Data *pd)
{
   _border_unregister(obj, pd);

   return efl_destructor(efl_super(obj, MY_CLASS));
}


#include "efl_ui_focus_manager_sub.eo.c"