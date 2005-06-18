#include "evas_common.h"
#include "evas_private.h"

/* all public */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
Evas_Smart *
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

   if (!name) return NULL;

   s = evas_mem_calloc(sizeof(Evas_Smart));
   if (!s) return NULL;

   s->magic = MAGIC_SMART;

   s->class_allocated = 1;

   s->smart_class = evas_mem_calloc(sizeof(Evas_Smart_Class));
   if (!s->smart_class)
     {
	free(s);
	return NULL;
     }
   s->smart_class->name = name;
   s->smart_class->add = func_add;
   s->smart_class->del = func_del;
   s->smart_class->layer_set = func_layer_set;
   s->smart_class->raise = func_raise;
   s->smart_class->lower = func_lower;
   s->smart_class->stack_above = func_stack_above;
   s->smart_class->stack_below = func_stack_below;
   s->smart_class->move = func_move;
   s->smart_class->resize = func_resize;
   s->smart_class->show = func_show;
   s->smart_class->hide = func_hide;
   s->smart_class->color_set = func_color_set;
   s->smart_class->clip_set = func_clip_set;
   s->smart_class->clip_unset = func_clip_unset;
   s->smart_class->data = (void *)data;

   return s;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
void
evas_smart_free(Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return;
   MAGIC_CHECK_END();
   s->delete_me = 1;
   if (s->usage > 0) return;
   if (s->class_allocated) free(s->smart_class);
   free(s);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
Evas_Smart *
evas_smart_class_new(Evas_Smart_Class *sc)
{
   Evas_Smart *s;

   if (!sc) return NULL;

   s = evas_mem_calloc(sizeof(Evas_Smart));
   if (!s) return NULL;

   s->magic = MAGIC_SMART;

   s->smart_class = sc;

   return s;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
Evas_Smart_Class *
evas_smart_class_get(Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return s->smart_class;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 *
 */
void *
evas_smart_data_get(Evas_Smart *s)
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
