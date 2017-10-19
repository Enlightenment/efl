#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

const char* Access_Name[] = {
    "invalid",
    "accelerator label",
    "alert",
    "animation",
    "arrow",
    "calendar",
    "canvas",
    "check box",
    "check menu item",
    "color chooser",
    "column header",
    "combo box",
    "dateeditor",
    "desktop icon",
    "desktop frame",
    "dial",
    "dialog",
    "directory pane",
    "drawing area",
    "file chooser",
    "filler",
    "focus traversable",
    "font chooser",
    "frame",
    "glass pane",
    "html container",
    "icon",
    "image",
    "internal frame",
    "label",
    "layered pane",
    "list",
    "list item",
    "menu",
    "menu bar",
    "menu item",
    "option pane",
    "page tab",
    "page tab list",
    "panel",
    "password text",
    "popup menu",
    "progress bar",
    "push button",
    "radio button",
    "radio menu item",
    "root pane",
    "row header",
    "scroll bar",
    "scroll pane",
    "separator",
    "slider",
    "spin button",
    "split pane",
    "status bar",
    "table",
    "table cell",
    "table column header",
    "table row header",
    "tearoff menu item",
    "terminal",
    "text",
    "toggle button",
    "tool bar",
    "tool tip",
    "tree",
    "tree table",
    "unknown",
    "viewport",
    "window",
    "extended",
    "header",
    "footer",
    "paragraph",
    "ruler",
    "application",
    "autocomplete",
    "editbar",
    "embedded",
    "entry",
    "chart",
    "caption",
    "document frame",
    "heading",
    "page",
    "section",
    "redundant object",
    "form",
    "link",
    "input method window",
    "table row",
    "tree item",
    "document spreadsheet",
    "document presentation",
    "document text",
    "document web",
    "document email",
    "comment",
    "list box",
    "grouping",
    "image map",
    "notification",
    "info bar",
    "last defined"
};

struct _Efl_Access_Event_Handler
{
   Efl_Event_Cb cb;
   void *data;
};

struct _Efl_Access_Data
{
   Efl_Access_Role role;
   const char    *name;
   const char    *description;
   const char    *translation_domain;
   Efl_Access_Relation_Set relations;
   Efl_Access_Type type: 2;
};

typedef struct _Efl_Access_Data Efl_Access_Data;


static Eina_List *global_callbacks;
static Eo *root;

EOLIAN static int
_efl_access_index_in_parent_get(Eo *obj, Efl_Access_Data *pd EINA_UNUSED)
{
   Eina_List *l, *children = NULL;
   Eo *chld, *parent = NULL;
   int ret = 0;

   parent = efl_access_parent_get(obj);
   if (!parent) return -1;

   children = efl_access_children_get(parent);
   if (!children) return -1;

   EINA_LIST_FOREACH(children, l, chld)
     {
       if (obj == chld)
         break;
       ret++;
     }
   if (ret == (int)eina_list_count(children))
     {
        ERR("Object %s not present in its AT-SPI parents (%s) children list! This should never happen.", efl_class_name_get(efl_class_get(obj)), efl_class_name_get(efl_class_get(parent)));
        ret = -1;
     }
   eina_list_free(children);
   return ret;
}

EOLIAN static Efl_Access *
_efl_access_parent_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   Efl_Access_Type type;
   Eo *parent = obj;

   do {
      parent = efl_parent_get(obj);
      if (efl_isa(parent, EFL_ACCESS_MIXIN))
        {
           type = efl_access_type_get(parent);
           if (type != EFL_ACCESS_TYPE_SKIPPED) break;
        }
   } while (parent);

   return efl_isa(parent, EFL_ACCESS_MIXIN) ? parent : NULL;
}

EOLIAN static void
_efl_access_parent_set(Eo *obj, Efl_Access_Data *pd EINA_UNUSED, Efl_Access *new_parent EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_parent_set\" function.",
       efl_class_name_get(efl_class_get(obj)));
}

