#include "evas_common_private.h"
#include "evas_private.h"

/* private calls */

/* FIXME: this is not optimal, but works. i should have a hash of keys per */
/* Evas and then a linked lists of grabs for that key and what */
/* modifiers/not_modifers they use */

static Evas_Key_Grab *evas_key_grab_new  (Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive);
static Evas_Key_Grab *evas_key_grab_find (Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive);

static Evas_Key_Grab *
evas_key_grab_new(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   g = evas_mem_calloc(sizeof(Evas_Key_Grab));
   if (!g) return NULL;
   g->object = eo_obj;
   g->modifiers = modifiers;
   g->not_modifiers = not_modifiers;
   g->exclusive = exclusive;
   g->keyname = strdup(keyname);
   if (obj->layer->evas->walking_grabs)
     g->just_added = EINA_TRUE;
   if (!g->keyname)
     {
        if (!evas_mem_free(strlen(keyname) + 1))
          {
             free(g);
             return NULL;
          }
        g->keyname = strdup(keyname);
        if (!g->keyname)
          {
             free(g);
             return NULL;
          }
     }
   obj->grabs = eina_list_append(obj->grabs, g);
   obj->layer->evas->grabs = eina_list_append(obj->layer->evas->grabs, g);
   return g;
}

static Evas_Key_Grab *
evas_key_grab_find(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
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
             if ((exclusive) ||  (eo_obj == g->object)) return g;
          }
     }
   return NULL;
}

/* local calls */

void
evas_object_grabs_cleanup(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
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

   g = evas_key_grab_find(eo_obj, obj, keyname, modifiers, not_modifiers, 0);
   if (!g) return;
   Evas_Object_Protected_Data *g_object = eo_data_scope_get(g->object, EVAS_OBJECT_CLASS);
   g_object->grabs = eina_list_remove(g_object->grabs, g);
   obj->layer->evas->grabs = eina_list_remove(obj->layer->evas->grabs, g);
   if (g->keyname) free(g->keyname);
   free(g);
}

/* public calls */

EOLIAN Eina_Bool
_evas_object_key_grab(Eo *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   if (((modifiers == not_modifiers) && (modifiers != 0)) || (!keyname)) return EINA_FALSE;
   if (exclusive)
     {
        g = evas_key_grab_find(eo_obj, obj, keyname, modifiers, not_modifiers,
                               exclusive);
        if (g) return EINA_FALSE;
     }
   g = evas_key_grab_new(eo_obj, obj, keyname, modifiers, not_modifiers, exclusive);
   return ((!g) ? EINA_FALSE : EINA_TRUE);
}

EOLIAN void
_evas_object_key_ungrab(Eo *eo_obj, Evas_Object_Protected_Data *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   if (!keyname) return;
   g = evas_key_grab_find(eo_obj, obj, keyname, modifiers, not_modifiers, 0);
   if (!g) return;
   Evas_Object_Protected_Data *g_object = eo_data_scope_get(g->object, EVAS_OBJECT_CLASS);
   if (g_object->layer->evas->walking_grabs)
     {
        if (!g->delete_me)
          {
             g_object->layer->evas->delete_grabs++;
             g->delete_me = EINA_TRUE;
          }
     }
   else
     evas_key_grab_free(g->object, g_object, keyname, modifiers, not_modifiers);
}

