#include "Edje.h"
#include "edje_private.h"

void
edje_init(void)
{
   static int initted = 0;
   
   if (initted) return;
   initted = 1;
   _edje_edd_setup();
   _edje_text_init();
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
   while (ed->color_classes)
     {
	Ejde_Color_Class *cc;
	
	cc = ed->color_classes->data;
	ed->color_classes = evas_list_remove(ed->color_classes, cc);
	if (cc->name) free(cc->name);
	free(cc);
     }
   while (ed->text_classes)
     {
	Ejde_Text_Class *tc;
	
	tc = ed->text_classes->data;
	ed->text_classes = evas_list_remove(ed->text_classes, tc);
	if (tc->name) free(tc->name);
	if (tc->font) free(tc->font);
	free(tc);
     }
   free(ed);
}

void
_edje_clean_objects(Edje *ed)
{
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
