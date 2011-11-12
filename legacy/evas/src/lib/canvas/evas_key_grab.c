#include "evas_common.h"
#include "evas_private.h"

/* private calls */

/* FIXME: this is not optimal, but works. i should have a hash of keys per */
/* Evas and then a linked lists of grabs for that key and what */
/* modifiers/not_modifers they use */

static Evas_Key_Grab *evas_key_grab_new  (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive);
static Evas_Key_Grab *evas_key_grab_find (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive);

static Evas_Key_Grab *
evas_key_grab_new(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   g = evas_mem_calloc(sizeof(Evas_Key_Grab));
   if (!g) return NULL;
   g->object = obj;
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
   g->object->grabs = eina_list_append(g->object->grabs, g);
   if (eina_error_get())
     {
        MERR_BAD();
        evas_mem_free(sizeof(Eina_List));
        g->object->grabs = eina_list_append(g->object->grabs, g);
        if (eina_error_get())
          {
             MERR_FATAL();
             free(g->keyname);
             free(g);
             return NULL;
          }
     }
   obj->layer->evas->grabs = eina_list_append(obj->layer->evas->grabs, g);
   if (eina_error_get())
     {
        MERR_BAD();
        evas_mem_free(sizeof(Eina_List));
        obj->layer->evas->grabs = eina_list_append(obj->layer->evas->grabs, g);
        if (eina_error_get())
          {
             MERR_FATAL();
             g->object->grabs = eina_list_remove(g->object->grabs, g);
             free(g->keyname);
             free(g);
             return NULL;
          }
     }
   return g;
}

static Evas_Key_Grab *
evas_key_grab_find(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
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
             if ((exclusive) ||  (obj == g->object)) return g;
          }
     }
   return NULL;
}

/* local calls */

void
evas_object_grabs_cleanup(Evas_Object *obj)
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
             if (g->keyname) free(g->keyname);
             free(g);
             obj->layer->evas->grabs = eina_list_remove(obj->layer->evas->grabs,
                                                        g);
             obj->grabs = eina_list_remove(obj->grabs, g);
          }
     }
}

void
evas_key_grab_free(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   g = evas_key_grab_find(obj, keyname, modifiers, not_modifiers, 0);
   if (!g) return;
   g->object->grabs = eina_list_remove(g->object->grabs, g);
   obj->layer->evas->grabs = eina_list_remove(obj->layer->evas->grabs, g);
   if (g->keyname) free(g->keyname);
   free(g);
}

/* public calls */

EAPI Eina_Bool
evas_object_key_grab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   if (!keyname) return EINA_FALSE;
   if (exclusive)
     {
        g = evas_key_grab_find(obj, keyname, modifiers, not_modifiers,
                               exclusive);
        if (g) return EINA_FALSE;
     }
   g = evas_key_grab_new(obj, keyname, modifiers, not_modifiers, exclusive);
   if (!g) return EINA_FALSE;
   return EINA_TRUE;
}

EAPI void
evas_object_key_ungrab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   g = evas_key_grab_find(obj, keyname, modifiers, not_modifiers, 0);
   if (!g) return;
   if (g->object->layer->evas->walking_grabs)
     {
        if (!g->delete_me)
          {
             g->object->layer->evas->delete_grabs++;
             g->delete_me = EINA_TRUE;
          }
     }
   else
     evas_key_grab_free(g->object, keyname, modifiers, not_modifiers);
}
