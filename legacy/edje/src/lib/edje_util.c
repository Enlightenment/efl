#include "Edje.h"
#include "edje_private.h"

int
edje_object_freeze(Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   return _edje_freeze(ed);
}

int
edje_object_thaw(Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   return _edje_thaw(ed);
}

void
edje_object_color_class_set(Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Edje *ed;
   Evas_List *l;
   Ejde_Color_Class *cc;

   ed = _edje_fetch(obj);
   if ((!ed) || (!color_class)) return;
   if (r < 0)   r = 0;
   if (r > 255) r = 255;
   if (g < 0)   g = 0;
   if (g > 255) g = 255;
   if (b < 0)   b = 0;
   if (b > 255) b = 255;
   if (a < 0)   a = 0;
   if (a > 255) a = 255;
   for (l = ed->color_classes; l; l = l->next)
     {
	cc = l->data;
	if (!strcmp(cc->name, color_class))
	  {
	     if ((cc->r == r) && (cc->g == g) && 
		 (cc->b == b) && (cc->a == a) &&
		 (cc->r2 == r2) && (cc->g2 == g2) && 
		 (cc->b2 == b2) && (cc->a2 == a2) &&
		 (cc->r3 == r3) && (cc->g3 == g3) && 
		 (cc->b3 == b3) && (cc->a3 == a3))
	       return;
	     cc->r = r;
	     cc->g = g;
	     cc->b = b;
	     cc->a = a;
	     cc->r2 = r2;
	     cc->g2 = g2;
	     cc->b2 = b2;
	     cc->a2 = a2;
	     cc->r3 = r3;
	     cc->g3 = g3;
	     cc->b3 = b3;
	     cc->a3 = a3;
	     ed->dirty = 1;
	     _edje_recalc(ed);
	     return;
	  }
     }
   cc = malloc(sizeof(Ejde_Color_Class));
   cc->name = strdup(color_class);
   if (!cc->name)
     {
	free(cc);
	return;
     }
   cc->r = r;
   cc->g = g;
   cc->b = b;
   cc->a = a;
   cc->r2 = r2;
   cc->g2 = g2;
   cc->b2 = b2;
   cc->a2 = a2;
   cc->r3 = r3;
   cc->g3 = g3;
   cc->b3 = b3;
   cc->a3 = a3;
   ed->color_classes = evas_list_append(ed->color_classes, cc);
   ed->dirty = 1;
   _edje_recalc(ed);
}

void
edje_object_text_class_set(Evas_Object *obj, const char *text_class, const char *font, double size)
{
   Edje *ed;
   Evas_List *l;
   Ejde_Text_Class *tc;

   ed = _edje_fetch(obj);
   if ((!ed) || (!text_class)) return;
   if (size < 0.0) size = 0.0;
   for (l = ed->text_classes; l; l = l->next)
     {
	tc = l->data;
	if (!strcmp(tc->name, text_class))
	  {
	     if ((tc->font) && (font) && 
		 (!strcmp(tc->font, font)) &&
		 (tc->size == size))
	       return;
	     if ((!tc->font) && (!font) && 
		 (tc->size == size))
	       return;
	     if (tc->font) free(tc->font);
	     if (font) tc->font = strdup(font);
	     else tc->font = NULL;
	     tc->size = size;
	     ed->dirty = 1;
	     _edje_recalc(ed);
	     return;
	  }
     }
   tc = malloc(sizeof(Ejde_Text_Class));
   tc->name = strdup(text_class);
   if (!tc->name)
     {
	free(tc);
	return;
     }
   if (font) tc->font = strdup(font);
   else tc->font = NULL;
   tc->size = size;
   ed->text_classes = evas_list_append(ed->text_classes, tc);
   ed->dirty = 1;
   _edje_recalc(ed);
}

int
edje_object_part_exists(Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return 0;
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp) return 0;
   return 1;
}

void
edje_object_part_geometry_get(Evas_Object *obj, const char *part, double *x, double *y, double *w, double *h )
{
   Evas_List *l;
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
}

