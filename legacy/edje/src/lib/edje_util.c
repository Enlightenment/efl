#include "Edje.h"
#include "edje_private.h"

Evas_Hash *_edje_color_class_hash = NULL;
Evas_Hash *_edje_color_class_member_hash = NULL;

Evas_Hash *_edje_text_class_hash = NULL;
Evas_Hash *_edje_text_class_member_hash = NULL;

void
edje_freeze(void)
{
   Evas_List *l;
   
   for (l = _edje_edjes; l; l = l->next)
     edje_object_freeze((Evas_Object *)(l->data));
}

void
edje_thaw(void)
{
   Evas_List *l;
   
   for (l = _edje_edjes; l; l = l->next)
     edje_object_thaw((Evas_Object *)(l->data));
}

const char *
edje_object_data_get(Evas_Object *obj, const char *key)
{
   Edje *ed;
   Evas_List *l;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (!key))
     return NULL;
   if (!ed->collection) return NULL;
   for (l = ed->collection->data; l; l = l->next)
     {
	Edje_Data *di;
	
	di = l->data;
	if (!strcmp(di->key, key)) return (const char *)di->value;
     }
   return NULL;
}

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
edje_color_class_set(const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Evas_List *members;
   Edje_Color_Class *cc;

   if (!color_class) return;

   cc = evas_hash_find(_edje_color_class_hash, color_class);
   if (!cc)
     {
        cc = malloc(sizeof(Edje_Color_Class));
	if (!cc) return;
	cc->name = strdup(color_class);
	if (!cc->name)
	  {
	     free(cc);
	     return;
	  }
	_edje_color_class_hash = evas_hash_add(_edje_color_class_hash, color_class, cc);
	if (evas_hash_alloc_error())
	  {
	     free(cc->name);
	     free(cc);
	     return;
	  }

     }

   if (r < 0)   r = 0;
   if (r > 255) r = 255;
   if (g < 0)   g = 0;
   if (g > 255) g = 255;
   if (b < 0)   b = 0;
   if (b > 255) b = 255;
   if (a < 0)   a = 0;
   if (a > 255) a = 255;
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

   members = evas_hash_find(_edje_color_class_member_hash, color_class);
   while (members)
     {
	Edje *ed;

	ed = members->data;
	ed->dirty = 1;
	_edje_recalc(ed);
	members = members->next;
     }
}

void
edje_object_color_class_set(Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Edje *ed;
   Evas_List *l;
   Edje_Color_Class *cc;

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
   cc = malloc(sizeof(Edje_Color_Class));
   if (!cc) return;
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
edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size)
{
   Evas_List *members;
   Edje_Text_Class *tc;

   if (!text_class) return;

   if (size < 0) size = 0;
   if (!font) font = "";

   tc = evas_hash_find(_edje_text_class_hash, text_class);
   if (!tc)
     {
        tc = calloc(1, sizeof(Edje_Text_Class));
	if (!tc) return;
	tc->name = strdup(text_class);
	if (!tc->name)
	  {
	     free(tc);
	     return;
	  }
	_edje_text_class_hash = evas_hash_add(_edje_text_class_hash, text_class, tc);
	if (evas_hash_alloc_error())
	  {
	     free(tc->name);
	     free(tc);
	     return;
	  }

	tc->font = strdup(font);
	tc->size = size;
	return;
     }

   if ((tc->size == size) && (!strcmp(tc->font, font)))
     return;
   free(tc->font);
   tc->font = strdup(font);
   if (!tc->font)
     {
	_edje_text_class_hash = evas_hash_del(_edje_text_class_hash, text_class, tc);
	free(tc);
	return;
     }
   tc->size = size;

   members = evas_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
	Edje *ed;

	ed = members->data;
	ed->dirty = 1;
	_edje_recalc(ed);
	members = members->next;
     }
}

