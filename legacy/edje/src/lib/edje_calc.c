#include "Edje.h"
#include "edje_private.h"

static void _edje_part_recalc_single(Edje *ed, Edje_Real_Part *ep, Edje_Part_Description *desc, Edje_Part_Description *chosen_desc, Edje_Real_Part *rel1_to_x, Edje_Real_Part *rel1_to_y, Edje_Real_Part *rel2_to_x, Edje_Real_Part *rel2_to_y, Edje_Real_Part *confine_to, Edje_Calc_Params *params);
static void _edje_part_recalc(Edje *ed, Edje_Real_Part *ep);

void
_edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, double pos)
{
   double npos;
   
   if (pos > 1.0) pos = 1.0;
   else if (pos < 0.0) pos = 0.0;
   npos = 0.0;
   /* take linear pos along timescale and use interpolation method */
   switch (mode)
     {
      case EDJE_TWEEN_MODE_SINUSOIDAL:
	npos = (1.0 - cos(pos * PI)) / 2.0;
	break;
      case EDJE_TWEEN_MODE_ACCELERATE:
	npos = 1.0 - sin((PI / 2.0) + (pos * PI / 2.0));
	break;
      case EDJE_TWEEN_MODE_DECELERATE:
	npos = sin(pos * PI / 2.0);
	break;
      case EDJE_TWEEN_MODE_LINEAR:
	npos = pos;
	break;
      default:
	break;
     }
   if (npos == ep->description_pos) return;

   ep->description_pos = npos;
   
   ed->dirty = 1;
}