void
edje_object_part_text_set(Evas_Object *obj, const char *part, const char *text)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_TEXT) return;
   if ((!rp->text.text) && (!text))
     return;
   if ((rp->text.text) && (text) && 
       (!strcmp(rp->text.text, text)))
     return;
   if (rp->text.text) free(rp->text.text);
   rp->text.text = strdup(text);
   ed->dirty = 1;
   _edje_recalc(ed);
}

const char *
edje_object_part_text_get(Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->type == EDJE_PART_TYPE_TEXT)
     return evas_object_text_text_get(rp->object);
   return NULL;
}

void
edje_object_part_swallow(Evas_Object *obj, const char *part, Evas_Object *obj_swallow)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp) return;
   if (rp->swallowed_object)
     {
	evas_object_clip_unset(rp->swallowed_object);
	rp->swallowed_object = NULL;
     }
   if (!obj_swallow) return;
   rp->swallowed_object = obj_swallow;
   evas_object_clip_set(rp->swallowed_object, ed->clipper);
   evas_object_stack_above(rp->swallowed_object, rp->object);
   ed->dirty = 1;
   _edje_recalc(ed);   
}

void
edje_object_part_unswallow(Evas_Object *obj, Evas_Object *obj_swallow)
{
   Edje *ed;
   Evas_List *l;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!obj_swallow)) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;
	if (rp->swallowed_object == obj_swallow)
	  {
	     evas_object_clip_unset(rp->swallowed_object);
	     rp->swallowed_object = NULL;
	     ed->dirty = 1;
	     _edje_recalc(ed);
	     return;
	  }
     }
}

Evas_Object *
edje_object_part_swallow_get(Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp) return NULL;
   return rp->swallowed_object;
}

void
edje_object_size_min_get(Evas_Object *obj, double *minw, double *minh)
{
   Edje *ed;
   double pw, ph;   
   int maxw, maxh;
   int ok;
   
   ed = _edje_fetch(obj);
   ed->calc_only = 1;
   pw = ed->w;
   ph = ed->h;
   ed->w = 0;
   ed->h = 0;
   
   maxw = 0;
   maxh = 0;
   
   ok = 1;
   while (ok)
     {
	Evas_List *l;
	
	ok = 0;
	ed->dirty = 1;
	_edje_recalc(ed);
	for (l = ed->parts; l; l = l->next)
	  {
	     Edje_Real_Part *ep;
	     int w, h;
	     
	     ep = l->data;
	     w = ep->w - ep->req.w;
	     h = ep->h - ep->req.h;
	     if (w > maxw)
	       {
		  maxw = w;
		  ok = 1;
	       }
	     if (h > maxh)
	       {
		  maxh = h;
		  ok = 1;
	       }
	  }
	if (ok)
	  {
	     ed->w += maxw;
	     ed->h += maxh;
	  }
     }
   ed->min.w = ed->w;
   ed->min.h = ed->h;
   
   if (minw) *minw = ed->min.w;
   if (minh) *minh = ed->min.h;
   
   ed->w = pw;
   ed->h = ph;
   ed->dirty = 1;
   _edje_recalc(ed);
   ed->calc_only = 0;
}





Edje_Real_Part *
_edje_real_part_get(Edje *ed, char *part)
{
   Evas_List *l;

   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *rp;
	
	rp = l->data;	
	if (!strcmp(rp->part->name, part)) return rp;
     }
   return NULL;
}

Ejde_Color_Class *
_edje_color_class_find(Edje *ed, char *color_class)
{
   Evas_List *l;
   
   if (!color_class) return NULL;
   for (l = ed->color_classes; l; l = l->next)
     {
	Ejde_Color_Class *cc;
	
	cc = l->data;
	if (!strcmp(color_class, cc->name)) return cc;
     }
   return NULL;
}

Ejde_Text_Class *
_edje_text_class_find(Edje *ed, char *text_class)
{
   Evas_List *l;
   
   if (!text_class) return NULL;
   for (l = ed->text_classes; l; l = l->next)
     {
	Ejde_Text_Class *tc;
	
	tc = l->data;
	if (!strcmp(text_class, tc->name)) return tc;
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
