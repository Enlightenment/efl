#include "Edje.h"
#include "edje_private.h"

int
edje_part_exists(Evas_Object *obj, const char *part)
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return 0;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if (!strcmp(rp->part->name, part)) return 1;
     }
   return 0;
}

void
edje_part_geometry_get(Evas_Object *obj, const char *part, double *x, double *y, double *w, double *h )
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);
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

void
edje_part_text_set(Evas_Object *obj, const char *part, const char *text)
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if (!strcmp(rp->part->name, part))
	  {
	     if (rp->part->type == EDJE_PART_TYPE_TEXT)
	       {
		  if (((rp->text.text) && (text)) ||
		      ((!rp->text.text) && (!text)))
		    return;
		  if ((rp->text.text) && (text) && 
		      (!strcmp(rp->text.text, text)))
		    return;
		  if (rp->text.text) free(rp->text.text);
		  rp->text.text = strdup(text);
		  ed->dirty = 1;
		  rp->dirty = 1;
		  _edje_recalc(ed);
	       }
	     return;
	  }
     }
}

const char *
edje_part_text_get(Evas_Object *obj, const char *part)
{
   Evas_List *l;
   Edje *ed;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return NULL;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if (!strcmp(rp->part->name, part))
	  {
	     if (rp->part->type == EDJE_PART_TYPE_TEXT)
	       return evas_object_text_text_get(rp->object);
	     else
	       return NULL;
	  }
     }
   return NULL;
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