void
edje_object_text_class_set(Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size)
{
   Edje *ed;
   Evas_List *l;
   Edje_Text_Class *tc;

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
   tc = malloc(sizeof(Edje_Text_Class));
   if (!tc) return;
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
edje_object_part_geometry_get(Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h )
{
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
   char *type;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp) return;
   if (rp->swallowed_object)
     {
	evas_object_smart_member_del(rp->swallowed_object);
	evas_object_event_callback_del(rp->swallowed_object, 
				       EVAS_CALLBACK_FREE, 
				       _edje_object_part_swallow_free_cb);
	evas_object_clip_unset(rp->swallowed_object);
	rp->swallowed_object = NULL;
     }
   if (!obj_swallow) return;
   rp->swallowed_object = obj_swallow;
   evas_object_smart_member_add(rp->swallowed_object, ed->obj);
   if (rp->clip_to) evas_object_clip_set(rp->swallowed_object, rp->clip_to->object);
   else evas_object_clip_set(rp->swallowed_object, ed->clipper);
   if (evas_object_layer_get(rp->swallowed_object) != ed->layer)
     evas_object_layer_set(rp->swallowed_object, ed->layer);
   evas_object_stack_above(rp->swallowed_object, rp->object);
   evas_object_event_callback_add(rp->swallowed_object,
				  EVAS_CALLBACK_FREE, 
				  _edje_object_part_swallow_free_cb,
				  obj);
   type = (char *)evas_object_type_get(obj_swallow);
   rp->swallow_params.min.w = 0;
   rp->swallow_params.min.w = 0;
   rp->swallow_params.max.w = -1;
   rp->swallow_params.max.h = -1;
   if ((type) && (!strcmp(type, "edje")))
     {
	Evas_Coord w, h;
	
	edje_object_size_min_get(obj_swallow, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	edje_object_size_max_get(obj_swallow, &w, &h);
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
   else if ((type) && ((!strcmp(type, "text")) ||
		       (!strcmp(type, "polygon")) ||
		       (!strcmp(type, "line"))))
     {
	Evas_Coord w, h;
	
	evas_object_geometry_get(obj_swallow, NULL, NULL, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
     {
	int w1, h1, w2, h2;
	
	w1 = (int)evas_object_data_get(obj_swallow, "\377 edje.minw");
	h1 = (int)evas_object_data_get(obj_swallow, "\377 edje.minh");
	w2 = (int)evas_object_data_get(obj_swallow, "\377 edje.maxw");
	h2 = (int)evas_object_data_get(obj_swallow, "\377 edje.maxh");
	rp->swallow_params.min.w = w1;
	rp->swallow_params.min.h = h1;
	if (w2 > 0) rp->swallow_params.max.w = w2;
	if (h2 > 0) rp->swallow_params.max.h = h2;
     }
   ed->dirty = 1;
   _edje_recalc(ed);   
}

void
edje_extern_object_min_size_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   int mw, mh;
   
   mw = minw;
   mh = minh;
   if (mw < 0) mw = 0;
   if (mh < 0) mh = 0;
   if (mw > 0)
     evas_object_data_set(obj, "\377 edje.minw", (void *)mw);
   else
     evas_object_data_del(obj, "\377 edje.minw");
   if (mh > 0)
     evas_object_data_set(obj, "\377 edje.minh", (void *)mh);
   else
     evas_object_data_del(obj, "\377 edje.minh");
}

void
edje_extern_object_max_size_set(Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh)
{
   int mw, mh;
   
   mw = maxw;
   mh = maxh;
   if (mw >= 0)
     evas_object_data_set(obj, "\377 edje.maxw", (void *)mw);
   else
     evas_object_data_del(obj, "\377 edje.maxw"); 
   if (mh >= 0)
     evas_object_data_set(obj, "\377 edje.maxh", (void *)mh);
   else
     evas_object_data_del(obj, "\377 edje.maxh");
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
	     evas_object_smart_member_del(rp->swallowed_object);
	     evas_object_clip_unset(rp->swallowed_object);
	     rp->swallowed_object = NULL;
	     rp->swallow_params.min.w = 0;
	     rp->swallow_params.min.h = 0;
	     rp->swallow_params.max.w = 0;
	     rp->swallow_params.max.h = 0;
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
edje_object_size_min_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (!ed->collection))
     {
	if (minw) *minw = 0;
	if (minh) *minh = 0;
	return;
     }
   if (minw) *minw = ed->collection->prop.min.w;
   if (minh) *minh = ed->collection->prop.min.h;
}

void
edje_object_size_max_get(Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (!ed->collection))
     {
	if (maxw) *maxw = 0;
	if (maxh) *maxh = 0;
	return;
     }
   if (ed->collection->prop.max.w == 0)
     {
	if (maxw) *maxw = 1e+37;
     }
   else
     {
	if (maxw) *maxw = ed->collection->prop.max.w;
     }
   if (ed->collection->prop.max.h == 0)
     {
	if (maxh) *maxh = 1e+37;
     }
   else
     {
	if (maxh) *maxh = ed->collection->prop.max.h;
     }
}

void
edje_object_size_min_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   Edje *ed;
   Evas_Coord pw, ph;   
   int maxw, maxh;
   int ok;
   
   ed = _edje_fetch(obj);
   if ((!ed) || (!ed->collection))
     {
	if (minw) *minw = 0;
	if (minh) *minh = 0;
	return;
     }
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

const char *
edje_object_part_state_get(Evas_Object *obj, const char *part, double *val_ret)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	if (val_ret) *val_ret = 0;
	return "";
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	if (val_ret) *val_ret = 0;
	return "";
     }
   if (!rp->chosen_description)
     {
	if (val_ret) *val_ret = rp->chosen_description->state.value;
	if (rp->chosen_description->state.name)
	  return rp->chosen_description->state.name;
	return "default";
     }
   else
     {
	if (rp->param1.description)
	  {
	     if (val_ret) *val_ret = rp->param1.description->state.value;
	     if (rp->param1.description->state.name)
	       return rp->param1.description->state.name;
	     return "default";
	  }
     }
   if (val_ret) *val_ret = 0;
   return "";
}