void
_edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, char  *d1, double v1, char *d2, double v2)
{
   if (!d1) d1 = "default";
   if (!d2) d2 = "default";

   if (!strcmp(d1, "default") && (v1 == 0.0))
     ep->param1.description = ep->part->default_desc;
   else
     {
	Evas_List *l;
	double min_dst;
	Edje_Part_Description *desc_found;

	desc_found = NULL;
	min_dst = 999.0;
	if (!strcmp("default", d1))
	  {
	     desc_found = ep->part->default_desc;
	     min_dst = ep->part->default_desc->state.value - v1;
	     if (min_dst < 0) min_dst = -min_dst;
	  }
	for (l = ep->part->other_desc; l; l = l->next)
	  {
	     Edje_Part_Description *desc;
	     
	     desc = l->data;
	     if (!strcmp(desc->state.name, d1))
	       {
		  double dst;
		  
		  dst = desc->state.value - v1;
		  if (dst == 0.0)
		    {
		       desc_found = desc;
		       break;
		    }
		  if (dst < 0.0) dst = -dst;
		  if (dst < min_dst)
		    {
		       desc_found = desc;
		       min_dst = dst;
		    }
	       }
	  }
	ep->param1.description = desc_found;
     }
   ep->param1.rel1_to_x = NULL;
   ep->param1.rel1_to_y = NULL;
   ep->param1.rel2_to_x = NULL;
   ep->param1.rel2_to_y = NULL;
   if (ep->param1.description)
     {
	if (ep->param1.description->rel1.id_x >= 0)
	  ep->param1.rel1_to_x = evas_list_nth(ed->parts, ep->param1.description->rel1.id_x);
	if (ep->param1.description->rel1.id_y >= 0)
	  ep->param1.rel1_to_y = evas_list_nth(ed->parts, ep->param1.description->rel1.id_y);
	if (ep->param1.description->rel2.id_x >= 0)
	  ep->param1.rel2_to_x = evas_list_nth(ed->parts, ep->param1.description->rel2.id_x);
	if (ep->param1.description->rel2.id_y >= 0)
	  ep->param1.rel2_to_y = evas_list_nth(ed->parts, ep->param1.description->rel2.id_y);
     }
   
   if (!strcmp(d2, "default") && (v2 == 0.0))
     ep->param2.description = ep->part->default_desc;
   else
     {
	Evas_List *l;
	double min_dst;
	Edje_Part_Description *desc_found;
	
	desc_found = NULL;
	min_dst = 999.0;
	if (!strcmp("default", d2))
	  {
	     desc_found = ep->part->default_desc;
	     min_dst = ep->part->default_desc->state.value - v2;
	     if (min_dst < 0) min_dst = -min_dst;
	  }
	for (l = ep->part->other_desc; l; l = l->next)
	  {
	     Edje_Part_Description *desc;
	     
	     desc = l->data;
	     if (!strcmp(desc->state.name, d2))
	       {
		  double dst;
		  
		  dst = desc->state.value - v2;
		  if (dst == 0.0)
		    {
		       desc_found = desc;
		       break;
		    }
		  if (dst < 0.0) dst = -dst;
		  if (dst < min_dst)
		    {
		       desc_found = desc;
		       min_dst = dst;
		    }
	       }
	  }
	ep->param2.description = desc_found;
     }
   if (!ep->param1.description)
     ep->param1.description = ep->part->default_desc;
   ep->param1.rel1_to_x = NULL;
   ep->param1.rel1_to_y = NULL;
   ep->param1.rel2_to_x = NULL;
   ep->param1.rel2_to_y = NULL;
   if (ep->param1.description)
     {
	if (ep->param1.description->rel1.id_x >= 0)
	  ep->param1.rel1_to_x = evas_list_nth(ed->parts, ep->param1.description->rel1.id_x);
	if (ep->param1.description->rel1.id_y >= 0)
	  ep->param1.rel1_to_y = evas_list_nth(ed->parts, ep->param1.description->rel1.id_y);
	if (ep->param1.description->rel2.id_x >= 0)
	  ep->param1.rel2_to_x = evas_list_nth(ed->parts, ep->param1.description->rel2.id_x);
	if (ep->param1.description->rel2.id_y >= 0)
	  ep->param1.rel2_to_y = evas_list_nth(ed->parts, ep->param1.description->rel2.id_y);
     }
   ep->param2.rel1_to_x = NULL;
   ep->param2.rel1_to_y = NULL;
   ep->param2.rel2_to_x = NULL;
   ep->param2.rel2_to_y = NULL;
   if (ep->param2.description)
     {
	if (ep->param2.description->rel1.id_x >= 0)
	  ep->param2.rel1_to_x = evas_list_nth(ed->parts, ep->param2.description->rel1.id_x);
	if (ep->param2.description->rel1.id_y >= 0)
	  ep->param2.rel1_to_y = evas_list_nth(ed->parts, ep->param2.description->rel1.id_y);
	if (ep->param2.description->rel2.id_x >= 0)
	  ep->param2.rel2_to_x = evas_list_nth(ed->parts, ep->param2.description->rel2.id_x);
	if (ep->param2.description->rel2.id_y >= 0)
	  ep->param2.rel2_to_y = evas_list_nth(ed->parts, ep->param2.description->rel2.id_y);
     }
   
   ed->dirty = 1;
}

void
_edje_recalc(Edje *ed)
{
   Evas_List *l;
   
   if (!ed->dirty) return;
   if (ed->freeze)
     {
	ed->recalc = 1;
	if (!ed->calc_only) return;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	ep->calculated = 0;
	ep->calculating = 0;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	if (!ep->calculated) _edje_part_recalc(ed, ep);
     }
   ed->dirty = 0;
   if (!ed->calc_only) ed->recalc = 0;
}

