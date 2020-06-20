#include "evas_common_private.h"
#include "evas_private.h"

/* private calls */

/* FIXME: this is not optimal, but works. i should have a hash of keys per */
/* Evas and then a linked lists of grabs for that key and what */
/* modifiers/not_modifers they use */

static Evas_Key_Grab *evas_key_grab_new  (Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive);
static Evas_Key_Grab *evas_key_grab_find (Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);

static Evas_Key_Grab *
evas_key_grab_new(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
{
   /* MEM OK */
   Eina_List *l;
   Evas_Key_Grab *g;
   Eina_Bool have_exclusion = EINA_FALSE;

   EINA_LIST_FOREACH(obj->layer->evas->grabs, l, g)
     {
        if ((g->modifiers == modifiers) &&
            (g->not_modifiers == not_modifiers) &&
            (!strcmp(g->keyname, keyname)) &&
            (g->exclusive))
          {
             have_exclusion = EINA_TRUE;
             break;
          }
     }

   if (have_exclusion && exclusive) return NULL;

   g = calloc(1, sizeof(Evas_Key_Grab));
   if (!g) return NULL;
   g->object = eo_obj;
   g->modifiers = modifiers;
   g->not_modifiers = not_modifiers;
   g->exclusive = exclusive;
   g->keyname = strdup(keyname);
   if (obj->layer->evas->walking_grabs)
     g->just_added = EINA_TRUE;
   g->is_active = EINA_TRUE;
   if (!g->keyname)
     {
        free(g);
        return NULL;
     }

   if (exclusive)
     {
        Evas_Key_Grab *ge;
        EINA_LIST_FOREACH(obj->layer->evas->grabs, l, ge)
          {
             if ((ge->modifiers == modifiers) &&
                 (ge->not_modifiers == not_modifiers) &&
                 (!strcmp(ge->keyname, keyname)))
               {
                  ge->is_active = EINA_FALSE;
               }
          }
     }
   if (have_exclusion) g->is_active = EINA_FALSE;

   obj->grabs = eina_list_append(obj->grabs, g);
   obj->layer->evas->grabs = eina_list_append(obj->layer->evas->grabs, g);
   return g;
}

static Evas_Key_Grab *
evas_key_grab_find(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Eina_List *l;
   Evas_Key_Grab *g;

   EINA_LIST_FOREACH(obj->layer->evas->grabs, l, g)
     {
        if ((g->modifiers == modifiers) &&
            (g->not_modifiers == not_modifiers) &&
            (!strcmp(g->keyname, keyname)))
          {
             if (eo_obj == g->object) return g;
          }
     }
   return NULL;
}

/* local calls */

void
evas_object_grabs_cleanup(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if ((!obj->layer) || (!obj->layer->evas)) return;
   if (obj->layer->evas->walking_grabs)
     {
        Eina_List *l;
        Evas_Key_Grab *g;

        EINA_LIST_FOREACH(obj->grabs, l, g)
          g->delete_me = EINA_TRUE;
     }
   else
     {
        while (obj->grabs)
          {
             Evas_Key_Grab *g = obj->grabs->data;
             obj->layer->evas->grabs =
               eina_list_remove(obj->layer->evas->grabs, g);
             obj->grabs = eina_list_remove(obj->grabs, g);
             if (g->keyname) free(g->keyname);
             free(g);
          }
     }
}

void
evas_key_grab_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   g = evas_key_grab_find(eo_obj, obj, keyname, modifiers, not_modifiers);
   if (!g) return;
   Evas_Object_Protected_Data *g_object = efl_data_scope_get(g->object, EFL_CANVAS_OBJECT_CLASS);
   g_object->grabs = eina_list_remove(g_object->grabs, g);
   obj->layer->evas->grabs = eina_list_remove(obj->layer->evas->grabs, g);
   if (g->keyname) free(g->keyname);
   free(g);
}

// Legacy implementation. TODO: remove use of Evas_Modifier_Mask

static Eina_Bool
_object_key_grab(Eo *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname,
                 Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers,
                 Eina_Bool exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   if (((modifiers == not_modifiers) && (modifiers != 0)) || (!keyname)) return EINA_FALSE;
   g = evas_key_grab_new(eo_obj, obj, keyname, modifiers, not_modifiers, exclusive);
   return ((!g) ? EINA_FALSE : EINA_TRUE);
}

