#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* all public */
Evas_Smart *
evas_smart_new(char          *name,
	       void         (*func_add) (Evas_Object *o),
	       void         (*func_del) (Evas_Object *o),
	       void         (*func_layer_set) (Evas_Object *o, int l),
	       void         (*func_raise) (Evas_Object *o),
	       void         (*func_lower) (Evas_Object *o),
	       void         (*func_stack_above) (Evas_Object *o, Evas_Object *above),
	       void         (*func_stack_below) (Evas_Object *o, Evas_Object *below),
	       void         (*func_move) (Evas_Object *o, double x, double y),
	       void         (*func_resize) (Evas_Object *o, double w, double h),
	       void         (*func_show) (Evas_Object *o),
	       void         (*func_hide) (Evas_Object *o),
	       void         (*func_color_set) (Evas_Object *o, int r, int g, int b, int a),
	       void         (*func_clip_set) (Evas_Object *o, Evas_Object *clip),
	       void         (*func_clip_unset) (Evas_Object *o),
	       void          *data)
{
   Evas_Smart *s;
   
   if (!name) return NULL;
   
   s = calloc(1, sizeof(Evas_Smart));
   if (!s) return NULL;

   s->magic = MAGIC_SMART;
   
   s->name = strdup(name);
   
   s->func_add = func_add;
   s->func_del = func_del;
   s->func_layer_set = func_layer_set;
   s->func_raise = func_raise;
   s->func_lower = func_lower;
   s->func_stack_above = func_stack_above;
   s->func_stack_below = func_stack_below;
   s->func_move = func_move;
   s->func_resize = func_resize;
   s->func_show = func_show;
   s->func_hide = func_hide;
   s->func_color_set = func_color_set;
   s->func_clip_set = func_clip_set;
   s->func_clip_unset = func_clip_unset;
   s->data = data;
   
   return s;
}

void
evas_smart_free(Evas_Smart *s)
{   
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return;
   MAGIC_CHECK_END();
   s->delete_me = 1;
   if (s->usage > 0) return;
   if (s->name) free(s->name);
   free(s);
}

void *
evas_smart_data_get(Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return s->data;
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
   if (s->delete_me) evas_smart_free(s);
}
