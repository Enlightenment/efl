#include "Edje.h"
#include "edje_private.h"

void
edje_part_geometry_get(Evas_Object *o, char *part, double *x, double *y, double *w, double *h )
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(o);   
   if ((!ed) || (!part))
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if ((!strcmp(rp->part->name, part)) && (rp->calculated))
	  {
	     if (x) *x = rp->x;
	     if (y) *y = rp->y;
	     if (w) *w = rp->w;
	     if (h) *h = rp->h;
	     return;
	  }
     }
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
}

Edje *
_edje_fetch(Evas_Object *obj)
{
   Edje *ed;
   char *type;
   
   type = (char *)evas_object_type_get(obj);
   if (!type) return NULL;
   if (strcmp(type, "edje")) return NULL;
   ed = evas_object_smart_data_get(obj);
   return ed;
}

int
_edje_glob_match(char *str, char *glob)
{
   if (!fnmatch(glob, str, 0)) return 1;
   return 0;
}

int
_edje_freeze(Edje *ed)
{
   ed->freeze++;
   return ed->freeze;
}

int
_edje_thaw(Edje *ed)
{
   ed->freeze--;
   if ((ed->freeze <= 0) && (ed->recalc))
     _edje_recalc(ed);
   return ed->freeze;
}