EOLIAN Eina_List*
_efl_access_attributes_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_attributes_set\" function.",
       efl_class_name_get(efl_class_get(obj)));
   return NULL;
}

EOLIAN static Efl_Access_Role
_efl_access_role_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   return pd->role;
}

EOLIAN static void
_efl_access_role_set(Eo *obj, Efl_Access_Data *pd EINA_UNUSED, Efl_Access_Role role)
{
   if (pd->role != role)
     {
        pd->role = role;
        efl_access_role_changed_signal_emit(obj);
     }
}

EOLIAN const char *
_efl_access_role_name_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   Efl_Access_Role role;

   role = efl_access_role_get(obj);

   return role > EFL_ACCESS_ROLE_LAST_DEFINED ? "" : Access_Name[role];
}

EOLIAN const char *
_efl_access_name_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd)
{
#ifdef ENABLE_NLS
   if (pd->translation_domain)
     return dgettext(pd->translation_domain, pd->name);
#endif
   return pd->name;
}

EOLIAN static void
_efl_access_name_set(Eo *obj EINA_UNUSED, Efl_Access_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->name, val);
}

const char * _efl_access_description_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd)
{
#ifdef ENABLE_NLS
   if (pd->translation_domain)
      return dgettext(pd->translation_domain, pd->description);
#endif
   return pd->description;
}

EOLIAN static void
_efl_access_description_set(Eo *obj EINA_UNUSED, Efl_Access_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->description, val);
}

EOLIAN static const char *
_efl_access_localized_role_name_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   const char *ret = NULL;
   ret = efl_access_role_name_get(obj);
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

EOLIAN static Eina_List *
_efl_access_children_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   Eina_List *children = NULL;
   Eina_Iterator *iter = NULL;
   Eo *chld;

   // By default use Efl_Object object hierarchy
   iter = efl_children_iterator_new(obj);
   if (!iter) return NULL;

   EINA_ITERATOR_FOREACH(iter, chld)
     {
        if (efl_isa(chld, EFL_ACCESS_MIXIN))
          children = eina_list_append(children, chld);
     }
   eina_iterator_free(iter);

   return children;
}

EOLIAN static Efl_Access_State_Set
_efl_access_state_set_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN Efl_Access_Relation_Set
_efl_access_relation_set_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd EINA_UNUSED)
{
   return efl_access_relation_set_clone(pd->relations);
}

EAPI void efl_access_attributes_list_free(Eina_List *list)
{
   Efl_Access_Attribute *attr;
   EINA_LIST_FREE(list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}

EOLIAN void
_efl_access_event_emit(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Eo *accessible, const Efl_Event_Description *event, void *event_info)
{
   Eina_List *l;
   Efl_Access_Event_Handler *hdl;
   Efl_Access_Type type;

   if (!accessible || !event || !efl_isa(accessible, EFL_ACCESS_MIXIN))
     {
        CRI("Invalid parameters, event: %s, obj: %s", event ? event->name : "NULL", accessible ? efl_class_name_get(accessible) : "NULL");
        return;
     }

   type = efl_access_type_get(accessible);
   if (type != EFL_ACCESS_TYPE_REGULAR)
     return;

   if (event == EFL_ACCESS_EVENT_CHILDREN_CHANGED)
     {
        Efl_Access_Event_Children_Changed_Data *event_data = event_info;
        type = efl_access_type_get(event_data->child);
        if (type != EFL_ACCESS_TYPE_REGULAR)
          return;
     }

   Efl_Event ev;
   ev.object = accessible;
   ev.desc = event;
   ev.info = event_info;
   EINA_LIST_FOREACH(global_callbacks, l, hdl)
     {
        if (hdl->cb)
          hdl->cb(hdl->data, &ev);
     }
}

EOLIAN Efl_Access_Event_Handler *
_efl_access_event_handler_add(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Efl_Event_Cb cb, void *data)
{
   Efl_Access_Event_Handler *ret = calloc(1, sizeof(Efl_Access_Event_Handler));

   ret->cb = cb;
   ret->data = data;

   global_callbacks = eina_list_append(global_callbacks, ret);

   return ret;
}

EOLIAN void 
_efl_access_event_handler_del(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Efl_Access_Event_Handler *handler)
{
   Eina_List *l, *l2;
   Efl_Access_Event_Handler *hdl;
   EINA_LIST_FOREACH_SAFE(global_callbacks, l, l2, hdl)
     {
        if (hdl == handler)
          {
             global_callbacks = eina_list_remove_list(global_callbacks, l);
             free(hdl);
             break;
          }
     }
}

EOLIAN void
_efl_access_translation_domain_set(Eo *obj EINA_UNUSED, Efl_Access_Data *pd, const char *domain)
{
   eina_stringshare_replace(&pd->translation_domain, domain);
}


EOLIAN const char*
_efl_access_translation_domain_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd)
{
   return pd->translation_domain;
}

