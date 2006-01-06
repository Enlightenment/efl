#include "evas_common.h"
#include "evas_private.h"

/* private calls */

/* FIXME: this is not optimal, but works. i should have a hash of keys per */
/* Evas and then a linked lists of grabs for that key and what */
/* modifiers/not_modifers they use */

static Evas_Key_Grab *evas_key_grab_new  (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int exclusive);
static Evas_Key_Grab *evas_key_grab_find (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int exclusive);

static Evas_Key_Grab *
evas_key_grab_new(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int exclusive)
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
     g->just_added = 1;
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
   g->object->grabs = evas_list_append(g->object->grabs, g);
   if (evas_list_alloc_error())
     {
	MERR_BAD();
	evas_mem_free(sizeof(Evas_List));
	g->object->grabs = evas_list_append(g->object->grabs, g);
	if (evas_list_alloc_error())
	  {
	     MERR_FATAL();
	     free(g);
	     free(g->keyname);
	     return NULL;
	  }
     }
   obj->layer->evas->grabs = evas_list_append(obj->layer->evas->grabs, g);
   if (evas_list_alloc_error())
     {
	MERR_BAD();
	evas_mem_free(sizeof(Evas_List));
	obj->layer->evas->grabs = evas_list_append(obj->layer->evas->grabs, g);
	if (evas_list_alloc_error())
	  {
	     MERR_FATAL();
	     g->object->grabs = evas_list_remove(g->object->grabs, g);
	     free(g);
	     free(g->keyname);
	     return NULL;
	  }
     }
   return g;
}

static Evas_Key_Grab *
evas_key_grab_find(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int exclusive)
{
   /* MEM OK */
   Evas_List *l;

   for (l = obj->layer->evas->grabs; l; l = l->next)
     {
	Evas_Key_Grab *g;

	g = l->data;
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
	Evas_List *l;

	for (l = obj->grabs; l; l = l->next)
	  {
	     Evas_Key_Grab *g;

	     g = l->data;
	     g->delete_me = 1;
	  }
     }
   else
     {
	while (obj->grabs)
	  {
	     Evas_Key_Grab *g;

	     g = obj->grabs->data;
	     if (g->keyname) free(g->keyname);
	     free(g);
	     obj->layer->evas->grabs = evas_list_remove(obj->layer->evas->grabs, g);
	     obj->grabs = evas_list_remove(obj->grabs, g);
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
   g->object->grabs = evas_list_remove(g->object->grabs, g);
   obj->layer->evas->grabs = evas_list_remove(obj->layer->evas->grabs, g);
   if (g->keyname) free(g->keyname);
   free(g);
}

/* public calls */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
EAPI Evas_Bool
evas_object_key_grab(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Evas_Bool exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (!keyname) return 0;
   if (exclusive)
     {
	g = evas_key_grab_find(obj, keyname, modifiers, not_modifiers, exclusive);
	if (g) return 0;
     }
   g = evas_key_grab_new(obj, keyname, modifiers, not_modifiers, exclusive);
   if (!g) return 0;
   return 1;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
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
	     g->delete_me = 1;
	  }
     }
   else
     evas_key_grab_free(g->object, keyname, modifiers, not_modifiers);
}
