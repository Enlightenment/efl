#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

const char* Atspi_Name[] = {
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

struct _Elm_Atspi_Event_Handler
{
   Eo_Event_Cb cb;
   void *data;
};

struct _Elm_Interface_Atspi_Accessible_Data
{
   Elm_Atspi_Role role;
   const char    *name;
   const char    *description;
   const char    *translation_domain;
   Elm_Atspi_Relation_Set relations;
   Elm_Atspi_Type type: 2;
};

typedef struct _Elm_Interface_Atspi_Accessible_Data Elm_Interface_Atspi_Accessible_Data;


static Eina_List *global_callbacks;
static Eo *root;

EOLIAN static int
_elm_interface_atspi_accessible_index_in_parent_get(Eo *obj, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   Eina_List *l, *children = NULL;
   Eo *chld, *parent = NULL;
   int ret = 0;

   parent = elm_interface_atspi_accessible_parent_get(obj);
   if (!parent) return -1;

   children = elm_interface_atspi_accessible_children_get(parent);
   if (!children) return -1;

   EINA_LIST_FOREACH(children, l, chld)
     {
       if (obj == chld)
         break;
       ret++;
     }
   if (ret == (int)eina_list_count(children))
     {
        ERR("Object %s not present in its AT-SPI parents (%s) children list! This should never happen.", eo_class_name_get(eo_class_get(obj)), eo_class_name_get(eo_class_get(parent)));
        ret = -1;
     }
   eina_list_free(children);
   return ret;
}

EOLIAN static Elm_Interface_Atspi_Accessible *
_elm_interface_atspi_accessible_parent_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   Elm_Atspi_Type type;
   Eo *parent = obj;

   do {
      parent = eo_parent_get(obj);
      if (eo_isa(parent, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
        {
           type = elm_interface_atspi_accessible_type_get(parent);
           if (type != ELM_ATSPI_TYPE_SKIPPED) break;
        }
   } while (parent);

   return eo_isa(parent, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN) ? parent : NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_parent_set(Eo *obj, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED, Elm_Interface_Atspi_Accessible *new_parent EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_parent_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
}

EOLIAN Eina_List*
_elm_interface_atspi_accessible_attributes_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   WRN("The %s object does not implement the \"accessible_attributes_set\" function.",
       eo_class_name_get(eo_class_get(obj)));
   return NULL;
}

EOLIAN static Elm_Atspi_Role
_elm_interface_atspi_accessible_role_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   return pd->role;
}

EOLIAN static void
_elm_interface_atspi_accessible_role_set(Eo *obj, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED, Elm_Atspi_Role role)
{
   if (pd->role != role)
     {
        pd->role = role;
        elm_interface_atspi_accessible_role_changed_signal_emit(obj);
     }
}

EOLIAN const char *
_elm_interface_atspi_accessible_role_name_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   Elm_Atspi_Role role;

   role = elm_interface_atspi_accessible_role_get(obj);

   return role > ELM_ATSPI_ROLE_LAST_DEFINED ? "" : Atspi_Name[role];
}

EOLIAN char *
_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd)
{
   if (pd->name)
     {
#ifdef ENABLE_NLS
        if (pd->translation_domain)
          return strdup(dgettext(pd->translation_domain, pd->name));
        else
          return strdup(pd->name);
#else
        return strdup(pd->name);
#endif
     }

   return NULL;
}

EOLIAN static void
_elm_interface_atspi_accessible_name_set(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd, char *val)
{
   eina_stringshare_replace(&pd->name, val);
}

const char * _elm_interface_atspi_accessible_description_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd)
{
#ifdef ENABLE_NLS
   if (pd->translation_domain)
      return dgettext(pd->translation_domain, pd->description);
#endif
   return pd->description;
}

EOLIAN static void
_elm_interface_atspi_accessible_description_set(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->description, val);
}

EOLIAN static const char *
_elm_interface_atspi_accessible_localized_role_name_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   const char *ret = NULL;
   ret = elm_interface_atspi_accessible_role_name_get(obj);
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

EOLIAN static Eina_List *
_elm_interface_atspi_accessible_children_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   Eina_List *children = NULL;
   Eina_Iterator *iter = NULL;
   Eo *chld;

   // By default use Eo_Base object hierarchy
   iter = eo_children_iterator_new(obj);
   if (!iter) return NULL;

   EINA_ITERATOR_FOREACH(iter, chld)
     {
        if (eo_isa(chld, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
          children = eina_list_append(children, chld);
     }
   eina_iterator_free(iter);

   return children;
}

EOLIAN static Elm_Atspi_State_Set
_elm_interface_atspi_accessible_state_set_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN Elm_Atspi_Relation_Set
_elm_interface_atspi_accessible_relation_set_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd EINA_UNUSED)
{
   return elm_atspi_relation_set_clone(pd->relations);
}

