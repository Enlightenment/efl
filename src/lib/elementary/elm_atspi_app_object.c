#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

extern Eina_List *_elm_win_list;
static Elm_Atspi_App_Object *instance;

typedef struct _Elm_Atspi_App_Object_Data Elm_Atspi_App_Object_Data;

struct _Elm_Atspi_App_Object_Data
{
   //TODO Efl.Access.Object - remove during refactor
   Eina_List     *relations;
   Eina_List     *attr_list;
   const char    *name;
   const char    *description;
   const char    *translation_domain;
   Efl_Access_Role role;
   Efl_Access_Reading_Info_Type reading_info;
   Efl_Access_Object *access_parent;
};

EOLIAN static Eo*
_elm_atspi_app_object_efl_object_constructor(Eo *obj, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   if (instance)
     {
        ERR("Attempt to crete new instance of Elm_Atspi_App_Object.");
        return NULL;
     }
   obj = efl_constructor(efl_super(obj, ELM_ATSPI_APP_OBJECT_CLASS));
   instance = obj;

   _pd->role = EFL_ACCESS_ROLE_APPLICATION;
   return obj;
}

EOLIAN static void
_elm_atspi_app_object_efl_object_destructor(Eo *obj, Elm_Atspi_App_Object_Data *_pd)
{
   efl_access_removed(obj);
   efl_access_object_attributes_clear(obj);
   efl_access_object_relationships_clear(obj);

   eina_stringshare_del(_pd->name);
   eina_stringshare_del(_pd->description);
   eina_stringshare_del(_pd->translation_domain);

   efl_destructor(efl_super(obj, ELM_ATSPI_APP_OBJECT_CLASS));
}

EOLIAN static Eina_List*
_elm_atspi_app_object_efl_access_object_access_children_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   Eina_List *l, *accs = NULL;
   Evas_Object *win;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        if (!efl_isa(win, EFL_ACCESS_OBJECT_INTERFACE))
          continue;
        accs = eina_list_append(accs, win);
     }

   return accs;
}

EOLIAN static const char*
_elm_atspi_app_object_efl_access_object_i18n_name_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   const char *ret;
   ret = elm_app_name_get();
   return ret;
}

EOLIAN static Elm_Atspi_App_Object*
_elm_atspi_app_object_instance_get(const Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   if (!instance)
     instance = efl_add(ELM_ATSPI_APP_OBJECT_CLASS, efl_main_loop_get());
   return instance;
}

EOLIAN static int
_elm_atspi_app_object_efl_access_object_index_in_parent_get(const Eo *obj, Elm_Atspi_App_Object_Data *pd EINA_UNUSED)
{
   Eina_List *l, *children = NULL;
   Eo *chld, *parent = NULL;
   int ret = 0;

   parent = efl_access_object_access_parent_get(obj);
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

EOLIAN static Efl_Access_Object*
_elm_atspi_app_object_efl_access_object_access_parent_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd EINA_UNUSED)
{
   return pd->access_parent;
}

EOLIAN static void
_elm_atspi_app_object_efl_access_object_access_parent_set(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd EINA_UNUSED, Efl_Access_Object *parent)
{
   pd->access_parent = parent;
}

EOLIAN static void
_elm_atspi_app_object_efl_access_object_attribute_append(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd, const char *key, const char *value)
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

EOLIAN static void _elm_atspi_app_object_efl_access_object_attributes_clear(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd)
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
_elm_atspi_app_object_efl_access_object_reading_info_type_set(Eo *obj, Elm_Atspi_App_Object_Data *pd, Efl_Access_Reading_Info_Type reading_info)
{
   Eina_Strbuf *buf = NULL;
   pd->reading_info = reading_info;
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
_elm_atspi_app_object_efl_access_object_reading_info_type_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd)
{
   return pd->reading_info;
}

EOLIAN static Efl_Access_Role
_elm_atspi_app_object_efl_access_object_role_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd)
{
   return pd->role;
}

EOLIAN static void
_elm_atspi_app_object_efl_access_object_role_set(Eo *obj, Elm_Atspi_App_Object_Data *pd, Efl_Access_Role role)
{
   if (pd->role != role)
     {
        pd->role = role;
        efl_access_role_changed_signal_emit(obj);
     }
}

EOLIAN const char *
_elm_atspi_app_object_efl_access_object_role_name_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd EINA_UNUSED)
{
   return "application";
}

EOLIAN static void
_elm_atspi_app_object_efl_access_object_i18n_name_set(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->name, val);
}

const char * _elm_atspi_app_object_efl_access_object_description_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd)
{
#ifdef ENABLE_NLS
   if (pd->translation_domain)
      return dgettext(pd->translation_domain, pd->description);
#endif
   return pd->description;
}

EOLIAN static void
_elm_atspi_app_object_efl_access_object_description_set(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd, const char *val)
{
   eina_stringshare_replace(&pd->description, val);
}

EOLIAN static const char *
_elm_atspi_app_object_efl_access_object_localized_role_name_get(const Eo *obj, Elm_Atspi_App_Object_Data *pd EINA_UNUSED)
{
   const char *ret = NULL;
   ret = efl_access_object_role_name_get(obj);
#ifdef ENABLE_NLS
   ret = gettext(ret);
#endif
   return ret;
}

EOLIAN Eina_Iterator *
_elm_atspi_app_object_efl_access_object_relations_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd)
{
   return eina_list_iterator_new(pd->relations);
}

EOLIAN void
_elm_atspi_app_object_efl_access_object_translation_domain_set(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd, const char *domain)
{
   eina_stringshare_replace(&pd->translation_domain, domain);
}

EOLIAN Efl_Access_State_Set
_elm_atspi_app_object_efl_access_object_state_set_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd EINA_UNUSED)
{
   return 0;
}

EOLIAN const char*
_elm_atspi_app_object_efl_access_object_translation_domain_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd)
{
   return pd->translation_domain;
}

static void
_on_rel_obj_del(void *data, const Efl_Event *event)
{
   Elm_Atspi_App_Object_Data *sd = data;
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

EOLIAN static Eina_Bool
_elm_atspi_app_object_efl_access_object_relationship_append(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *sd, Efl_Access_Relation_Type type, const Efl_Access_Object *relation)
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
_elm_atspi_app_object_efl_access_object_relationship_remove(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *sd, Efl_Access_Relation_Type type, const Efl_Access_Object *relation)
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
_elm_atspi_app_object_efl_access_object_relationships_clear(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *sd)
{
   Efl_Access_Relation *rel;

   EINA_LIST_FREE(sd->relations, rel)
     {
        Eina_List *l;

        EINA_LIST_FOREACH(rel->objects, l, obj)
          efl_event_callback_del(obj, EFL_EVENT_DEL, _on_rel_obj_del, sd);
        eina_list_free(rel->objects);
        free(rel);
     }
}

EOLIAN static Eina_List*
_elm_atspi_app_object_efl_access_object_attributes_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *pd)
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

#include "elm_atspi_app_object.eo.c"
