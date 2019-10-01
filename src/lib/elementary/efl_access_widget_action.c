#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_ACTION_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED

#include "elm_priv.h"

extern Eina_Hash *_elm_key_bindings;

EOLIAN static Eina_Bool
_efl_access_widget_action_efl_access_action_action_do(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Efl_Access_Action_Data *actions = NULL;
   const char *param;
   Eina_Bool (*func)(Eo *eo, const char *params) = NULL;
   int tmp = 0;

   actions = efl_access_widget_action_elm_actions_get(obj);
   if (!actions) return EINA_FALSE;

   while (actions[tmp].name)
     {
        if (tmp == id)
          {
             func = actions[tmp].func;
             param = actions[tmp].param;
             break;
          }
        tmp++;
     }

   if (!func)
     return EINA_FALSE;

   return func(obj, param);
}

EOLIAN static char*
_efl_access_widget_action_efl_access_action_action_keybinding_get(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Efl_Access_Action_Data *actions = NULL;
   Eina_List *l1, *binding_list;
   const char *action = NULL, *param = NULL;
   Elm_Config_Binding_Key *binding;
   int tmp = 0;

   if (!efl_isa(obj, EFL_UI_WIDGET_CLASS))
      return NULL;

   actions = efl_access_widget_action_elm_actions_get(obj);
   if (!actions) return NULL;

   while (actions[tmp].name)
     {
        if (tmp == id)
          {
             action = actions[tmp].action;
             param = actions[tmp].param;
             break;
          }
        tmp++;
     }
   if (!action) return NULL;

   binding_list = eina_hash_find(_elm_key_bindings, elm_widget_type_get(obj));

   if (binding_list)
     {
        EINA_LIST_FOREACH(binding_list, l1, binding)
          {
             if (!strcmp(binding->action, action) && (!param ||
                 !strcmp(binding->params, param)))
               {
                  Eina_List *l2;
                  Elm_Config_Binding_Modifier *bm;
                  char *ret;
                  Eina_Strbuf *buf = eina_strbuf_new();
                  eina_strbuf_append_printf(buf, "%s", binding->key);
                  EINA_LIST_FOREACH(binding->modifiers, l2, bm)
                    if (bm->flag) eina_strbuf_append_printf(buf, "+%s", bm->mod);
                  ret = eina_strbuf_string_steal(buf);
                  eina_strbuf_free(buf);
                  return ret;
               }
          }
     }

   return NULL;
}

EOLIAN static const char *
_efl_access_widget_action_efl_access_action_action_name_get(const Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Efl_Access_Action_Data *actions = NULL;
   int tmp = 0;

   actions = efl_access_widget_action_elm_actions_get(obj);
   if (!actions) return NULL;

   while (actions[tmp].name)
     {
        if (tmp == id) return actions[tmp].name;
        tmp++;
     }
   return NULL;
}

EOLIAN static Eina_Bool
_efl_access_widget_action_efl_access_action_action_description_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int id EINA_UNUSED, const char *description EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static const char *
_efl_access_widget_action_efl_access_action_action_description_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int id EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_List*
_efl_access_widget_action_efl_access_action_actions_get(const Eo *obj, void *pd EINA_UNUSED)
{
   const Efl_Access_Action_Data *actions = NULL;
   Eina_List *ret = NULL;
   int tmp = 0;

   actions = efl_access_widget_action_elm_actions_get(obj);
   if (!actions) return NULL;

   while (actions[tmp].name)
     {
        ret = eina_list_append(ret, actions[tmp].name);
        tmp++;
     }

   return ret;
}

#include "efl_access_widget_action.eo.c"
