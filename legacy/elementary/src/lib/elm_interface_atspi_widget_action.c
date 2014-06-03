#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define ELM_INTERFACE_ATSPI_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include "elm_interface_atspi_action.eo.h"
#include "elm_interface_atspi_widget_action.eo.h"

extern Eina_Hash *_elm_key_bindings;

EOLIAN static Eina_Bool
_elm_interface_atspi_widget_action_elm_interface_atspi_action_action_do(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Elm_Action *actions = NULL;
   Eina_Bool (*func)(Eo *eo, const char *params) = NULL;
   int tmp = 0;

   eo_do(obj, actions = elm_interface_atspi_widget_action_elm_actions_get());
   if (!actions) return EINA_FALSE;

   while (actions[tmp].name)
     {
        if (tmp == id)
          {
             func = actions[tmp].func;
             break;
          }
        tmp++;
     }

   if (!func)
     return EINA_FALSE;

   return func(obj, NULL);
}

EOLIAN static const char *
_elm_interface_atspi_widget_action_elm_interface_atspi_action_keybinding_get(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Elm_Action *actions = NULL;
   Eina_List *l, *binding_list;
   const char *action = NULL;
   Elm_Config_Binding_Key *binding;
   int tmp = 0;

   if (!eo_isa(obj, ELM_WIDGET_CLASS))
      return NULL;

   eo_do(obj, actions = elm_interface_atspi_widget_action_elm_actions_get());
   if (!actions) return NULL;

   while (actions[tmp].name)
     {
        if (tmp == id)
          {
             action = actions->name;
             break;
          }
        tmp++;
     }
   if (!action) return NULL;

   binding_list = eina_hash_find(_elm_key_bindings, elm_widget_type_get(obj));

   if (binding_list)
     {
        EINA_LIST_FOREACH(binding_list, l, binding)
          {
             if (!strcmp(binding->action, action))
               return binding->key;
          }
     }

   return NULL;
}

EOLIAN static const char *
_elm_interface_atspi_widget_action_elm_interface_atspi_action_name_get(Eo *obj, void *pd EINA_UNUSED, int id)
{
   const Elm_Action *actions = NULL;
   int tmp = 0;

   eo_do(obj, actions = elm_interface_atspi_widget_action_elm_actions_get());
   if (!actions) return EINA_FALSE;

   while (actions[tmp].name)
     {
        if (tmp == id) return actions->name;
        tmp++;
     }
   return NULL;
}

EOLIAN static Eina_Bool
_elm_interface_atspi_widget_action_elm_interface_atspi_action_description_set(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int id EINA_UNUSED, const char *description EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static const char *
_elm_interface_atspi_widget_action_elm_interface_atspi_action_description_get(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, int id EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Eina_List*
_elm_interface_atspi_widget_action_elm_interface_atspi_action_actions_get(Eo *obj, void *pd EINA_UNUSED)
{
   const Elm_Action *actions = NULL;
   Eina_List *ret = NULL;
   int tmp = 0;

   eo_do(obj, actions = elm_interface_atspi_widget_action_elm_actions_get());
   if (!actions) return NULL;

   while (actions[tmp].name)
     {
        ret = eina_list_append(ret, actions[tmp].name);
        tmp++;
     }

   return ret;
}

#include "elm_interface_atspi_widget_action.eo.c"
