#include "Edje.h"
#include "edje_private.h"

Edje *_edje_fetch(Evas_Object *obj);

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

Evas_Object *
edje_add(Evas *evas)
{
   if (!_edje_smart)
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
   return evas_object_smart_add(evas, _edje_smart);
}

void
edje_file_set(Evas_Object *obj, const char *file, const char *part)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return;
}

/*** internal calls ***/

/* manipulation calls */

static void
_edje_part_description_apply(Edje *ed, 
			     Edje_Real_Part *ep, 
			     Edje_Part_Description *de1, 
			     Edje_Part_Description *de2, 
			     double pos)
{
   if ((ep->param1.description == de1) && 
       (ep->param2.description == de2) && 
       (ep->description_pos == pos))
     return;
   
   ep->param1.description = de1;
   ep->param2.description = de2;
   ep->description_pos = pos;
   
   ed->dirty = 1;
   ep->dirty = 1;
}

/* calculation functions */

static void
_edje_part_recalc_single(Edje *ed,
			 Edje_Real_Part *ep, 
			 Edje_Part_Description *desc, 
			 Edje_Real_Part *rel1_to, 
			 Edje_Real_Part *rel2_to, 
			 Edje_Real_Part *confine_to,
			 Edje_Calc_Params *params)
{
   /* horiz */
   if (rel1_to)
     {
	params->x = desc->rel1.offset_x +
	  rel1_to->x + (desc->rel1.relative_x * rel1_to->w);
     }
   else
     {
	params->x = desc->rel1.offset_x +
	  (desc->rel1.relative_x * ed->w);
     }
   if (rel2_to)
     {
	params->w = desc->rel2.offset_x +
	  rel2_to->x + (desc->rel2.relative_x * rel2_to->w) -
	  params->x;
     }
   else
     {
	params->w = desc->rel2.offset_x +
	  (desc->rel2.relative_x * ed->w) -
	  params->x;
     }
   
   /* vert */
}

static void
_edje_part_recalc(Edje *ed, Edje_Real_Part *ep)
{
   Edje_Calc_Params p1, p2;
   
   if (ep->calculated) return;
   if (ep->param1.rel1_to)    _edje_part_recalc(ed, ep->param1.rel1_to);
   if (ep->param1.rel2_to)    _edje_part_recalc(ed, ep->param1.rel2_to);
   if (ep->param1.confine_to) _edje_part_recalc(ed, ep->param1.confine_to);
   if (ep->param2.rel1_to)    _edje_part_recalc(ed, ep->param2.rel1_to);
   if (ep->param2.rel2_to)    _edje_part_recalc(ed, ep->param2.rel2_to);
   if (ep->param2.confine_to) _edje_part_recalc(ed, ep->param2.confine_to);
   
   /* actually calculate now */
   if (ep->param1.description)
     _edje_part_recalc_single(ed, ep, ep->param1.description, ep->param1.rel1_to, ep->param1.rel2_to, ep->param1.confine_to, &p1);
   if (ep->param2.description)
     _edje_part_recalc_single(ed, ep, ep->param1.description, ep->param2.rel1_to, ep->param2.rel2_to, ep->param2.confine_to, &p2);
   
   ep->calculated = 1;
   ep->dirty = 0;
}

static void
_edje_recalc(Edje *ed)
{
   Evas_List *l;
   
   if (!ed->dirty) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	ep->calculated = 0;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	if (!ep->calculated) _edje_part_recalc(ed, ep);
     }
   ed->dirty = 0;
}

/* utility functions we will use a lot */

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

/* evas smart object methods - required by evas smart objects to do the */
/* dirty work on smrt objects */

static void
_edje_smart_add(Evas_Object * obj)
{
   Edje *ed;
   
   ed = calloc(1, sizeof(Edje));
   if (!ed) return;
   evas_object_smart_data_set(obj, ed);
   ed->evas = evas_object_evas_get(obj);
   ed->clipper = evas_object_rectangle_add(ed->evas);
   evas_object_smart_member_add(ed->clipper, obj);
   evas_object_color_set(ed->clipper, 255, 255, 255, 255);
   evas_object_move(ed->clipper, 0, 0);
   evas_object_resize(ed->clipper, 0, 0);
}

static void
_edje_smart_del(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_del(ed->clipper);
   free(ed);
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
	evas_object_layer_set(ep->object, ed->layer);
     }
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
}

static void 
_edje_smart_move(Evas_Object * obj, double x, double y)
{
   Edje *ed;
   Evas_List *l;
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   ed->x = x;
   ed->y = y;

   evas_object_move(ed->clipper, ed->x, ed->y);
   
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_move(ep->object, ed->x + ep->x, ed->y + ep->y);
     }
}

static void 
_edje_smart_resize(Evas_Object * obj, double w, double h)
{
   Edje *ed;
   int nw, nh;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   nw = w;
   nh = h;
   ed->w = w;
   ed->h = h;
   if ((nw == ed->w) || (nh == ed->h)) return;
   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = 1;
   _edje_recalc(ed);
}

static void 
_edje_smart_show(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_show(ed->clipper);
}

static void 
_edje_smart_hide(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_hide(ed->clipper);
}

static void 
_edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_color_set(ed->clipper, r, g, b, a);
}

static void 
_edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_clip_set(ed->clipper, clip);
}

static void 
_edje_smart_clip_unset(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_clip_unset(ed->clipper);
}