int
_edje_part_dragable_calc(Edje *ed, Edje_Real_Part *ep, double *x, double *y)
{
   if ((ep->part->dragable.x != 0) || (ep->part->dragable.y != 0))
     {
	if (ep->confine_to)
	  {
	     double dx, dy, dw, dh;
	     int ret;
	     
	     ret = 0;
	     if ((ep->part->dragable.x != 0) && 
		 (ep->part->dragable.y != 0 )) ret = 3;
	     else if (ep->part->dragable.x != 0) ret = 1;
	     else if (ep->part->dragable.y != 0) ret = 2;
	     
	     dx = ep->x - ep->confine_to->x;
	     dw = ep->confine_to->w - ep->w;
	     if (dw < 1) dw = 1;
	     dx /= dw;
	     
	     dy = ep->y - ep->confine_to->y;
	     dh = ep->confine_to->h - ep->h;
	     if (dh < 1) dh = 1;
	     dy /= dh;
	     
	     if (x) *x = dx;
	     if (y) *y = dy;
	     
	     return ret;
	  }
	else
	  {
	     if (x) *x = (double)(ep->drag.tmp.x + ep->drag.x);
	     if (y) *y = (double)(ep->drag.tmp.y + ep->drag.y);
	     return 0;
	  }
     }
   if (x) *x = 0.0;
   if (y) *y = 0.0;
   return 0;
   ed = NULL;
}

void
_edje_dragable_pos_set(Edje *ed, Edje_Real_Part *ep, double x, double y)
{
   if ((ep->part->dragable.x) || (ep->part->dragable.y))
     {
	if (ep->confine_to)
	  {
	     ep->drag.x = x;
	     ep->drag.y = y;
	  }
	else
	  {
	     ep->drag.x = x;
	     ep->drag.y = y;
	  }
     }
   ed->dirty = 1;
   _edje_recalc(ed);   
}