int
edje_object_part_drag_dir_get(Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;
   
   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	return EDJE_DRAG_DIR_NONE;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	return EDJE_DRAG_DIR_NONE;
     }
   if ((rp->part->dragable.x) && (rp->part->dragable.y)) return EDJE_DRAG_DIR_XY;
   else if (rp->part->dragable.x) return EDJE_DRAG_DIR_X;
   else if (rp->part->dragable.y) return EDJE_DRAG_DIR_Y;
   return EDJE_DRAG_DIR_NONE;
}

void
edje_object_part_drag_value_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	return;
     }
   if (rp->drag.down.count > 0) return;
   if(rp->part->dragable.confine_id != -1)
   {
    if (dx < 0.0) dx = 0.0;
    else if (dx > 1.0) dx = 1.0;
    if (dy < 0.0) dy = 0.0;
    else if (dy > 1.0) dy = 1.0;
   }
   if (rp->part->dragable.x < 0) dx = 1.0 - dx;
   if (rp->part->dragable.y < 0) dy = 1.0 - dy;
   if ((rp->drag.val.x == dx) && (rp->drag.val.y == dy)) return;
   rp->drag.val.x = dx;
   rp->drag.val.y = dy;
   _edje_dragable_pos_set(ed, rp, dx, dy);
   _edje_emit(ed, "drag,set", rp->part->name);   
}

void
edje_object_part_drag_value_get(Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   double ddx, ddy;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   ddx = rp->drag.val.x;
   ddy = rp->drag.val.y;
   if (rp->part->dragable.x < 0) ddx = 1.0 - ddx;
   if (rp->part->dragable.y < 0) ddy = 1.0 - ddy;
   if (dx) *dx = ddx;
   if (dy) *dy = ddy;
}

void
edje_object_part_drag_size_set(Evas_Object *obj, const char *part, double dw, double dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	return;
     }
   if (dw < 0.0) dw = 0.0;
   else if (dw > 1.0) dw = 1.0;
   if (dh < 0.0) dh = 0.0;
   else if (dh > 1.0) dh = 1.0;
   if ((rp->drag.size.x == dw) && (rp->drag.size.y == dh)) return;
   rp->drag.size.x = dw;
   rp->drag.size.y = dh;
   ed->dirty = 1;
   _edje_recalc(ed);
}

