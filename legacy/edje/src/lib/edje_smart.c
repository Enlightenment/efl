#include "Edje.h"
#include "edje_private.h"

static void _edje_smart_add(Evas_Object * obj);
static void _edje_smart_del(Evas_Object * obj);
static void _edje_smart_layer_set(Evas_Object * obj, int layer);
static void _edje_smart_raise(Evas_Object * obj);
static void _edje_smart_lower(Evas_Object * obj);
static void _edje_smart_stack_above(Evas_Object * obj, Evas_Object * above);
static void _edje_smart_stack_below(Evas_Object * obj, Evas_Object * below);
static Evas_Object *_edje_smart_above_get(Evas_Object * obj);
static Evas_Object *_edje_smart_below_get(Evas_Object * obj);
static void _edje_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _edje_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _edje_smart_show(Evas_Object * obj);
static void _edje_smart_hide(Evas_Object * obj);
static void _edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _edje_smart_clip_unset(Evas_Object * obj);

static Evas_Smart *_edje_smart = NULL;

Evas_List *_edje_edjes = NULL;

/************************** API Routines **************************/

/* FIXDOC: Verify/Expand */
/** Constructs the Edje object
 * @param evas A valid Evas handle
 * @return The Evas_Object pointer.
 *
 * Creates the Edje smart object, returning the Evas_Object handle.
 */
Evas_Object *
edje_object_add(Evas *evas)
{
   if (!_edje_smart)
     {
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
   evas_smart_above_get_set(_edje_smart, _edje_smart_above_get);
   evas_smart_below_get_set(_edje_smart, _edje_smart_below_get);
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
   _edje_edjes = evas_list_append(_edje_edjes, obj);
}

static void
_edje_smart_del(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   _edje_block_violate(ed);
   ed->delete_me = 1;
   _edje_clean_objects(ed);
   _edje_unref(ed);
   _edje_edjes = evas_list_remove(_edje_edjes, obj);
   /* ... be paranoid */
   evas_object_smart_data_set(obj, NULL);
}

static void
_edje_smart_layer_set(Evas_Object * obj, int layer)
{
   Edje *ed;
   Evas_List *l;
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (ed->layer == layer) return;
   ed->layer = layer;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	if (ep->extra_objects)
	  {
	     Evas_List *el;
	     
	     for (el = ep->extra_objects; el; el = el->next)
	       {
		  Evas_Object *o;
		  
		  o = el->data;
		  evas_object_layer_set(o, ed->layer);
	       }
	  }
	evas_object_layer_set(ep->object, ed->layer);
	if (ep->swallowed_object)
	  evas_object_layer_set(ep->swallowed_object, ed->layer);
     }
   _edje_emit(ed, "layer,set", "");
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
	if (ep->extra_objects)
	  {
	     Evas_List *el;
	     
	     for (el = ep->extra_objects; el; el = el->next)
	       {
		  Evas_Object *o;
		  
		  o = el->data;
		  evas_object_raise(o);
	       }
	  }
	evas_object_raise(ep->object);
	if (ep->swallowed_object)
	  evas_object_raise(ep->swallowed_object);	
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
	if (ep->swallowed_object)
	  evas_object_lower(ep->swallowed_object);
	evas_object_lower(ep->object);
	if (ep->extra_objects)
	  {
	     Evas_List *el;
	     
	     for (el = ep->extra_objects; el; el = el->next)
	       {
		  Evas_Object *o;
		  
		  o = el->data;
		  evas_object_lower(o);
	       }
	  }
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
	if (ep->swallowed_object)
	  evas_object_stack_above(ep->swallowed_object, above);
	evas_object_stack_above(ep->object, above);
	if (ep->extra_objects)
	  {
	     Evas_List *el;
	     
	     for (el = evas_list_last(ep->extra_objects); el; el = el->prev)
	       {
		  Evas_Object *o;
		  
		  o = el->data;
		  evas_object_stack_above(o, above);
	       }
	  }
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
	if (ep->extra_objects)
	  {
	     Evas_List *el;
	     
	     for (el = ep->extra_objects; el; el = el->next)
	       {
		  Evas_Object *o;
		  
		  o = el->data;
		  evas_object_stack_below(o, below);
	       }
	  }
	evas_object_stack_below(ep->object, below);
	if (ep->swallowed_object)
	  evas_object_stack_below(ep->swallowed_object, below);
     }
   _edje_emit(ed, "stack_below", "");
}

static Evas_Object *
_edje_smart_above_get(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return obj;
   if (ed->parts)
   {
      Edje_Real_Part *ep;

      ep = evas_list_last(ed->parts)->data;
      if (ep->swallowed_object)
         return ep->swallowed_object;
      return ep->object;
   }
   return obj;
}

static Evas_Object *
_edje_smart_below_get(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return obj;
   if (ed->parts)
   {
      Edje_Real_Part *ep;

      ep = ed->parts->data;
      return ep->object;
   }
   return obj;
}

static void 
_edje_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
   Edje *ed;
   Evas_List *l;
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((ed->x == x) && (ed->y == y)) return;
   ed->x = x;
   ed->y = y;
//   evas_object_move(ed->clipper, ed->x, ed->y);
   
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	Evas_Coord ox, oy;
	
	ep = l->data;
	evas_object_geometry_get(ep->object, &ox, &oy, NULL, NULL);
	evas_object_move(ep->object, ed->x + ep->x + ep->offset.x, ed->y + ep->y +ep->offset.y);
	if (ep->swallowed_object)
	  {
	     evas_object_geometry_get(ep->swallowed_object, &ox, &oy, NULL, NULL);
	     evas_object_move(ep->swallowed_object, ed->x + ep->x + ep->offset.x, ed->y + ep->y +ep->offset.y);
	  }
	if (ep->extra_objects)
	  {
	     Evas_List *el;
	     
	     for (el = ep->extra_objects; el; el = el->next)
	       {
		  Evas_Object *o;
		  Evas_Coord oox, ooy;
		  
		  o = el->data;
		  evas_object_geometry_get(o, &oox, &ooy, NULL, NULL);
		  evas_object_move(o, ed->x + ep->x + ep->offset.x + (oox - ox), ed->y + ep->y + ep->offset.y + (ooy - oy));
	       }
	  }
     }
   _edje_emit(ed, "move", "");
}

static void 
_edje_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if ((w == ed->w) && (h == ed->h)) return;
   ed->w = w;
   ed->h = h;
//   evas_object_resize(ed->clipper, ed->w, ed->h);
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
   if (evas_object_visible_get(obj)) return;
   if ((ed->collection) && (evas_object_clipees_get(ed->clipper)))
     evas_object_show(ed->clipper);
   _edje_emit(ed, "show", "");
}

static void 
_edje_smart_hide(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (!evas_object_visible_get(obj)) return;
   if ((ed->collection) && (evas_object_clipees_get(ed->clipper)))
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
