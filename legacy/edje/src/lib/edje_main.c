#include "Edje.h"
#include "edje_private.h"

void
edje_init(void)
{
   static int initted = 0;
   
   if (initted) return;
   initted = 1;
   _edje_edd_setup();
}

Edje *
_edje_add(Evas_Object *obj)
{
   Edje *ed;
   
   ed = calloc(1, sizeof(Edje));
   if (!ed) return NULL;
   ed->evas = evas_object_evas_get(obj);
   ed->clipper = evas_object_rectangle_add(ed->evas);
   evas_object_smart_member_add(ed->clipper, obj);
   evas_object_color_set(ed->clipper, 255, 255, 255, 255);
   evas_object_move(ed->clipper, 0, 0);
   evas_object_resize(ed->clipper, 0, 0);
   evas_object_pass_events_set(ed->clipper, 1);
   ed->have_objects = 1;
   ed->references = 1;
   return ed;
}

void
_edje_del(Edje *ed)
{
   _edje_file_del(ed);
   _edje_clean_objects(ed);
   if (ed->path) free(ed->path);
   if (ed->part) free(ed->part);
   while (ed->callbacks)
     {
	Edje_Signal_Callback *escb;
	
	escb = ed->callbacks->data;
	ed->callbacks = evas_list_remove(ed->callbacks, escb);
	free(escb->signal);
	free(escb->source);
	free(escb);
     }
   free(ed);
}

void
_edje_clean_part_objects(Edje *ed)
{
   Evas_List *l;
   
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;
	evas_object_del(rp->object);
	rp->object = NULL;
     }   
}

void
_edje_clean_objects(Edje *ed)
{
   Evas_List *l;
   
   ed->have_objects = 0;
   _edje_clean_part_objects(ed);
   evas_object_del(ed->clipper);
   ed->evas = NULL;
   ed->obj = NULL;
   ed->clipper = NULL;
}

void
_edje_ref(Edje *ed)
{
   ed->references++;
}

void
_edje_unref(Edje *ed)
{
   ed->references--;
   if (ed->references <= 0)
     _edje_del(ed);
}