void
edje_object_part_drag_size_get(Evas_Object *obj, const char *part, double *dw, double *dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return;
     }   
   if (dw) *dw = rp->drag.size.x;
   if (dh) *dh = rp->drag.size.y;
}

void
edje_object_part_drag_step_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	return;
     }
   if (rp->drag.down.count > 0) return;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag.step.x = dx;
   rp->drag.step.y = dy;
}

void
edje_object_part_drag_step_get(Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   if (dx) *dx = rp->drag.step.x;
   if (dy) *dy = rp->drag.step.y;
}

void
edje_object_part_drag_page_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	return;
     }
   if (rp->drag.down.count > 0) return;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag.page.x = dx;
   rp->drag.page.y = dy;
}

void
edje_object_part_drag_page_get(Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   if (dx) *dx = rp->drag.page.x;
   if (dy) *dy = rp->drag.page.y;
}

void
edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   double px, py;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	return;
     }
   if (rp->drag.down.count > 0) return;
   px = rp->drag.val.x;
   py = rp->drag.val.y;
   rp->drag.val.x += dx * rp->drag.step.x * rp->part->dragable.x;
   rp->drag.val.y += dy * rp->drag.step.y * rp->part->dragable.y;
   if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
   else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
   if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
   else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
   if ((px == rp->drag.val.x) && (py == rp->drag.val.y)) return;
   _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
   _edje_emit(ed, "drag,step", rp->part->name);   
}

void
edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   double px, py;

   ed = _edje_fetch(obj);   
   if ((!ed) || (!part))
     {
	return;
     }
   rp = _edje_real_part_get(ed, (char *)part);
   if (!rp)
     {
	return;
     }
   if (rp->drag.down.count > 0) return;
   px = rp->drag.val.x;
   py = rp->drag.val.y;
   rp->drag.val.x += dx * rp->drag.page.x * rp->part->dragable.x;
   rp->drag.val.y += dy * rp->drag.page.y * rp->part->dragable.y;
   if      (rp->drag.val.x < 0.0) rp->drag.val.x = 0.0;
   else if (rp->drag.val.x > 1.0) rp->drag.val.x = 1.0;
   if      (rp->drag.val.y < 0.0) rp->drag.val.y = 0.0;
   else if (rp->drag.val.y > 1.0) rp->drag.val.y = 1.0;
   if ((px == rp->drag.val.x) && (py == rp->drag.val.y)) return;
   _edje_dragable_pos_set(ed, rp, rp->drag.val.x, rp->drag.val.y);
   _edje_emit(ed, "drag,page", rp->part->name);   
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

Edje_Color_Class *
_edje_color_class_find(Edje *ed, char *color_class)
{
   Evas_List *l;
   
   if ((!ed) || (!color_class)) return NULL;
   for (l = ed->color_classes; l; l = l->next)
     {
	Edje_Color_Class *cc;
	
	cc = l->data;
	if (!strcmp(color_class, cc->name)) return cc;
     }
   return evas_hash_find(_edje_color_class_hash, color_class);
}

void
_edje_color_class_member_add(Edje *ed, char *color_class)
{
   Evas_List *members;

   if ((!ed) || (!color_class)) return;
   members = evas_hash_find(_edje_color_class_member_hash, color_class);
   if (members) _edje_color_class_member_hash = evas_hash_del(_edje_color_class_member_hash, color_class, members);

   members = evas_list_prepend(members, ed);
   _edje_color_class_member_hash = evas_hash_add(_edje_color_class_member_hash, color_class, members);
}

void
_edje_color_class_member_del(Edje *ed, char *color_class)
{
   Evas_List *members;

   if ((!ed) || (!color_class)) return;
   members = evas_hash_find(_edje_color_class_member_hash, color_class);
   if (!members) return;

   _edje_color_class_member_hash = evas_hash_del(_edje_color_class_member_hash, color_class, members);
   members = evas_list_remove(members, ed);
   if (members) _edje_color_class_member_hash = evas_hash_add(_edje_color_class_member_hash, color_class, members);
}