static void
_edje_part_recalc_single(Edje *ed,
			 Edje_Real_Part *ep, 
			 Edje_Part_Description *desc, 
			 Edje_Part_Description *chosen_desc,
			 Edje_Real_Part *rel1_to_x, 
			 Edje_Real_Part *rel1_to_y, 
			 Edje_Real_Part *rel2_to_x, 
			 Edje_Real_Part *rel2_to_y, 
			 Edje_Real_Part *confine_to,
			 Edje_Calc_Params *params)
{
   int minw, minh, maxw, maxh;

   /* relative coords of top left & bottom right */
   if (rel1_to_x)
     params->x = desc->rel1.offset_x +
     rel1_to_x->x + (desc->rel1.relative_x * rel1_to_x->w);
   else
     params->x = desc->rel1.offset_x +
     (desc->rel1.relative_x * ed->w);
   if (rel1_to_y)
     params->y = desc->rel1.offset_y +
     rel1_to_y->y + (desc->rel1.relative_y * rel1_to_y->h);
   else
     params->y = desc->rel1.offset_y +
     (desc->rel1.relative_y * ed->h);
   if (rel2_to_x)
     params->w = desc->rel2.offset_x +
     rel2_to_x->x + (desc->rel2.relative_x * rel2_to_x->w) -
     params->x + 1;
   else
     params->w = desc->rel2.offset_x +
     (desc->rel2.relative_x * ed->w) -
     params->x + 1;
   if (rel2_to_y)
     params->h = desc->rel2.offset_y +
     rel2_to_y->y + (desc->rel2.relative_y * rel2_to_y->h) -
     params->y + 1;
   else
     params->h = desc->rel2.offset_y +
     (desc->rel2.relative_y * ed->h) -
     params->y + 1;

   /* aspect */
   if (params->h > 0)
     {
	double aspect;
	double new_w, new_h;
   
	new_h = params->h;
	new_w = params->w;
	aspect = (double)params->w / (double)params->h;
	/* adjust for max aspect (width / height) */
	if ((desc->aspect.max > 0.0) && (aspect > desc->aspect.max))
	  {
	     new_h = (params->w / desc->aspect.max);
	     new_w = (params->h * desc->aspect.max);
	  }
	/* adjust for min aspect (width / height) */
	if ((desc->aspect.min > 0.0) && (aspect < desc->aspect.min))
	  {
	     new_h = (params->w / desc->aspect.min);
	     new_w = (params->h * desc->aspect.min);
	  }
	/* do real adjustment */
	if ((params->h - new_h) > (params->w - new_w))
	  {
	     if (params->h < new_h)
	       {
		  params->y = params->y +
		    ((params->h - new_h) * (1.0 - desc->align.y));
		  params->h = new_h;
	       }
	     else if (params->h > new_h)
	       {
		  params->y = params->y +
		    ((params->h - new_h) * desc->align.y);
		  params->h = new_h;
	       }
	  }
	else
	  {
	     if (params->w < new_w)
	       {
		  params->x = params->x +
		    ((params->w - new_w) * (1.0 - desc->align.x));
		  params->w = new_w;
	       }
	     else if (params->w > new_w)
	       {
		  params->x = params->x +
		    ((params->w - new_w) * desc->align.x);
		  params->w = new_w;
	       }
	  }
     }

   /* size step */
   if (desc->step.x > 0)
     {
	int steps;
	int new_w;
	
	steps = params->w / desc->step.x;
	new_w = desc->step.x * steps;
	if (params->w > new_w)
	  {
	     params->x = params->x +
	       ((params->w - new_w) * desc->align.x);
	     params->w = new_w;
	  }	
     }
   if (desc->step.y > 0)
     {
	int steps;
	int new_h;
	
	steps = params->h / desc->step.y;
	new_h = desc->step.y * steps;
	if (params->h > new_h)
	  {
	     params->y = params->y +
	       ((params->h - new_h) * desc->align.y);
	     params->h = new_h;
	  }	
     }
   minw = desc->min.w;
   minh = desc->min.h;
   if (ep->swallow_params.min.w > desc->min.w) minw = ep->swallow_params.min.w;
   if (ep->swallow_params.min.h > desc->min.h) minh = ep->swallow_params.min.h;
   /* if we have text that wants to make the min size the text size... */
   if ((chosen_desc) && (ep->part->type == EDJE_PART_TYPE_TEXT))
     {
	char   *text;
	char   *font;
	int     size;
	double  tw, th;
	
	text = chosen_desc->text.text;
	font = chosen_desc->text.font;
	size = chosen_desc->text.size;
	
	if ((chosen_desc->text.text_class) && (strlen(chosen_desc->text.text_class) > 0))
	  {
	     Edje_Text_Class *tc;
	     
	     tc = _edje_text_class_find(ed, chosen_desc->text.text_class);
	     if (tc)
	       {
		  if (tc->font) font = tc->font;
		  if (tc->size > 0) size = tc->size;
	       }
	  }
	
	if (ep->text.text) text = ep->text.text;
	if (ep->text.font) font = ep->text.font;
	if (ep->text.size > 0) size = ep->text.size;
	evas_object_text_font_set(ep->object, font, size);
	if ((chosen_desc->text.min_x) || (chosen_desc->text.min_y))
	  {
	     evas_object_text_text_set(ep->object, text);
	     evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	     if (chosen_desc->text.min_x)
	       minw = tw +
	       _edje_text_styles[ep->part->effect].pad.l +
	       _edje_text_styles[ep->part->effect].pad.r;
	     if (chosen_desc->text.min_y)
	       minh = th + 
	       _edje_text_styles[ep->part->effect].pad.t +
	       _edje_text_styles[ep->part->effect].pad.b;
	  }
     }
   /* rememebr what our size is BEFORE we go limit it */
   params->req.x = params->x;
   params->req.y = params->y;
   params->req.w = params->w;
   params->req.h = params->h;
   /* adjust for min size */
   if (minw >= 0)
     {
	if (params->w < minw)
	  {
	     params->x = params->x + 
	       ((params->w - minw) * (1.0 - desc->align.x));
	     params->w = minw;
	  }
     }
   if (minh >= 0)
     {
	if (params->h < minh)
	  {
	     params->y = params->y + 
	       ((params->h - minh) * (1.0 - desc->align.y));
	     params->h = minh;
	  }
     }
   maxw = desc->max.w;
   maxh = desc->max.h;
   if ((ep->swallow_params.max.w >= 0) &&
       (ep->swallow_params.max.w < maxw)) maxw = ep->swallow_params.max.w;
   if ((ep->swallow_params.max.h >= 0) &&
       (ep->swallow_params.max.h < maxh)) maxh = ep->swallow_params.max.h;
   /* adjust for max size */
   if (maxw >= 0)
     {
	if (params->w > maxw)
	  {
	     params->x = params->x + 
	       ((params->w - maxw) * desc->align.x);
	     params->w = maxw;
	  }
     }
   if (maxh >= 0)
     {
	if (params->h > maxh)
	  {
	     params->y = params->y + 
	       ((params->h - maxh) * desc->align.y);
	     params->h = maxh;
	  }
     }
   /* confine */
   if (confine_to)
     {
	int offset;
	int step;
	double v;
	
	/* complex dragable params */
	v = ep->drag.size.x * confine_to->w;
	if ((((minw > 0) && (v > minw)) || (minw <= 0)) &&
	    (((maxw >= 0) && (v < maxw)) || (maxw < 0))) params->w = v;
	offset = (ep->drag.x * (confine_to->w - params->w)) +
	  ep->drag.tmp.x;
	if (ep->part->dragable.step_x > 0)
	  {
	     params->x = confine_to->x + 
	       ((offset / ep->part->dragable.step_x) * ep->part->dragable.step_x);
	  }
	else if (ep->part->dragable.count_x > 0)
	  {
	     step = (confine_to->w - params->w) / ep->part->dragable.count_x;
	     params->x = confine_to->x +
	       ((offset / step) * step);	       
	  }
	v = ep->drag.size.y * confine_to->h;
	if ((((minh > 0) && (v > minh)) || (minh <= 0)) &&
	    (((maxh >= 0) && (v < maxh)) || (maxh < 0))) params->h = v;
	offset = (ep->drag.y * (confine_to->h - params->h)) +
	  ep->drag.tmp.y;
	if (ep->part->dragable.step_y > 0)
	  {
	     params->y = confine_to->y + 
	       ((offset / ep->part->dragable.step_y) * ep->part->dragable.step_y);
	  }
	else if (ep->part->dragable.count_y > 0)
	  {
	     step = (confine_to->h - params->h) / ep->part->dragable.count_y;
	     params->y = confine_to->y +
	       ((offset / step) * step);	       
	  }
	params->req_drag.x = params->x;
	params->req_drag.y = params->y;
	params->req_drag.w = params->w;
	params->req_drag.h = params->h;
	/* limit to confine */
	if (params->x < confine_to->x)
	  {
	     params->x = confine_to->x;
	  }
	if ((params->x + params->w) > (confine_to->x + confine_to->w))
	  {
	     params->x = confine_to->x + (confine_to->w - params->w);
	  }
	if (params->y < confine_to->y)
	  {
	     params->y = confine_to->y;
	  }
	if ((params->y + params->h) > (confine_to->y + confine_to->h))
	  {
	     params->y = confine_to->y + (confine_to->h - params->h);
	  }
     }
   else
     {
	/* simple dragable params */
	params->x += ep->drag.x + ep->drag.tmp.x;
	params->y += ep->drag.y + ep->drag.tmp.y;
	params->req_drag.x = params->x;
	params->req_drag.y = params->y;
	params->req_drag.w = params->w;
	params->req_drag.h = params->h;
     }
   /* fill */
   params->smooth = desc->fill.smooth;
   params->fill.x = desc->fill.pos_abs_x + (params->w * desc->fill.pos_rel_x);
   params->fill.y = desc->fill.pos_abs_y + (params->h * desc->fill.pos_rel_y);
   params->fill.w = desc->fill.abs_x + (params->w * desc->fill.rel_x);
   params->fill.h = desc->fill.abs_y + (params->h * desc->fill.rel_y);
   /* colors */
   params->color.r = desc->color.r;
   params->color.g = desc->color.g;
   params->color.b = desc->color.b;
   params->color.a = desc->color.a;
   params->color2.r = desc->color2.r;
   params->color2.g = desc->color2.g;
   params->color2.b = desc->color2.b;
   params->color2.a = desc->color2.a;
   params->color3.r = desc->color3.r;
   params->color3.g = desc->color3.g;
   params->color3.b = desc->color3.b;
   params->color3.a = desc->color3.a;
   /* visible */
   params->visible = desc->visible;
   /* border */
   params->border.l = desc->border.l;
   params->border.r = desc->border.r;
   params->border.t = desc->border.t;
   params->border.b = desc->border.b;
   /* text.align */
   params->text.align.x = desc->text.align.x;
   params->text.align.y = desc->text.align.y;
}

