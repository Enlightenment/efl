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
   return ed;
}

void
_edje_del(Edje *ed)
{
   _edje_file_del(ed);
   if (ed->path) free(ed->path);
   if (ed->part) free(ed->part);
   evas_object_del(ed->clipper);
   printf("FIXME: leak: ed->callbacks\n");
   free(ed);
}
