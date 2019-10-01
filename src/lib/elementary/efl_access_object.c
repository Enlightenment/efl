#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

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

struct _Efl_Access_Object_Data
{
   Eina_List     *relations;
   Eina_List     *attr_list;
   const char    *name;
   const char    *description;
   const char    *translation_domain;
   Efl_Access_Role role;
   Efl_Access_Reading_Info_Type reading_info;
   Efl_Access_Type type: 2;
};

typedef struct _Efl_Access_Object_Data Efl_Access_Object_Data;


static Eina_List *global_callbacks;
static Eo *root;

EOLIAN static int
_efl_access_object_index_in_parent_get(const Eo *obj, Efl_Access_Object_Data *pd EINA_UNUSED)
{
   Eina_List *l, *children = NULL;
   Eo *chld, *parent = NULL;
   int ret = 0;

   parent = efl_provider_find(efl_parent_get(obj), EFL_ACCESS_OBJECT_MIXIN);
   if (!parent) return -1;

   children = efl_access_object_access_children_get(parent);
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


EOLIAN static Efl_Object *
_efl_access_object_efl_object_provider_find(const Eo *obj, Efl_Access_Object_Data *pd EINA_UNUSED, const Efl_Object *klass)
{
   if (efl_isa(obj, klass))
     {
        if (klass == EFL_ACCESS_OBJECT_MIXIN)
          {
             Efl_Access_Type type = efl_access_object_access_type_get(obj);
             if (type != EFL_ACCESS_TYPE_SKIPPED) return (Eo*)obj;
          }
        else return (Eo*)obj;
     }
   return efl_provider_find(efl_super(obj, EFL_ACCESS_OBJECT_MIXIN), klass);
}

EOLIAN Eina_List*
_efl_access_object_attributes_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
   Eina_List *attr_list = NULL;
   if (pd->attr_list)
     {
        Eina_List *l = NULL;
        Efl_Access_Attribute *t_attr = NULL;
        EINA_LIST_FOREACH(pd->attr_list, l, t_attr)
          {
             Efl_Access_Attribute *attr = calloc(1, sizeof(Efl_Access_Attribute));
             if (!attr)
               return attr_list;

             attr->key = eina_stringshare_add(t_attr->key);
             attr->value = eina_stringshare_add(t_attr->value);
             attr_list = eina_list_append(attr_list, attr);
          }
     }
   return attr_list;
}

EOLIAN static void
_efl_access_object_attribute_append(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd, const char *key, const char *value)
{
   Eina_List *l;
   Efl_Access_Attribute *attr = NULL;

   if (!key || !value) return;

   /* Check existing attributes has this key */
   EINA_LIST_FOREACH(pd->attr_list, l, attr)
     {
        if (!strcmp((const char *)attr->key, key))
          {
             eina_stringshare_replace(&attr->value, value);
             return;
          }
     }

   /* Add new attribute */
   attr = calloc(1, sizeof(Efl_Access_Attribute));
   if (!attr) return;

   attr->key = eina_stringshare_add(key);
   attr->value = eina_stringshare_add(value);
   pd->attr_list = eina_list_append(pd->attr_list, attr);
}

EOLIAN static void
_efl_access_object_attribute_del(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd, const char *key)
{
   Eina_List *l;
   Efl_Access_Attribute *attr = NULL;

   if (!key) return;
   if (!pd->attr_list) return;

   /* Check whether existing attribute list has this key and delete */
   EINA_LIST_FOREACH(pd->attr_list, l, attr)
     {
        if (!strcmp((const char *)attr->key, key))
          {
             pd->attr_list = eina_list_remove_list(pd->attr_list, l);
             eina_stringshare_del(attr->key);
             eina_stringshare_del(attr->value);
             free(attr);
             return;
          }
     }
}

EOLIAN static void _efl_access_object_attributes_clear(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
   if (!pd->attr_list) return;
   Efl_Access_Attribute *attr;
   EINA_LIST_FREE(pd->attr_list, attr)
     {
        eina_stringshare_del(attr->key);
        eina_stringshare_del(attr->value);
        free(attr);
     }
   pd->attr_list = NULL;
}

EOLIAN static void
_efl_access_object_reading_info_type_set(Eo *obj, Efl_Access_Object_Data *pd, Efl_Access_Reading_Info_Type reading_info)
{
   Eina_Strbuf *buf = NULL;
   if (reading_info == pd->reading_info)
     return;
   pd->reading_info = reading_info;
   if (!pd->reading_info)
     {
        efl_access_object_attribute_del(obj, "reading_info_type");
        return;
     }
   buf = eina_strbuf_new();
   eina_strbuf_reset(buf);
   if (reading_info & (EFL_ACCESS_READING_INFO_TYPE_NAME))
     {
        eina_strbuf_append(buf, "name");
        eina_strbuf_append_char(buf, '|');
     }
   if (reading_info & (EFL_ACCESS_READING_INFO_TYPE_ROLE))
     {
        eina_strbuf_append(buf, "role");
        eina_strbuf_append_char(buf, '|');
     }
   if (reading_info & (EFL_ACCESS_READING_INFO_TYPE_DESCRIPTION))
     {
        eina_strbuf_append(buf, "description");
        eina_strbuf_append_char(buf, '|');
     }
   if (reading_info & (EFL_ACCESS_READING_INFO_TYPE_STATE))
     {
        eina_strbuf_append(buf, "state");
     }
   efl_access_object_attribute_append(obj, "reading_info_type", eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

EOLIAN Efl_Access_Reading_Info_Type
_efl_access_object_reading_info_type_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
   return pd->reading_info;
}

EOLIAN static Efl_Access_Role
_efl_access_object_role_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
   return pd->role;
}

