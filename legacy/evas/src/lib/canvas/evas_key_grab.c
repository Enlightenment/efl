#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* private calls */

/* FIXME: this is not optimal, but works. i should have a hash of keys per */
/* Evas and then a linked lists of grabs for that key and what */
/* modifiers/not_modifers they use */

static Evas_Key_Grab *evas_key_grab_new  (Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int exclusive);
static void           evas_key_grab_free (Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);
static Evas_Key_Grab *evas_key_grab_find (Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);

static Evas_Key_Grab *
evas_key_grab_new(Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;
   
   g = evas_mem_calloc(sizeof(Evas_Key_Grab));
   if (!g) return;
   g->keyname = strdup(keyname);
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
   g->object = obj;
   g->modifiers = modifiers;
   g->not_modifiers = not_modifiers;
   g->exclusive = exclusive;
   return g;
}

static void
evas_key_grab_free(Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Evas_Key_Grab *g;
   
   g = evas_key_grab_find(obj, keyname, modifiers, not_modifiers);
   if (!g) return;
   g->object->grabs = evas_list_remove(g->object->grabs, g);
   if (g->keyname) free(g->keyname);
   free(g);
}

static Evas_Key_Grab *
evas_key_grab_find(Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
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
	     if ((!obj) ||  (obj == g->object)) return g;
	  }
     }
   return NULL;
}

/* local calls */

void
evas_object_grabs_cleanup(Evas_Object *obj)
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

/* public calls */

int
evas_object_key_grab(Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, int exclusive)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   if (exclusive)
     {
	g = evas_key_grab_find(NULL, keyname, modifiers, not_modifiers);
	if (g) return 0;
     }
   g = evas_key_grab_new(obj, keyname, modifiers, not_modifiers, exclusive);
   if (!g) return 0;
   return 1;
}

void
evas_object_key_ungrab(Evas_Object *obj, char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers)
{
   /* MEM OK */
   Evas_Key_Grab *g;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   g = evas_key_grab_find(obj, keyname, modifiers, not_modifiers);
   if (!g) return;
   evas_key_grab_free(g->object, keyname, modifiers, not_modifiers);
}
