#include "Edje.h"
#include "edje_private.h"

static void _edje_smart_add(Evas_Object * obj);
static void _edje_smart_del(Evas_Object * obj);
static void _edje_smart_layer_set(Evas_Object * obj, int layer);
static void _edje_smart_raise(Evas_Object * obj);
static void _edje_smart_lower(Evas_Object * obj);
static void _edje_smart_stack_above(Evas_Object * obj, Evas_Object * above);
static void _edje_smart_stack_below(Evas_Object * obj, Evas_Object * below);
static void _edje_smart_move(Evas_Object * obj, double x, double y);
static void _edje_smart_resize(Evas_Object * obj, double w, double h);
static void _edje_smart_show(Evas_Object * obj);
static void _edje_smart_hide(Evas_Object * obj);
static void _edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _edje_smart_clip_unset(Evas_Object * obj);

static Evas_Smart *_edje_smart = NULL;

/* API Routines */
Evas_Object *
edje_add(Evas *evas)
{
   if (!_edje_smart)
     {
	edje_init();
	_edje_smart = evas_smart_new("edje",
				     _edje_smart_add,
				     _edje_smart_del,
				     _edje_smart_layer_set,
				     _edje_smart_raise,
				     _edje_smart_lower,
				     _edje_smart_stack_above,
				     _edje_smart_stack_below,
				     _edje_smart_move,
				     _edje_smart_resize,
				     _edje_smart_show,
				     _edje_smart_hide,
				     _edje_smart_color_set,
				     _edje_smart_clip_set, 
				     _edje_smart_clip_unset, 
				     NULL);
     }
   return evas_object_smart_add(evas, _edje_smart);
}

/* Private Routines */
static void
_edje_smart_add(Evas_Object * obj)
{
   Edje *ed;

   ed = _edje_add(obj);
   if (!ed) return;
   evas_object_smart_data_set(obj, ed);
   ed->obj = obj;
   evas_object_smart_member_add(ed->clipper, ed->obj);
}

static void
_edje_smart_del(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   _edje_del(ed);
}

static void
_edje_smart_layer_set(Evas_Object * obj, int layer)
{
   Edje *ed;
   Evas_List *l;
   char buf[256];
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (ed->layer == layer) return;
   ed->layer = layer;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_layer_set(ep->object, ed->layer);
     }
   snprintf(buf, sizeof(buf), "layer,set,%i", layer);
   _edje_emit(ed, buf, "");
}

static void
_edje_smart_raise(Evas_Object * obj)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_raise(ep->object);
     }
   _edje_emit(ed, "raise", "");
}

static void
_edje_smart_lower(Evas_Object * obj)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = evas_list_last(ed->parts); l; l = l->prev)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_lower(ep->object);
     }
   _edje_emit(ed, "lower", "");
}

static void 
_edje_smart_stack_above(Evas_Object * obj, Evas_Object * above)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = evas_list_last(ed->parts); l; l = l->prev)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_stack_above(ep->object, above);
     }
   _edje_emit(ed, "stack_above", "");
}

static void
_edje_smart_stack_below(Evas_Object * obj, Evas_Object * below)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_stack_below(ep->object, below);
     }
   _edje_emit(ed, "stack_below", "");
}

static void 
_edje_smart_move(Evas_Object * obj, double x, double y)
{
   Edje *ed;
   Evas_List *l;
   double xx, yy;
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((ed->x == x) && (ed->y == y)) return;
   ed->x = x;
   ed->y = y;
   evas_object_move(ed->clipper, ed->x, ed->y);
   
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_move(ep->object, ed->x + ep->x, ed->y + ep->y);
     }
   _edje_emit(ed, "move", "");
}

static void 
_edje_smart_resize(Evas_Object * obj, double w, double h)
{
   Edje *ed;
   int nw, nh;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   nw = ed->w;
   nh = ed->h;
   ed->w = w;
   ed->h = h;
   if ((nw == ed->w) && (nh == ed->h)) return;
   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = 1;
   _edje_recalc(ed);
   _edje_emit(ed, "resize", "");
}

static void 
_edje_smart_show(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((ed->collection) && (ed->parts))
     evas_object_show(ed->clipper);
   _edje_emit(ed, "show", "");
}

static void 
_edje_smart_hide(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_hide(ed->clipper);
   _edje_emit(ed, "hide", "");
}

static void 
_edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_color_set(ed->clipper, r, g, b, a);
   _edje_emit(ed, "color_set", "");
}

static void 
_edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (evas_object_clip_get(obj) == clip) return;
   evas_object_clip_set(ed->clipper, clip);
   _edje_emit(ed, "clip_set", "");
}

static void 
_edje_smart_clip_unset(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (!evas_object_clip_get(obj)) return;
   evas_object_clip_unset(ed->clipper);
   _edje_emit(ed, "clip_unset", "");
}