EOLIAN static void
_efl_access_object_role_set(Eo *obj, Efl_Access_Object_Data *pd, Efl_Access_Role role)
{
   if (pd->role != role)
     {
        pd->role = role;
        efl_access_role_changed_signal_emit(obj);
     }
}

EOLIAN const char *
_efl_access_object_role_name_get(const Eo *obj, Efl_Access_Object_Data *pd EINA_UNUSED)
{
   Efl_Access_Role role;

   role = efl_access_object_role_get(obj);

   return role > EFL_ACCESS_ROLE_LAST_DEFINED ? "" : Access_Name[role];
}

EOLIAN const char *
_efl_access_object_i18n_name_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
#ifdef ENABLE_NLS
   if (pd->translation_domain)
     return dgettext(pd->translation_domain, pd->name);
#endif
   return pd->name;
}

EOLIAN static void
_efl_access_object_i18n_name_set(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->name, val);
}

const char * _efl_access_object_description_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
#ifdef ENABLE_NLS
   if (pd->translation_domain)
      return dgettext(pd->translation_domain, pd->description);
#endif
   return pd->description;
}

EOLIAN static void
_efl_access_object_description_set(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->description, val);
}

EOLIAN static const char *
_efl_access_object_localized_role_name_get(const Eo *obj, Efl_Access_Object_Data *pd EINA_UNUSED)
{
   const char *ret = NULL;
   ret = efl_access_object_role_name_get(obj);
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

EOLIAN static Eina_List *
_efl_access_object_access_children_get(const Eo *obj, Efl_Access_Object_Data *pd EINA_UNUSED)
{
   Eina_List *children = NULL;
   Eina_Iterator *iter = NULL;
   Eo *chld;

   // By default use Efl_Object object hierarchy
   /* XXX const */
   iter = efl_children_iterator_new((Eo *)obj);
   if (!iter) return NULL;

   EINA_ITERATOR_FOREACH(iter, chld)
     {
        if (efl_isa(chld, EFL_ACCESS_OBJECT_MIXIN))
          children = eina_list_append(children, chld);
     }
   eina_iterator_free(iter);

   return children;
}

EOLIAN static Efl_Access_State_Set
_efl_access_object_state_set_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN Eina_Iterator *
_efl_access_object_relations_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
   return eina_list_iterator_new(pd->relations);
}

EAPI void
efl_access_attribute_free(Efl_Access_Attribute *attr)
{
   eina_stringshare_del(attr->key);
   eina_stringshare_del(attr->value);
   free(attr);
}

EAPI void efl_access_attributes_list_free(Eina_List *list)
{
   Efl_Access_Attribute *attr;
   EINA_LIST_FREE(list, attr)
     {
        efl_access_attribute_free(attr);
     }
}