EAPI void
efl_access_relation_free(Efl_Access_Relation *relation)
{
   eina_list_free(relation->objects);
   free(relation);
}

EAPI Efl_Access_Relation *
efl_access_relation_clone(const Efl_Access_Relation *relation)
{
   Efl_Access_Relation *ret = calloc(1, sizeof(Efl_Access_Relation));
   if (!ret) return NULL;

   ret->type = relation->type;
   ret->objects = eina_list_clone(relation->objects);
   return ret;
}

static void
_on_rel_obj_del(void *data, const Efl_Event *event)
{
   Efl_Access_Relation_Set *set = data;
   Efl_Access_Relation *rel;
   Eina_List *l, *l2, *p, *p2;
   Eo *rel_obj;

   EINA_LIST_FOREACH_SAFE(*set, l, l2, rel)
     {
        EINA_LIST_FOREACH_SAFE(rel->objects, p, p2, rel_obj)
          {
          if (rel_obj == event->object)
               rel->objects = eina_list_remove_list(rel->objects, p);
          }
        if (!rel->objects)
          {
             *set = eina_list_remove_list(*set, l);
             free(rel);
          }
     }
}

EAPI Eina_Bool
efl_access_relation_set_relation_append(Efl_Access_Relation_Set *set, Efl_Access_Relation_Type type, const Eo *rel_obj)
{
   Efl_Access_Relation *rel;
   Eina_List *l;

   if (!efl_isa(rel_obj, EFL_ACCESS_MIXIN))
     return EINA_FALSE;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             if (!eina_list_data_find(rel->objects, rel_obj))
               {
                  rel->objects = eina_list_append(rel->objects, rel_obj);
                  efl_event_callback_add((Eo *) rel_obj, EFL_EVENT_DEL, _on_rel_obj_del, set);
               }
             return EINA_TRUE;
          }
     }

   rel = calloc(1, sizeof(Efl_Access_Relation));
   if (!rel) return EINA_FALSE;

   rel->type = type;
   rel->objects = eina_list_append(rel->objects, rel_obj);
   *set = eina_list_append(*set, rel);

   efl_event_callback_add((Eo *) rel_obj, EFL_EVENT_DEL, _on_rel_obj_del, set);
   return EINA_TRUE;
}

EAPI void
efl_access_relation_set_relation_remove(Efl_Access_Relation_Set *set, Efl_Access_Relation_Type type, const Eo *rel_obj)
{
   Eina_List *l;
   Efl_Access_Relation *rel;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             if (eina_list_data_find(rel->objects, rel_obj))
               {
                  efl_event_callback_del((Eo *) rel_obj, EFL_EVENT_DEL, _on_rel_obj_del, set);
                  rel->objects = eina_list_remove(rel->objects, rel_obj);
               }
             if (!rel->objects)
               {
                  *set = eina_list_remove(*set, rel);
                  efl_access_relation_free(rel);
               }
             return;
          }
     }
}

