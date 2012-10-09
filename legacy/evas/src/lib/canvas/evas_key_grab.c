#include "evas_common.h"
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
   if (eina_error_get())
     {
        MERR_BAD();
        evas_mem_free(sizeof(Eina_List));
        obj->grabs = eina_list_append(obj->grabs, g);
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
             obj->grabs = eina_list_remove(obj->grabs, g);
             free(g->keyname);
             free(g);
             return NULL;
          }
     }
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
             if (g->keyname) free(g->keyname);
             free(g);
             obj->layer->evas->grabs = eina_list_remove(obj->layer->evas->grabs,
                                                        g);
             obj->grabs = eina_list_remove(obj->grabs, g);
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
   Evas_Object_Protected_Data *g_object = eo_data_get(g->object, EVAS_OBJ_CLASS);
   g_object->grabs = eina_list_remove(g_object->grabs, g);
   obj->layer->evas->grabs = eina_list_remove(obj->layer->evas->grabs, g);
   if (g->keyname) free(g->keyname);
   free(g);
}

/* public calls */

EAPI Eina_Bool
evas_object_key_grab(Evas_Object *eo_obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Eina_Bool exclusive)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool ret = EINA_FALSE;
   eo_do(eo_obj, evas_obj_key_grab(keyname, modifiers, not_modifiers, exclusive, &ret));
   return ret;
}

void
_key_grab(Eo *eo_obj, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);
   Evas_Modifier_Mask modifiers = va_arg(*list, Evas_Modifier_Mask);
   Evas_Modifier_Mask not_modifiers = va_arg(*list, Evas_Modifier_Mask);
   Eina_Bool exclusive = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   /* MEM OK */
   Evas_Key_Grab *g;

   Evas_Object_Protected_Data *obj = _pd;
   if (!keyname)
      *ret = EINA_FALSE;
   if (exclusive)
     {
        g = evas_key_grab_find(eo_obj, obj, keyname, modifiers, not_modifiers,
                               exclusive);
        if (g)
           *ret = EINA_FALSE;
     }
   g = evas_key_grab_new(eo_obj, obj, keyname, modifiers, not_modifiers, exclusive);
   *ret = (!g) ? EINA_FALSE : EINA_TRUE;
}

EAPI void
evas_object_key_ungrab(Evas_Object *eo_obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_obj, evas_obj_key_ungrab(keyname, modifiers, not_modifiers));
}

void
_key_ungrab(Eo *eo_obj, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);
   Evas_Modifier_Mask modifiers = va_arg(*list, Evas_Modifier_Mask);
   Evas_Modifier_Mask not_modifiers = va_arg(*list, Evas_Modifier_Mask);

   /* MEM OK */
   Evas_Key_Grab *g;

   if (!keyname) return;
   Evas_Object_Protected_Data *obj = _pd;
   g = evas_key_grab_find(eo_obj, obj, keyname, modifiers, not_modifiers, 0);
   if (!g) return;
   Evas_Object_Protected_Data *g_object = eo_data_get(g->object, EVAS_OBJ_CLASS);
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
