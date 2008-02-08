#include "evas_common.h"
#include "evas_private.h"

/* all public */

/**
 * @defgroup Evas_Smart_Group Evas Smart Functions
 *
 * Functions that deal with Evas_Smart's.
 *
 */

/**
 * Create an Evas_Smart, which can be used to instantiate new smart objects.
 *
 * This function internally creates an Evas_Smart_Class and sets the
 * provided callbacks. Callbacks that are unneeded (or marked DEPRECATED
 * below) should be set to NULL.
 *
 * Alternatively you can create an Evas_Smart_Class yourself and use 
 * evas_smart_class_new().
 *
 * @param name a unique name for the smart
 * @param func_add callback called when smart object is added
 * @param func_del callback called when smart object is deleted
 * @param func_layer_set DEPRECATED
 * @param func_raise DEPRECATED
 * @param func_lower DEPRECATED
 * @param func_stack_above DEPRECATED
 * @param func_stack_below DEPRECATED
 * @param func_move callback called when smart object is moved 
 * @param func_resize callback called when smart object is resized 
 * @param func_show callback called when smart object is shown
 * @param func_hide callback called when smart object is hidden
 * @param func_color_set callback called when smart object has its color set
 * @param func_clip_set callback called when smart object has its clip set
 * @param func_clip_unset callback called when smart object has its clip unset
 * @param data a pointer to user data for the smart
 * @return an Evas_Smart
 *
 */
EAPI Evas_Smart *
evas_smart_new(const char *name,
	       void      (*func_add) (Evas_Object *o),
	       void      (*func_del) (Evas_Object *o),
	       void      (*func_layer_set) (Evas_Object *o, int l),
	       void      (*func_raise) (Evas_Object *o),
	       void      (*func_lower) (Evas_Object *o),
	       void      (*func_stack_above) (Evas_Object *o, Evas_Object *above),
	       void      (*func_stack_below) (Evas_Object *o, Evas_Object *below),
	       void      (*func_move) (Evas_Object *o, Evas_Coord x, Evas_Coord y),
	       void      (*func_resize) (Evas_Object *o, Evas_Coord w, Evas_Coord h),
	       void      (*func_show) (Evas_Object *o),
	       void      (*func_hide) (Evas_Object *o),
	       void      (*func_color_set) (Evas_Object *o, int r, int g, int b, int a),
	       void      (*func_clip_set) (Evas_Object *o, Evas_Object *clip),
	       void      (*func_clip_unset) (Evas_Object *o),
	       const void *data)
{
   Evas_Smart *s;
   Evas_Smart_Class *sc;

   printf("----- WARNING. evas_smart_new() will be deprecated and removed soon\n"
	  "----- Please use evas_smart_class_new() instead\n");
   
   if (!name) return NULL;

   s = evas_mem_calloc(sizeof(Evas_Smart));
   if (!s) return NULL;

   s->magic = MAGIC_SMART;

   s->class_allocated = 1;

   sc = evas_mem_calloc(sizeof(Evas_Smart_Class));
   if (!sc)
     {
	free(s);
	return NULL;
     }
   sc->name = name;
   sc->add = func_add;
   sc->del = func_del;
   sc->move = func_move;
   sc->resize = func_resize;
   sc->show = func_show;
   sc->hide = func_hide;
   sc->color_set = func_color_set;
   sc->clip_set = func_clip_set;
   sc->clip_unset = func_clip_unset;
   sc->data = (void *)data;
   s->smart_class = sc;

   return s;
}

/**
 * Free an Evas_Smart
 *
 * If this smart was created using evas_smart_class_new(), the associated
 * Evas_Smart_Class will not be freed.
 *
 * @param s the Evas_Smart to free
 *
 */
EAPI void
evas_smart_free(Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return;
   MAGIC_CHECK_END();
   s->delete_me = 1;
   if (s->usage > 0) return;
   if (s->class_allocated) free((void *)s->smart_class);
   free(s);
}

/**
 * Creates an Evas_Smart from an Evas_Smart_Class.
 *
 * @param Evas_Smart_Class the smart class definition
 * @return an Evas_Smart
 */
EAPI Evas_Smart *
evas_smart_class_new(const Evas_Smart_Class *sc)
{
   Evas_Smart *s;

   if (!sc) return NULL;

   /* api does not match abi! for now refuse as we only have 1 version */
   if (sc->version != EVAS_SMART_CLASS_VERSION) return NULL;
   
   s = evas_mem_calloc(sizeof(Evas_Smart));
   if (!s) return NULL;

   s->magic = MAGIC_SMART;

   s->smart_class = sc;

   return s;
}

/**
 * Get the Evas_Smart_Class of an Evas_Smart
 *
 * @param s the Evas_Smart
 * @return the Evas_Smart_Class
 */
EAPI const Evas_Smart_Class *
evas_smart_class_get(const Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return s->smart_class;
}

/**
 * Get the data pointer set on an Evas_Smart.
 *
 * This data pointer is set either as the final parameter to 
 * evas_smart_new or as the data field in the Evas_Smart_Class passed
 * in to evas_smart_class_new
 *
 * @param Evas_Smart 
 *
 */
EAPI void *
evas_smart_data_get(const Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return (void *)s->smart_class->data;
}

/* internal funcs */
void
evas_object_smart_use(Evas_Smart *s)
{
   s->usage++;
}

void
evas_object_smart_unuse(Evas_Smart *s)
{
   s->usage--;
   if ((s->usage <= 0) && (s->delete_me)) evas_smart_free(s);
}