EAPI void
efl_access_relation_set_relation_type_remove(Efl_Access_Relation_Set *set, Efl_Access_Relation_Type type)
{
   Eina_List *l;
   Efl_Access_Relation *rel;
   Eo *obj;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             EINA_LIST_FOREACH(rel->objects, l, obj)
                efl_event_callback_del(obj, EFL_EVENT_DEL, _on_rel_obj_del, set);
             *set = eina_list_remove(*set, rel);
             efl_access_relation_free(rel);
             return;
          }
     }
}

EAPI void
efl_access_relation_set_free(Efl_Access_Relation_Set set)
{
   Efl_Access_Relation *rel;
   Eina_List *l;
   Eo *obj;

   EINA_LIST_FREE(set, rel)
     {
        EINA_LIST_FOREACH(rel->objects, l, obj)
           efl_event_callback_del(obj, EFL_EVENT_DEL, _on_rel_obj_del, set);
        efl_access_relation_free(rel);
     }
}

EAPI Efl_Access_Relation_Set
efl_access_relation_set_clone(const Efl_Access_Relation_Set set)
{
   Efl_Access_Relation_Set ret = NULL;
   Eina_List *l;
   Efl_Access_Relation *rel;

   EINA_LIST_FOREACH(set, l, rel)
     {
        Efl_Access_Relation *cpy = efl_access_relation_clone(rel);
        ret = eina_list_append(ret, cpy);
     }

   return ret;
}

EOLIAN static Eina_Bool
_efl_access_relationship_append(Eo *obj EINA_UNUSED, Efl_Access_Data *sd, Efl_Access_Relation_Type type, const Efl_Access *relation_obj)
{
   return efl_access_relation_set_relation_append(&sd->relations, type, relation_obj);
}

EOLIAN static void
_efl_access_relationship_remove(Eo *obj EINA_UNUSED, Efl_Access_Data *sd, Efl_Access_Relation_Type type, const Efl_Access *relation_obj)
{
   if (relation_obj)
     efl_access_relation_set_relation_remove(&sd->relations, type, relation_obj);
   else
     efl_access_relation_set_relation_type_remove(&sd->relations, type);
}

EOLIAN static void
_efl_access_relationships_clear(Eo *obj EINA_UNUSED, Efl_Access_Data *sd)
{
   efl_access_relation_set_free(sd->relations);
   sd->relations = NULL;
}

EOLIAN Eo*
_efl_access_root_get(Eo *class EINA_UNUSED, void *pd EINA_UNUSED)
{
   if (!root)
      root = efl_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   return root;
}

EOLIAN Efl_Access_Type
_efl_access_type_get(Eo *obj EINA_UNUSED, Efl_Access_Data *pd)
{
   return pd->type;
}

EOLIAN void
_efl_access_type_set(Eo *obj, Efl_Access_Data *pd, Efl_Access_Type val)
{
   Efl_Access *parent;
   if (val == pd->type)
     return;

   parent = efl_access_parent_get(obj);

   switch (val)
     {
      case EFL_ACCESS_TYPE_DISABLED:
      case EFL_ACCESS_TYPE_SKIPPED:
         if (parent) efl_access_children_changed_del_signal_emit(parent, obj);
         efl_access_removed(obj);
         break;
      case EFL_ACCESS_TYPE_REGULAR:
         if (parent) efl_access_children_changed_added_signal_emit(parent, obj);
         efl_access_added(obj);
     }
   pd->type = val;
}

EOLIAN void
_efl_access_efl_object_destructor(Eo *obj, Efl_Access_Data *pd)
{
   eina_stringshare_del(pd->name);
   eina_stringshare_del(pd->description);
   eina_stringshare_del(pd->translation_domain);
   efl_access_relation_set_free(pd->relations);

   efl_destructor(efl_super(obj, EFL_ACCESS_MIXIN));
}

void
_efl_access_shutdown(void)
{
   Efl_Access_Event_Handler *hdl;

   EINA_LIST_FREE(global_callbacks, hdl)
     free(hdl);

   ELM_SAFE_DEL(root);
}

#include "efl_access.eo.c"