EOLIAN void
_efl_access_object_event_emit(Eo *accessible, const Efl_Event_Description *event, void *event_info)
{
   Eina_List *l;
   Efl_Access_Event_Handler *hdl;
   Efl_Access_Type type;

   if (!accessible || !event || !efl_isa(accessible, EFL_ACCESS_OBJECT_MIXIN))
     {
        CRI("Invalid parameters, event: %s, obj: %s", event ? event->name : "NULL", accessible ? efl_class_name_get(accessible) : "NULL");
        return;
     }

   type = efl_access_object_access_type_get(accessible);
   if (type != EFL_ACCESS_TYPE_REGULAR)
     return;

   if (event == EFL_ACCESS_OBJECT_EVENT_CHILDREN_CHANGED)
     {
        Efl_Access_Event_Children_Changed_Data *event_data = event_info;
        type = efl_access_object_access_type_get(event_data->child);
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
_efl_access_object_event_handler_add(Efl_Event_Cb cb, void *data)
{
   Efl_Access_Event_Handler *ret = calloc(1, sizeof(Efl_Access_Event_Handler));
   if (!ret) return NULL;

   ret->cb = cb;
   ret->data = data;

   global_callbacks = eina_list_append(global_callbacks, ret);

   return ret;
}

EOLIAN void
_efl_access_object_event_handler_del(Efl_Access_Event_Handler *handler)
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
_efl_access_object_translation_domain_set(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd, const char *domain)
{
   eina_stringshare_replace(&pd->translation_domain, domain);
}


EOLIAN const char*
_efl_access_object_translation_domain_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
   return pd->translation_domain;
}

static void
_on_rel_obj_del(void *data, const Efl_Event *event)
{
   Efl_Access_Object_Data *sd = data;
   Efl_Access_Relation *rel;
   Eina_List *l, *l2, *p, *p2;
   Eo *rel_obj;

   EINA_LIST_FOREACH_SAFE(sd->relations, l, l2, rel)
     {
        EINA_LIST_FOREACH_SAFE(rel->objects, p, p2, rel_obj)
          {
             if (rel_obj == event->object)
               rel->objects = eina_list_remove_list(rel->objects, p);
          }
        if (!rel->objects)
          {
             sd->relations = eina_list_remove_list(sd->relations, l);
             free(rel);
          }
     }
}

static void
efl_access_relation_set_free(Efl_Access_Object_Data *sd)
{
   Efl_Access_Relation *rel;
   Eo *obj;

   EINA_LIST_FREE(sd->relations, rel)
     {
        Eina_List *l;

        EINA_LIST_FOREACH(rel->objects, l, obj)
          efl_event_callback_del(obj, EFL_EVENT_DEL, _on_rel_obj_del, sd);
        eina_list_free(rel->objects);
        free(rel);
     }
}

EOLIAN static Eina_Bool
_efl_access_object_relationship_append(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *sd, Efl_Access_Relation_Type type, const Efl_Access_Object *relation)
{
   Efl_Access_Relation *rel;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->relations, l, rel)
     {
        if (rel->type == type)
          {
             if (!eina_list_data_find(rel->objects, relation))
               {
                  rel->objects = eina_list_append(rel->objects, relation);
                  efl_event_callback_add((Eo *) relation, EFL_EVENT_DEL, _on_rel_obj_del, sd);
               }
             return EINA_TRUE;
          }
     }

   rel = calloc(1, sizeof(Efl_Access_Relation));
   if (!rel) return EINA_FALSE;

   rel->type = type;
   rel->objects = eina_list_append(rel->objects, relation);
   sd->relations = eina_list_append(sd->relations, rel);

   efl_event_callback_add((Eo *) relation, EFL_EVENT_DEL, _on_rel_obj_del, sd);

   return EINA_TRUE;
}

EOLIAN static void
_efl_access_object_relationship_remove(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *sd, Efl_Access_Relation_Type type, const Efl_Access_Object *relation)
{
   Efl_Access_Relation *rel;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->relations, l, rel)
     {
        if (rel->type == type)
          {
             if (relation)
               {
                  if (eina_list_data_find(rel->objects, relation))
                    {
                       efl_event_callback_del((Eo *) relation, EFL_EVENT_DEL, _on_rel_obj_del, sd);
                       rel->objects = eina_list_remove(rel->objects, relation);
                    }
                  if (!rel->objects)
                    {
                       sd->relations = eina_list_remove(sd->relations, rel);
                       free(rel);
                    }
               }
             else
               {
                  Eina_List *ll;
                  Eo *ro;

                  EINA_LIST_FOREACH(rel->objects, ll, ro)
                    efl_event_callback_del(ro, EFL_EVENT_DEL, _on_rel_obj_del, sd);
                  sd->relations = eina_list_remove(sd->relations, rel);
                  free(rel);
               }
             return ;
          }
     }
}

EOLIAN static void
_efl_access_object_relationships_clear(Eo *obj EINA_UNUSED, Efl_Access_Object_Data *sd)
{
   efl_access_relation_set_free(sd);
   sd->relations = NULL;
}

EOLIAN Eo*
_efl_access_object_access_root_get(void)
{
   if (!root)
     root = efl_add(ELM_ATSPI_APP_OBJECT_CLASS, efl_main_loop_get());

   return root;
}

EOLIAN Efl_Access_Type
_efl_access_object_access_type_get(const Eo *obj EINA_UNUSED, Efl_Access_Object_Data *pd)
{
   return pd->type;
}

EOLIAN void
_efl_access_object_access_type_set(Eo *obj, Efl_Access_Object_Data *pd, Efl_Access_Type val)
{
   Efl_Access_Object *parent;
   if (val == pd->type)
     return;

   parent = efl_provider_find(efl_parent_get(obj), EFL_ACCESS_OBJECT_MIXIN);

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
_efl_access_object_efl_object_invalidate(Eo *obj, Efl_Access_Object_Data *pd)
{
   efl_access_relation_set_free(pd);

   efl_invalidate(efl_super(obj, EFL_ACCESS_OBJECT_MIXIN));
}

EOLIAN void
_efl_access_object_efl_object_destructor(Eo *obj, Efl_Access_Object_Data *pd)
{
   eina_stringshare_del(pd->name);
   eina_stringshare_del(pd->description);
   eina_stringshare_del(pd->translation_domain);

   efl_destructor(efl_super(obj, EFL_ACCESS_OBJECT_MIXIN));
}

void
_efl_access_shutdown(void)
{
   Efl_Access_Event_Handler *hdl;

   EINA_LIST_FREE(global_callbacks, hdl)
     free(hdl);

   ELM_SAFE_DEL(root);
}

#include "efl_access_object.eo.c"
