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
   g->object->grabs = eina_list_append(g->object->grabs, g);
   if (eina_error_get())
     {
	MERR_BAD();
	evas_mem_free(sizeof(Eina_List));
	g->object->grabs = eina_list_append(g->object->grabs, g);
	if (eina_error_get())
	  {
	     MERR_FATAL();
	     free(g);
	     free(g->keyname);
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
	  g->delete_me = 1;
     }
   else
     {
	while (obj->grabs)
	  {
	     Evas_Key_Grab *g;

	     g = obj->grabs->data;
	     if (g->keyname) free(g->keyname);
	     free(g);
	     obj->layer->evas->grabs = eina_list_remove(obj->layer->evas->grabs, g);
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

/**
 * Requests @p keyname key events be directed to @p obj.
 *
 * Key grabs allow an object to receive key events for specific key strokes
 * even if another object has focus.  If the grab is non-exclusive then all
 * objects that have grabs on the key will get the event, however if the
 * grab is exclusive, no other object can get a grab on the key and only
 * that object will get the event.
 *
 * @p keyname is a platform dependent symbolic name for the key pressed.
 * It is sometimes possible to convert the string to an ASCII value of the
 * key, but not always for example the enter key may be returned as the
 * string 'Enter'.
 *
 * Typical platforms are Linux frame buffer (Ecore_FB) and X server (Ecore_X)
 * when using Evas with Ecore and Ecore_Evas.
 *
 * For a list of keynames for the Linux frame buffer, please refer to the
 * Ecore_FB documentation.
 * 
 * @p modifiers and @p not_modifiers are bit masks of all the modifiers that are required and
 * not required respectively for the new grab.  Modifiers can be things such as shift and ctrl
 * as well as user defigned types via evas_key_modifier_add.
 *
 * @see evas_object_key_ungrab
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_key_modifier_add
 * 
 * @param obj the object to direct @p keyname events to.
 * @param keyname the key to request events for.
 * @param modifiers a mask of modifiers that should be present to trigger the event.
 * @param not_modifiers a mask of modifiers that should not be present to trigger the event.
 * @param exclusive request that the @p obj is the only object receiving the @p keyname events.
 * @return Boolean indicating whether the grab succeeded
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
 * Request that the grab on @p obj be removed.
 * 
 * Removes the grab on @p obj if @p keyname, @p modifiers, and @p not_modifiers
 * match.
 * 
 * @see evas_object_key_grab
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_focus_get
 *
 * @param obj the object that has an existing grab.
 * @param keyname the key the grab is for.
 * @param modifiers a mask of modifiers that should be present to trigger the event.
 * @param not_modifiers a mask of modifiers that should not be present to trigger the event.
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