static void
_object_key_ungrab(Eo *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname,
                   Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Evas_Key_Grab *g;
   Eina_List *l;

   if (!keyname) return;
   g = evas_key_grab_find(eo_obj, obj, keyname, modifiers, not_modifiers);
   if (!g) return;
   Evas_Object_Protected_Data *g_object = efl_data_scope_get(g->object, EFL_CANVAS_OBJECT_CLASS);
   if (g_object->layer->evas->walking_grabs)
     {
        if (!g->delete_me)
          {
             g_object->layer->evas->delete_grabs++;
             g->delete_me = EINA_TRUE;
          }
     }
   else
     {
        if (g->exclusive)
          {
             Evas_Key_Grab *ge;
             EINA_LIST_FOREACH(obj->layer->evas->grabs, l, ge)
               {
                  if ((ge->modifiers == modifiers) &&
                     (ge->not_modifiers == not_modifiers) &&
                     (!strcmp(ge->keyname, keyname)))
                    {
                       if (!ge->is_active) ge->is_active = EINA_TRUE;
                    }
               }
          }

        evas_key_grab_free(g->object, g_object, keyname, modifiers, not_modifiers);
     }
}

static inline Evas_Modifier_Mask
_efl_input_modifier_to_evas_modifier_mask(Evas_Public_Data *e, Efl_Input_Modifier in)
{
   Evas_Modifier_Mask out = 0;
   size_t i;

   static const Efl_Input_Modifier mods[] = {
      EFL_INPUT_MODIFIER_ALT,
      EFL_INPUT_MODIFIER_CONTROL,
      EFL_INPUT_MODIFIER_SHIFT,
      EFL_INPUT_MODIFIER_META,
      EFL_INPUT_MODIFIER_ALTGR,
      EFL_INPUT_MODIFIER_HYPER,
      EFL_INPUT_MODIFIER_SUPER
   };

   for (i = 0; i < EINA_C_ARRAY_LENGTH(mods); i++)
     if (in & mods[i])
       {
          out |= evas_key_modifier_mask_get
                (e->evas, _efl_input_modifier_to_string(mods[i]));
       }

   return out;
}

// EO API

EOLIAN Eina_Bool
_efl_canvas_object_key_grab(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                            const char *keyname, Efl_Input_Modifier mod,
                            Efl_Input_Modifier not_mod, Eina_Bool exclusive)
{
   Evas_Modifier_Mask modifiers, not_modifiers;

   EVAS_OBJECT_DATA_VALID_CHECK(obj, EINA_FALSE);
   modifiers = _efl_input_modifier_to_evas_modifier_mask(obj->layer->evas, mod);
   not_modifiers = _efl_input_modifier_to_evas_modifier_mask(obj->layer->evas, not_mod);

   return _object_key_grab(eo_obj, obj, keyname, modifiers, not_modifiers, exclusive);
}

EOLIAN void
_efl_canvas_object_key_ungrab(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                              const char *keyname, Efl_Input_Modifier mod,
                              Efl_Input_Modifier not_mod)
{
   Evas_Modifier_Mask modifiers, not_modifiers;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   modifiers = _efl_input_modifier_to_evas_modifier_mask(obj->layer->evas, mod);
   not_modifiers = _efl_input_modifier_to_evas_modifier_mask(obj->layer->evas, not_mod);

   _object_key_ungrab(eo_obj, obj, keyname, modifiers, not_modifiers);
}

// Legacy API

EAPI Eina_Bool
evas_object_key_grab(Evas_Object *eo_obj, const char *keyname,
                     Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers,
                     Eina_Bool exclusive)
{
   Evas_Object_Protected_Data *obj;

   obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
   EVAS_OBJECT_DATA_VALID_CHECK(obj, EINA_FALSE);

   return _object_key_grab(eo_obj, obj, keyname, modifiers, not_modifiers, exclusive);
}

EAPI void
evas_object_key_ungrab(Efl_Canvas_Object *eo_obj, const char *keyname,
                       Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   Evas_Object_Protected_Data *obj;

   obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
   EVAS_OBJECT_DATA_VALID_CHECK(obj);

   _object_key_ungrab(eo_obj, obj, keyname, modifiers, not_modifiers);
}