EAPI void elm_atspi_attributes_list_free(Eina_List *list)
{
   Elm_Atspi_Attribute *attr;
   EINA_LIST_FREE(list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
}

EOLIAN void
_elm_interface_atspi_accessible_event_emit(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Eo *accessible, const Eo_Event_Description *event, void *event_info)
{
   Eina_List *l;
   Elm_Atspi_Event_Handler *hdl;
   Elm_Atspi_Type type;

   if (!accessible || !event || !eo_isa(accessible, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
     {
        CRI("Invalid parameters, event: %s, obj: %s", event ? event->name : "NULL", accessible ? eo_class_name_get(accessible) : "NULL");
        return;
     }

   type = elm_interface_atspi_accessible_type_get(accessible);
   if (type != ELM_ATSPI_TYPE_REGULAR)
     return;

   if (event == ELM_INTERFACE_ATSPI_ACCESSIBLE_EVENT_CHILDREN_CHANGED)
     {
        Elm_Atspi_Event_Children_Changed_Data *atspi_data = event_info;
        type = elm_interface_atspi_accessible_type_get(atspi_data->child);
        if (type != ELM_ATSPI_TYPE_REGULAR)
          return;
     }

   Eo_Event ev;
   ev.object = accessible;
   ev.desc = event;
   ev.info = event_info;
   EINA_LIST_FOREACH(global_callbacks, l, hdl)
     {
        if (hdl->cb)
          hdl->cb(hdl->data, &ev);
     }
}

EOLIAN Elm_Atspi_Event_Handler *
_elm_interface_atspi_accessible_event_handler_add(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Eo_Event_Cb cb, void *data)
{
   Elm_Atspi_Event_Handler *ret = calloc(1, sizeof(Elm_Atspi_Event_Handler));

   ret->cb = cb;
   ret->data = data;

   global_callbacks = eina_list_append(global_callbacks, ret);

   return ret;
}

EOLIAN void 
_elm_interface_atspi_accessible_event_handler_del(Eo *class EINA_UNUSED, void *pd EINA_UNUSED, Elm_Atspi_Event_Handler *handler)
{
   Eina_List *l, *l2;
   Elm_Atspi_Event_Handler *hdl;
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
_elm_interface_atspi_accessible_translation_domain_set(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd, const char *domain)
{
   eina_stringshare_replace(&pd->translation_domain, domain);
}


EOLIAN const char*
_elm_interface_atspi_accessible_translation_domain_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd)
{
   return pd->translation_domain;
}

EAPI void
elm_atspi_relation_free(Elm_Atspi_Relation *relation)
{
   eina_list_free(relation->objects);
   free(relation);
}

EAPI Elm_Atspi_Relation *
elm_atspi_relation_clone(const Elm_Atspi_Relation *relation)
{
   Elm_Atspi_Relation *ret = calloc(1, sizeof(Elm_Atspi_Relation));
   if (!ret) return NULL;

   ret->type = relation->type;
   ret->objects = eina_list_clone(relation->objects);
   return ret;
}

static Eina_Bool
_on_rel_obj_del(void *data, const Eo_Event *event)
{
   Elm_Atspi_Relation_Set *set = data;
   Elm_Atspi_Relation *rel;
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
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_atspi_relation_set_relation_append(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type, const Eo *rel_obj)
{
   Elm_Atspi_Relation *rel;
   Eina_List *l;

   if (!eo_isa(rel_obj, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
     return EINA_FALSE;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             if (!eina_list_data_find(rel->objects, rel_obj))
               {
                  rel->objects = eina_list_append(rel->objects, rel_obj);
                  eo_event_callback_add((Eo *) rel_obj, EO_EVENT_DEL, _on_rel_obj_del, set);
               }
             return EINA_TRUE;
          }
     }

   rel = calloc(1, sizeof(Elm_Atspi_Relation));
   if (!rel) return EINA_FALSE;

   rel->type = type;
   rel->objects = eina_list_append(rel->objects, rel_obj);
   *set = eina_list_append(*set, rel);

   eo_event_callback_add((Eo *) rel_obj, EO_EVENT_DEL, _on_rel_obj_del, set);
   return EINA_TRUE;
}

EAPI void
elm_atspi_relation_set_relation_remove(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type, const Eo *rel_obj)
{
   Eina_List *l;
   Elm_Atspi_Relation *rel;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             if (eina_list_data_find(rel->objects, rel_obj))
               {
                  eo_event_callback_del((Eo *) rel_obj, EO_EVENT_DEL, _on_rel_obj_del, set);
                  rel->objects = eina_list_remove(rel->objects, rel_obj);
               }
             if (!rel->objects)
               {
                  *set = eina_list_remove(*set, rel);
                  elm_atspi_relation_free(rel);
               }
             return;
          }
     }
}

EAPI void
elm_atspi_relation_set_relation_type_remove(Elm_Atspi_Relation_Set *set, Elm_Atspi_Relation_Type type)
{
   Eina_List *l;
   Elm_Atspi_Relation *rel;
   Eo *obj;

   EINA_LIST_FOREACH(*set, l, rel)
     {
        if (rel->type == type)
          {
             EINA_LIST_FOREACH(rel->objects, l, obj)
                eo_event_callback_del(obj, EO_EVENT_DEL, _on_rel_obj_del, set);
             *set = eina_list_remove(*set, rel);
             elm_atspi_relation_free(rel);
             return;
          }
     }
}

EAPI void
elm_atspi_relation_set_free(Elm_Atspi_Relation_Set set)
{
   Elm_Atspi_Relation *rel;
   Eina_List *l;
   Eo *obj;

   EINA_LIST_FREE(set, rel)
     {
        EINA_LIST_FOREACH(rel->objects, l, obj)
           eo_event_callback_del(obj, EO_EVENT_DEL, _on_rel_obj_del, set);
        elm_atspi_relation_free(rel);
     }
}

EAPI Elm_Atspi_Relation_Set
elm_atspi_relation_set_clone(const Elm_Atspi_Relation_Set set)
{
   Elm_Atspi_Relation_Set ret = NULL;
   Eina_List *l;
   Elm_Atspi_Relation *rel;

   EINA_LIST_FOREACH(set, l, rel)
     {
        Elm_Atspi_Relation *cpy = elm_atspi_relation_clone(rel);
        ret = eina_list_append(ret, cpy);
     }

   return ret;
}

EOLIAN static Eina_Bool
_elm_interface_atspi_accessible_relationship_append(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *sd, Elm_Atspi_Relation_Type type, const Elm_Interface_Atspi_Accessible *relation_obj)
{
   return elm_atspi_relation_set_relation_append(&sd->relations, type, relation_obj);
}

EOLIAN static void
_elm_interface_atspi_accessible_relationship_remove(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *sd, Elm_Atspi_Relation_Type type, const Elm_Interface_Atspi_Accessible *relation_obj)
{
   if (relation_obj)
     elm_atspi_relation_set_relation_remove(&sd->relations, type, relation_obj);
   else
     elm_atspi_relation_set_relation_type_remove(&sd->relations, type);
}

EOLIAN static void
_elm_interface_atspi_accessible_relationships_clear(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *sd)
{
   elm_atspi_relation_set_free(sd->relations);
   sd->relations = NULL;
}

EOLIAN Eo*
_elm_interface_atspi_accessible_root_get(Eo *class EINA_UNUSED, void *pd EINA_UNUSED)
{
   if (!root)
      root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);

   return root;
}

EOLIAN Elm_Atspi_Type
_elm_interface_atspi_accessible_type_get(Eo *obj EINA_UNUSED, Elm_Interface_Atspi_Accessible_Data *pd)
{
   return pd->type;
}

EOLIAN void
_elm_interface_atspi_accessible_type_set(Eo *obj, Elm_Interface_Atspi_Accessible_Data *pd, Elm_Atspi_Type val)
{
   Elm_Interface_Atspi_Accessible *parent;
   if (val == pd->type)
     return;

   parent = elm_interface_atspi_accessible_parent_get(obj);

   switch (val)
     {
      case ELM_ATSPI_TYPE_DISABLED:
      case ELM_ATSPI_TYPE_SKIPPED:
         if (parent) elm_interface_atspi_accessible_children_changed_del_signal_emit(parent, obj);
         elm_interface_atspi_accessible_removed(obj);
         break;
      case ELM_ATSPI_TYPE_REGULAR:
         if (parent) elm_interface_atspi_accessible_children_changed_added_signal_emit(parent, obj);
         elm_interface_atspi_accessible_added(obj);
     }
   pd->type = val;
}

#include "elm_interface_atspi_accessible.eo.c"