/**
 * Used to free the member lists that are stored in the text_class
 * and color_class hashtables.
 */
static Evas_Bool member_list_free(Evas_Hash *hash, const char *key,
                                  void *data, void *fdata)
{
	evas_list_free(data);

	return 1;
}

void
_edje_color_class_members_free(void)
{
   if (!_edje_color_class_member_hash) return;

   evas_hash_foreach(_edje_color_class_member_hash, member_list_free,
                     NULL);
   evas_hash_free(_edje_color_class_member_hash);
   _edje_color_class_member_hash = NULL;
}

void
_edje_color_class_on_del(Edje *ed, Edje_Part *ep)
{
   Evas_List *tmp;

   if ((ep->default_desc) && (ep->default_desc->color_class)) _edje_color_class_member_del(ed, ep->default_desc->color_class);
   for (tmp = ep->other_desc; tmp; tmp = tmp->next)
     {
        Edje_Part_Description *desc;

	desc = tmp->data;
	if (desc->color_class)
	  {
	     _edje_color_class_member_del(ed, desc->color_class);
	     desc->color_class = NULL;
	  }
     }
}

Edje_Text_Class *
_edje_text_class_find(Edje *ed, char *text_class)
{
   Evas_List *l;
   
   if ((!ed) || (!text_class)) return NULL;
   for (l = ed->text_classes; l; l = l->next)
     {
	Edje_Text_Class *tc;
	
	tc = l->data;
	if (!strcmp(text_class, tc->name)) return tc;
     }
   return evas_hash_find(_edje_text_class_hash, text_class);
}

void
_edje_text_class_member_add(Edje *ed, char *text_class)
{
   Evas_List *members;

   if ((!ed) || (!text_class)) return;
   members = evas_hash_find(_edje_text_class_member_hash, text_class);
   if (members) _edje_text_class_member_hash = evas_hash_del(_edje_text_class_member_hash, text_class, members);

   members = evas_list_prepend(members, ed);
   _edje_text_class_member_hash = evas_hash_add(_edje_text_class_member_hash, text_class, members);
}

void
_edje_text_class_member_del(Edje *ed, char *text_class)
{
   Evas_List *members;

   if ((!ed) || (!text_class)) return;
   members = evas_hash_find(_edje_text_class_member_hash, text_class);
   if (!members) return;

   _edje_text_class_member_hash = evas_hash_del(_edje_text_class_member_hash, text_class, members);
   members = evas_list_remove(members, ed);
   if (members) _edje_text_class_member_hash = evas_hash_add(_edje_text_class_member_hash, text_class, members);
}

void
_edje_text_class_members_free(void)
{
   if (!_edje_text_class_member_hash) return;

   evas_hash_foreach(_edje_text_class_member_hash, member_list_free,
                     NULL);
   evas_hash_free(_edje_text_class_member_hash);
   _edje_text_class_member_hash = NULL;
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
   if (!strcmp(glob, "*")) return 1;
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
   if (ed->freeze < 0) ed->freeze = 0;
   if ((ed->freeze == 0) && (ed->recalc)) _edje_recalc(ed);
   return ed->freeze;
}

int
_edje_block(Edje *ed)
{
   ed->block++;
   return ed->block;
}

int
_edje_unblock(Edje *ed)
{
   ed->block--;
   if (ed->block == 0)
     {
	ed->block_break = 0;
	ed->dont_clear_signals = 0;
	if (ed->emissions) _edje_emit(ed, "", NULL);
     }
   return ed->block;
}

int
_edje_block_break(Edje *ed)
{
   if (ed->block_break) return 1;
   return 0;
}

void
_edje_block_violate(Edje *ed)
{
   if (ed->block > 0) ed->block_break = 1;
}

void
_edje_object_part_swallow_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Object *edje_obj;
   
   edje_obj = data;
   edje_object_part_unswallow(edje_obj, obj);
   return;
   e = NULL;
   event_info = NULL;
}