static void
_edje_part_recalc(Edje *ed, Edje_Real_Part *ep)
{
   Edje_Calc_Params p1, p2, p3;
   Edje_Part_Description *chosen_desc;
   double pos = 0.0;

   if (ep->calculated) return;
   if (ep->calculating)
     {
	printf("EDJE ERROR: circular dependancy when calculating part \"%s\"\n",
	       ep->part->name);
	return;
     }
   ep->calculating = 1;
   if (ep->param1.rel1_to_x)  _edje_part_recalc(ed, ep->param1.rel1_to_x);
   if (ep->param1.rel1_to_y)  _edje_part_recalc(ed, ep->param1.rel1_to_y);
   if (ep->param1.rel2_to_x)  _edje_part_recalc(ed, ep->param1.rel2_to_x);
   if (ep->param1.rel2_to_y)  _edje_part_recalc(ed, ep->param1.rel2_to_y);
   if (ep->confine_to)        _edje_part_recalc(ed, ep->confine_to);
   if (ep->param2.rel1_to_x)  _edje_part_recalc(ed, ep->param2.rel1_to_x);
   if (ep->param2.rel1_to_y)  _edje_part_recalc(ed, ep->param2.rel1_to_y);
   if (ep->param2.rel2_to_x)  _edje_part_recalc(ed, ep->param2.rel2_to_x);
   if (ep->param2.rel2_to_y)  _edje_part_recalc(ed, ep->param2.rel2_to_y);
   
   /* actually calculate now */
   if (ep->description_pos == 0.0)
     chosen_desc = ep->param1.description;
   else
     chosen_desc = ep->param2.description;
   if (!chosen_desc) chosen_desc = ep->param2.description;
   if (!chosen_desc)
     {
	ep->calculating = 0;
	ep->calculated = 1;
	return;
     }
   
   ep->chosen_description = chosen_desc;
   if (ep->param1.description)
     _edje_part_recalc_single(ed, ep, ep->param1.description, chosen_desc, ep->param1.rel1_to_x, ep->param1.rel1_to_y, ep->param1.rel2_to_x, ep->param1.rel2_to_y, ep->confine_to, &p1);
   if (ep->param2.description)
     {
	_edje_part_recalc_single(ed, ep, ep->param2.description, chosen_desc, ep->param2.rel1_to_x, ep->param2.rel1_to_y, ep->param2.rel2_to_x, ep->param2.rel2_to_y, ep->confine_to, &p2);

	pos = ep->description_pos;
	
	/* visible is special */
	if ((p1.visible) && (!p2.visible))
	  {
	     if (pos == 1.0)
	       p3.visible = 0;
	     else
	       p3.visible = 1;
	  }
	else if ((!p1.visible) && (p2.visible))
	  {
	     if (pos == 0.0)
	       p3.visible = 0;
	     else
	       p3.visible = 1;
	  }
	else
	  p3.visible = p1.visible;

	if (pos < 0.5)
	  p3.smooth = p1.smooth;
	else
	  p3.smooth = p2.smooth;
	
	p3.x = (p1.x * (1.0 - pos)) + (p2.x * (pos));
	p3.y = (p1.y * (1.0 - pos)) + (p2.y * (pos));
	p3.w = (p1.w * (1.0 - pos)) + (p2.w * (pos));
	p3.h = (p1.h * (1.0 - pos)) + (p2.h * (pos));

	p3.req.x = (p1.req.x * (1.0 - pos)) + (p2.req.x * (pos));
	p3.req.y = (p1.req.y * (1.0 - pos)) + (p2.req.y * (pos));
	p3.req.w = (p1.req.w * (1.0 - pos)) + (p2.req.w * (pos));
	p3.req.h = (p1.req.h * (1.0 - pos)) + (p2.req.h * (pos));

	p3.req_drag.x = (p1.req_drag.x * (1.0 - pos)) + (p2.req_drag.x * (pos));
	p3.req_drag.y = (p1.req_drag.y * (1.0 - pos)) + (p2.req_drag.y * (pos));
	p3.req_drag.w = (p1.req_drag.w * (1.0 - pos)) + (p2.req_drag.w * (pos));
	p3.req_drag.h = (p1.req_drag.h * (1.0 - pos)) + (p2.req_drag.h * (pos));
	
	p3.fill.x = (p1.fill.x * (1.0 - pos)) + (p2.fill.x * (pos));
	p3.fill.y = (p1.fill.y * (1.0 - pos)) + (p2.fill.y * (pos));
	p3.fill.w = (p1.fill.w * (1.0 - pos)) + (p2.fill.w * (pos));
	p3.fill.h = (p1.fill.h * (1.0 - pos)) + (p2.fill.h * (pos));
	
	p3.color.r = (p1.color.r * (1.0 - pos)) + (p2.color.r * (pos));
	p3.color.g = (p1.color.g * (1.0 - pos)) + (p2.color.g * (pos));
	p3.color.b = (p1.color.b * (1.0 - pos)) + (p2.color.b * (pos));
	p3.color.a = (p1.color.a * (1.0 - pos)) + (p2.color.a * (pos));
	
	p3.color2.r = (p1.color2.r * (1.0 - pos)) + (p2.color2.r * (pos));
	p3.color2.g = (p1.color2.g * (1.0 - pos)) + (p2.color2.g * (pos));
	p3.color2.b = (p1.color2.b * (1.0 - pos)) + (p2.color2.b * (pos));
	p3.color2.a = (p1.color2.a * (1.0 - pos)) + (p2.color2.a * (pos));
	
	p3.color3.r = (p1.color3.r * (1.0 - pos)) + (p2.color3.r * (pos));
	p3.color3.g = (p1.color3.g * (1.0 - pos)) + (p2.color3.g * (pos));
	p3.color3.b = (p1.color3.b * (1.0 - pos)) + (p2.color3.b * (pos));
	p3.color3.a = (p1.color3.a * (1.0 - pos)) + (p2.color3.a * (pos));
	
	p3.border.l = (p1.border.l * (1.0 - pos)) + (p2.border.l * (pos));
	p3.border.r = (p1.border.r * (1.0 - pos)) + (p2.border.r * (pos));
	p3.border.t = (p1.border.t * (1.0 - pos)) + (p2.border.t * (pos));
	p3.border.b = (p1.border.b * (1.0 - pos)) + (p2.border.b * (pos));

	p3.text.align.x = (p1.text.align.x * (1.0 - pos)) + (p2.text.align.x * (pos));
	p3.text.align.y = (p1.text.align.y * (1.0 - pos)) + (p2.text.align.y * (pos));
     }
   else
     p3 = p1;

   if ((chosen_desc->color_class) && (strlen(chosen_desc->color_class) > 0))
     {
	Edje_Color_Class *cc;
	
	cc = _edje_color_class_find(ed, chosen_desc->color_class);
	if (cc)
	  {
	     p3.color.r = (((int)cc->r + 1) * p3.color.r) >> 8;
	     p3.color.g = (((int)cc->g + 1) * p3.color.g) >> 8;
	     p3.color.b = (((int)cc->b + 1) * p3.color.b) >> 8;
	     p3.color.a = (((int)cc->a + 1) * p3.color.a) >> 8;
	     p3.color2.r = (((int)cc->r2 + 1) * p3.color2.r) >> 8;
	     p3.color2.g = (((int)cc->g2 + 1) * p3.color2.g) >> 8;
	     p3.color2.b = (((int)cc->b2 + 1) * p3.color2.b) >> 8;
	     p3.color2.a = (((int)cc->a2 + 1) * p3.color2.a) >> 8;
	     p3.color3.r = (((int)cc->r3 + 1) * p3.color3.r) >> 8;
	     p3.color3.g = (((int)cc->g3 + 1) * p3.color3.g) >> 8;
	     p3.color3.b = (((int)cc->b3 + 1) * p3.color3.b) >> 8;
	     p3.color3.a = (((int)cc->a3 + 1) * p3.color3.a) >> 8;
	  }
     }
   
   ep->req.x = p3.req.x;
   ep->req.y = p3.req.y;
   ep->req.w = p3.req.w;
   ep->req.h = p3.req.h;

   if (ep->drag.need_reset)
     {
	double dx, dy;
	
	dx = 0;
	dy = 0;
	_edje_part_dragable_calc(ed, ep, &dx, &dy);
        ep->drag.x = dx;
	ep->drag.y = dy;
	ep->drag.tmp.x = 0;
	ep->drag.tmp.y = 0;
	ep->drag.need_reset = 0;
     }
   if (!ed->calc_only)
     {
	if (ep->part->type == EDJE_PART_TYPE_RECTANGLE)
	  {
	     evas_object_move(ep->object, ed->x + p3.x, ed->y + p3.y);
	     evas_object_resize(ep->object, p3.w, p3.h);
	     evas_object_color_set(ep->object, p3.color.r, p3.color.g, p3.color.b, p3.color.a);
	     if (p3.visible) evas_object_show(ep->object);
	     else evas_object_hide(ep->object);
	  }
	else if (ep->part->type == EDJE_PART_TYPE_TEXT)
	  {
	     _edje_text_recalc_apply(ed, ep, &p3, chosen_desc);
	  }
	else if (ep->part->type == EDJE_PART_TYPE_IMAGE)
	  {
	     char buf[4096];
	     int image_id;
	     int image_count, image_num;
	     
	     evas_object_move(ep->object, ed->x + p3.x, ed->y + p3.y);
	     evas_object_resize(ep->object, p3.w, p3.h);
	     evas_object_image_fill_set(ep->object, p3.fill.x, p3.fill.y, p3.fill.w, p3.fill.h);
	     evas_object_image_smooth_scale_set(ep->object, p3.smooth);
	     
	     evas_object_image_border_set(ep->object, p3.border.l, p3.border.r, p3.border.t, p3.border.b);
	     image_id = ep->param1.description->image.id;
	     image_count = 2;
	     if (ep->param2.description)
	       image_count += evas_list_count(ep->param2.description->image.tween_list);
	     image_num = (pos * ((double)image_count - 0.5));
	     if (image_num > (image_count - 1))
	       image_num = image_count - 1;
	     if (image_num == 0)
	       image_id = ep->param1.description->image.id;
	     else if (image_num == (image_count - 1))
	       image_id = ep->param2.description->image.id;
	     else
	       {
		  Edje_Part_Image_Id *imid;
		  
		  imid = evas_list_nth(ep->param2.description->image.tween_list, image_num - 1);
		  if (imid) image_id = imid->id;
	       }
	     
	     snprintf(buf, sizeof(buf), "images/%i", image_id);
	     evas_object_image_file_set(ep->object, ed->file->path, buf);
	     evas_object_color_set(ep->object, p3.color.r, p3.color.g, p3.color.b, p3.color.a);
	     if (p3.visible) evas_object_show(ep->object);
	     else evas_object_hide(ep->object);
	  }
	if (ep->swallowed_object)
	  {
	     evas_object_move(ep->swallowed_object, ed->x + p3.x, ed->y + p3.y);
	     evas_object_resize(ep->swallowed_object, p3.w, p3.h);
	     if (p3.visible) evas_object_show(ep->swallowed_object);
	     else evas_object_hide(ep->swallowed_object);
	  }
     }
   
   ep->x = p3.x;
   ep->y = p3.y;
   ep->w = p3.w;
   ep->h = p3.h;
   
   ep->calculated = 1;
   ep->calculating = 0;
}
