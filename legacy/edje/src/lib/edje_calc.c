/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

#define FLAG_NONE 0
#define FLAG_X    0x01
#define FLAG_Y    0x02
#define FLAG_XY   (FLAG_X | FLAG_Y)

static void _edje_part_recalc_single(Edje *ed, Edje_Real_Part *ep, Edje_Part_Description *desc, Edje_Part_Description *chosen_desc, Edje_Real_Part *rel1_to_x, Edje_Real_Part *rel1_to_y, Edje_Real_Part *rel2_to_x, Edje_Real_Part *rel2_to_y, Edje_Real_Part *confine_to, Edje_Calc_Params *params, int flags);
static void _edje_part_recalc(Edje *ed, Edje_Real_Part *ep, int flags);

void
_edje_part_pos_set(Edje *ed, Edje_Real_Part *ep, int mode, double pos)
{
   double npos;

   pos = CLAMP(pos, 0.0, 1.0);

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

Edje_Part_Description *
_edje_part_description_find(Edje *ed, Edje_Real_Part *rp, const char *name,
                            double val)
{
   Edje_Part *ep = rp->part;
   Edje_Part_Description *ret = NULL;
   Evas_List *l;
   double min_dst = 999.0;

   if (!strcmp(name, "default") && val == 0.0)
     return ep->default_desc;

   if (!strcmp(name, "custom"))
     return rp->custom.description;

   if (!strcmp(name, "default"))
     {
	ret = ep->default_desc;
	min_dst = ABS(ep->default_desc->state.value - val);
     }

   for (l = ep->other_desc; l; l = l->next)
     {
	Edje_Part_Description *d = l->data;

	if (!strcmp(d->state.name, name))
	  {
	     double dst;

	     dst = ABS(d->state.value - val);
	     if (dst < min_dst)
	       {
		  ret = d;
		  min_dst = dst;
	       }
	  }
     }

   return ret;
}

void
_edje_part_description_apply(Edje *ed, Edje_Real_Part *ep, const char *d1, double v1, const char *d2, double v2)
{
   if (!d1) d1 = "default";
   if (!d2) d2 = "default";

   ep->param1.description = _edje_part_description_find(ed, ep, d1, v1);
   if (!ep->param1.description)
     ep->param1.description = ep->part->default_desc; /* never NULL */

   ep->param2.description = _edje_part_description_find(ed, ep, d2, v2);

   ep->param1.rel1_to_x = ep->param1.rel1_to_y = NULL;
   ep->param1.rel2_to_x = ep->param1.rel2_to_y = NULL;

   if (ep->param1.description->rel1.id_x >= 0)
     ep->param1.rel1_to_x = ed->table_parts[ep->param1.description->rel1.id_x % ed->table_parts_size];
   if (ep->param1.description->rel1.id_y >= 0)
     ep->param1.rel1_to_y = ed->table_parts[ep->param1.description->rel1.id_y % ed->table_parts_size];
   if (ep->param1.description->rel2.id_x >= 0)
     ep->param1.rel2_to_x = ed->table_parts[ep->param1.description->rel2.id_x % ed->table_parts_size];
   if (ep->param1.description->rel2.id_y >= 0)
     ep->param1.rel2_to_y = ed->table_parts[ep->param1.description->rel2.id_y % ed->table_parts_size];

   ep->param2.rel1_to_x = ep->param2.rel1_to_y = NULL;
   ep->param2.rel2_to_x = ep->param2.rel2_to_y = NULL;

   if (ep->param2.description)
     {
	if (ep->param2.description->rel1.id_x >= 0)
	  ep->param2.rel1_to_x = ed->table_parts[ep->param2.description->rel1.id_x % ed->table_parts_size];
	if (ep->param2.description->rel1.id_y >= 0)
	  ep->param2.rel1_to_y = ed->table_parts[ep->param2.description->rel1.id_y % ed->table_parts_size];
	if (ep->param2.description->rel2.id_x >= 0)
	  ep->param2.rel2_to_x = ed->table_parts[ep->param2.description->rel2.id_x % ed->table_parts_size];
	if (ep->param2.description->rel2.id_y >= 0)
	  ep->param2.rel2_to_y = ed->table_parts[ep->param2.description->rel2.id_y % ed->table_parts_size];
     }

   if (ep->description_pos == 0.0)
     ep->chosen_description = ep->param1.description;
   else
     ep->chosen_description = ep->param2.description;

   ed->dirty = 1;
}

void
_edje_recalc(Edje *ed)
{
   int i;

   if (!ed->dirty)
     {
	return;
     }
   if ((ed->freeze > 0) || (_edje_freeze_val > 0))
     {
	ed->recalc = 1;
	if (!ed->calc_only)
	  {
	     if (_edje_freeze_val > 0) _edje_freeze_calc_count++;
	     return;
	  }
     }
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *ep;

	ep = ed->table_parts[i];
	ep->calculated = FLAG_NONE;
	ep->calculating = FLAG_NONE;
     }
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *ep;

	ep = ed->table_parts[i];
	if (ep->calculated != FLAG_XY)
	  _edje_part_recalc(ed, ep, (~ep->calculated) & FLAG_XY);
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
	     int ret = 0;

	     if ((ep->part->dragable.x != 0) &&
		 (ep->part->dragable.y != 0 )) ret = 3;
	     else if (ep->part->dragable.x != 0) ret = 1;
	     else if (ep->part->dragable.y != 0) ret = 2;

	     dx = ep->x - ep->confine_to->x;
	     dw = ep->confine_to->w - ep->w;
	     if (dw != 0.0) dx /= dw;
	     else dx = 0.0;

	     dy = ep->y - ep->confine_to->y;
	     dh = ep->confine_to->h - ep->h;
	     if (dh != 0) dy /= dh;
	     else dy = 0.0;

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
   /* check whether this part is dragable at all */
   if (!ep->part->dragable.x && !ep->part->dragable.y) return;

   /* instead of checking for equality, we really should check that
    * the difference is greater than foo, but I have no idea what
    * value we would set foo to, because it would depend on the
    * size of the dragable...
    */
   if (ep->drag.x != x || ep->drag.tmp.x)
     {
	ep->drag.x = x;
	ep->drag.tmp.x = 0;
	ep->drag.need_reset = 0;
	ed->dirty = 1;
     }

   if (ep->drag.y != y || ep->drag.tmp.y)
     {
	ep->drag.y = y;
	ep->drag.tmp.y = 0;
	ep->drag.need_reset = 0;
	ed->dirty = 1;
     }

   _edje_recalc(ed); /* won't do anything if dirty flag isn't set */
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
			 Edje_Calc_Params *params,
			 int flags)
{
   int minw = 0, minh = 0, maxw = 0, maxh = 0;

   flags = FLAG_XY;

//   if (flags & FLAG_X)
     {
	minw = desc->min.w;
	if (ep->part->scale) minw = (int)(((double)minw) * _edje_scale);
	if (ep->swallow_params.min.w > desc->min.w)
	  minw = ep->swallow_params.min.w;

	/* XXX TODO: remove need of EDJE_INF_MAX_W, see edje_util.c */
	if ((ep->swallow_params.max.w <= 0) ||
	    (ep->swallow_params.max.w == EDJE_INF_MAX_W))
	  {
	     maxw = desc->max.w;
	     if (maxw > 0)
	       {
		  if (ep->part->scale) maxw = (int)(((double)maxw) * _edje_scale);
		  if (maxw < 1) maxw = 1;
	       }
	  }
	else
	  {
	     if (desc->max.w <= 0)
	       maxw = ep->swallow_params.max.w;
	     else
	       {
		  maxw = desc->max.w;
		  if (maxw > 0)
		    {
		       if (ep->part->scale) maxw = (int)(((double)maxw) * _edje_scale);
		       if (maxw < 1) maxw = 1;
		    }
		  if (ep->swallow_params.max.w < maxw)
		    maxw = ep->swallow_params.max.w;
	       }
	  }
	if (maxw >= 0)
	  {
	     if (maxw < minw) maxw = minw;
	  }
     }
//   if (flags & FLAG_Y)
     {
	minh = desc->min.h;
	if (ep->part->scale) minh = (int)(((double)minh) * _edje_scale);
	if (ep->swallow_params.min.h > desc->min.h)
	  minh = ep->swallow_params.min.h;

	/* XXX TODO: remove need of EDJE_INF_MAX_H, see edje_util.c */
	if ((ep->swallow_params.max.h <= 0) ||
	    (ep->swallow_params.max.h == EDJE_INF_MAX_H))
	  {
	     maxh = desc->max.h;
	     if (maxh > 0)
	       {
		  if (ep->part->scale) maxh = (int)(((double)maxh) * _edje_scale);
		  if (maxh < 1) maxh = 1;
	       }
	  }
	else
	  {
	     if (desc->max.h <= 0)
	       maxh = ep->swallow_params.max.h;
	     else
	       {
		  maxh = desc->max.h;
		  if (maxh > 0)
		    {
		       if (ep->part->scale) maxh = (int)(((double)maxh) * _edje_scale);
		       if (maxh < 1) maxh = 1;
		    }
		  if (ep->swallow_params.max.h < maxh)
		    maxh = ep->swallow_params.max.h;
	       }
	  }
	if (maxh >= 0)
	  {
	     if (maxh < minh) maxh = minh;
	  }
     }
   /* relative coords of top left & bottom right */
   if (flags & FLAG_X)
     {
	if (rel1_to_x)
	  params->x = desc->rel1.offset_x +
	  rel1_to_x->x + (desc->rel1.relative_x * rel1_to_x->w);
	else
	  params->x = desc->rel1.offset_x +
	  (desc->rel1.relative_x * ed->w);
	if (rel2_to_x)
	  params->w = desc->rel2.offset_x +
	  rel2_to_x->x + (desc->rel2.relative_x * rel2_to_x->w) -
	  params->x + 1;
	else
	  params->w = desc->rel2.offset_x +
	  (desc->rel2.relative_x * ed->w) -
	  params->x + 1;
     }
   if (flags & FLAG_Y)
     {
	if (rel1_to_y)
	  params->y = desc->rel1.offset_y +
	  rel1_to_y->y + (desc->rel1.relative_y * rel1_to_y->h);
	else
	  params->y = desc->rel1.offset_y +
	  (desc->rel1.relative_y * ed->h);
	if (rel2_to_y)
	  params->h = desc->rel2.offset_y +
	  rel2_to_y->y + (desc->rel2.relative_y * rel2_to_y->h) -
	  params->y + 1;
	else
	  params->h = desc->rel2.offset_y +
	  (desc->rel2.relative_y * ed->h) -
	  params->y + 1;
     }

   /* aspect */
   if ((params->h > 0) &&
       (((flags | ep->calculated) & FLAG_XY) == FLAG_XY))
     {
	int apref;
	double aspect, amax, amin;
	double new_w = 0, new_h = 0, want_x, want_y, want_w, want_h;

	want_x = params->x;
	want_w = new_w = params->w;

	want_y = params->y;
	want_h = new_h = params->h;

	aspect = (double)params->w / (double)params->h;
	apref = desc->aspect.prefer;
	amax = desc->aspect.max;
	amin = desc->aspect.min;
	if ((ep->swallow_params.aspect.w > 0) &&
	    (ep->swallow_params.aspect.h > 0))
	  amin = amax =
	  (double)ep->swallow_params.aspect.w /
	  (double)ep->swallow_params.aspect.h;
	if (ep->swallow_params.aspect.mode > EDJE_ASPECT_CONTROL_NONE)
	  {
	     switch (ep->swallow_params.aspect.mode)
	       {
		case EDJE_ASPECT_CONTROL_NEITHER:
		  apref = EDJE_ASPECT_PREFER_NONE;
		  break;
		case EDJE_ASPECT_CONTROL_HORIZONTAL:
		  apref = EDJE_ASPECT_PREFER_HORIZONTAL;
		  break;
		case EDJE_ASPECT_CONTROL_VERTICAL:
		  apref = EDJE_ASPECT_PREFER_VERTICAL;
		  break;
		case EDJE_ASPECT_CONTROL_BOTH:
		  apref = EDJE_ASPECT_PREFER_BOTH;
		  break;
		default:
		  break;
	       }
	  }

	switch (apref)
	  {
	   case EDJE_ASPECT_PREFER_NONE:
	      /* keep botth dimensions in check */
	      /* adjust for min aspect (width / height) */
	      if ((amin > 0.0) && (aspect < amin))
		{
		   new_h = (params->w / amin);
		   new_w = (params->h * amin);
		}
	      /* adjust for max aspect (width / height) */
	      if ((amax > 0.0) && (aspect > amax))
		{
		   new_h = (params->w / amax);
		   new_w = (params->h * amax);
		}
	      if ((amax > 0.0) && (new_w < params->w))
		{
		   new_w = params->w;
		   new_h = params->w / amax;
		}
	      if ((amax > 0.0) && (new_h < params->h))
		{
		   new_w = params->h * amax;
		   new_h = params->h;
		}
	      break;
	   /* prefer vertical size as determiner */
	   case  EDJE_ASPECT_PREFER_VERTICAL:
	      /* keep both dimensions in check */
	      /* adjust for max aspect (width / height) */
	      if ((amax > 0.0) && (aspect > amax))
		new_w = (params->h * amax);
	      /* adjust for min aspect (width / height) */
	      if ((amin > 0.0) && (aspect < amin))
		new_w = (params->h * amin);
	      break;
	   /* prefer horizontal size as determiner */
	   case EDJE_ASPECT_PREFER_HORIZONTAL:
	      /* keep both dimensions in check */
	      /* adjust for max aspect (width / height) */
	      if ((amax > 0.0) && (aspect > amax))
		new_h = (params->w / amax);
	      /* adjust for min aspect (width / height) */
	      if ((amin > 0.0) && (aspect < amin))
		new_h = (params->w / amin);
	      break;
	   case EDJE_ASPECT_PREFER_BOTH:
	      /* keep both dimensions in check */
	      /* adjust for max aspect (width / height) */
	      if ((amax > 0.0) && (aspect > amax))
		{
		   new_w = (params->h * amax);
		   new_h = (params->w / amax);
		}
	      /* adjust for min aspect (width / height) */
	      if ((amin > 0.0) && (aspect < amin))
		{
		   new_w = (params->h * amin);
		   new_h = (params->w / amin);
		}
	      break;
	  }

        if (!((amin > 0.0) && (amax > 0.0) && (apref == EDJE_ASPECT_PREFER_NONE)))
	  {
	     if ((maxw >= 0) && (new_w > maxw)) new_w = maxw;
	     if (new_w < minw) new_w = minw;

	     if ((maxh >= 0) && (new_h > maxh)) new_h = maxh;
	     if (new_h < minh) new_h = minh;
	  }

	/* do real adjustment */
	if (apref == EDJE_ASPECT_PREFER_BOTH)
	  {
	     if (amin == 0.0) amin = amax;
	     if (amin != 0.0)
	       {
		  /* fix h and vary w */
		  if (new_w > params->w)
		    {
//		  params->w = new_w;
// EXCEEDS BOUNDS in W
		       new_h = (params->w / amin);
		       new_w = params->w;
		       if (new_h > params->h)
			 {
			    new_h = params->h;
			    new_w = (params->h * amin);
			 }
		    }
		  /* fix w and vary h */
		  else
		    {
//		  params->h = new_h;
// EXCEEDS BOUNDS in H
		       new_h = params->h;
		       new_w = (params->h * amin);
		       if (new_w > params->w)
			 {
			    new_h = (params->w / amin);
			    new_w = params->w;
			 }
		    }
		  params->w = new_w;
		  params->h = new_h;
	       }
	  }
	else
	  {
	     if ((amin > 0.0) && (amax > 0.0) && (apref == EDJE_ASPECT_PREFER_NONE))
	       {
		  params->w = new_w;
		  params->h = new_h;
	       }
	     else if ((params->h - new_h) > (params->w - new_w))
	       {
		  if (params->h < new_h)
		    params->h = new_h;
		  else if (params->h > new_h)
		    params->h = new_h;
		  if (apref == EDJE_ASPECT_PREFER_VERTICAL)
		    params->w = new_w;
	       }
	     else
	       {
		  if (params->w < new_w)
		    params->w = new_w;
		  else if (params->w > new_w)
		    params->w = new_w;
		  if (apref == EDJE_ASPECT_PREFER_HORIZONTAL)
		    params->h = new_h;
	       }
	  }
	params->x = want_x + ((want_w - params->w) * desc->align.x);
	params->y = want_y + ((want_h - params->h) * desc->align.y);
     }

   /* size step */
   if (flags & FLAG_X)
     {
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
     }
   if (flags & FLAG_Y)
     {
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
     }
   /* if we have text that wants to make the min size the text size... */
   if ((chosen_desc) && (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK))
     {
	Evas_Coord tw, th, ins_l, ins_r, ins_t, ins_b;
	const char *text = "";
	const char *style = "";
	Edje_Style *stl  = NULL;
	Evas_List *l;

	if (chosen_desc->text.id_source >= 0)
	  {
	     ep->text.source = ed->table_parts[chosen_desc->text.id_source % ed->table_parts_size];
	     if (ep->text.source->chosen_description->text.style)
	       style = ep->text.source->chosen_description->text.style;
	  }
	else
	  {
	     ep->text.source = NULL;
	     if (chosen_desc->text.style)
	       style = chosen_desc->text.style;
	  }

	if (chosen_desc->text.id_text_source >= 0)
	  {
	     ep->text.text_source = ed->table_parts[chosen_desc->text.id_text_source % ed->table_parts_size];
	     text = ep->text.text_source->chosen_description->text.text;
	     if (ep->text.text_source->text.text) text = ep->text.text_source->text.text;
	  }
	else
	  {
	     ep->text.text_source = NULL;
	     text = chosen_desc->text.text;
	     if (ep->text.text) text = ep->text.text;
	  }

	for (l = ed->file->styles; l; l = l->next)
	  {
	     stl = l->data;
	     if ((stl->name) && (!strcmp(stl->name, style))) break;
	     stl = NULL;
	  }

	if (ep->part->scale)
	  evas_object_scale_set(ep->object, _edje_scale);
	
	if (stl)
	  {
	     const char *ptxt;

	     if (evas_object_textblock_style_get(ep->object) != stl->style)
	       evas_object_textblock_style_set(ep->object, stl->style);
	     // FIXME: need to account for editing
	     if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
	       {
		  // do nothing - should be done elsewhere
	       }
	     else
	       {
		  ptxt = evas_object_textblock_text_markup_get(ep->object);
		  if (((!ptxt) && (text)) ||
		      ((ptxt) && (text) && (strcmp(ptxt, text))) ||
		      ((ptxt) && (!text)))
		    evas_object_textblock_text_markup_set(ep->object, text);
	       }
	     if ((chosen_desc->text.min_x) || (chosen_desc->text.min_y))
	       {
		  int mw = 0, mh = 0;

		  tw = th = 0;
		  if (!chosen_desc->text.min_x)
		    {
		       evas_object_resize(ep->object, params->w, params->h);
		       evas_object_textblock_size_formatted_get(ep->object, &tw, &th);
		    }
		  else
		    evas_object_textblock_size_native_get(ep->object, &tw, &th);
		  evas_object_textblock_style_insets_get(ep->object, &ins_l, &ins_r, &ins_t, &ins_b);
		  mw = ins_l + tw + ins_r;
		  mh = ins_t + th + ins_b;
		  if (chosen_desc->text.min_x)
		    {
		       if (mw > minw) minw = mw;
		    }
		  if (chosen_desc->text.min_y)
		    {
		       if (mh > minh) minh = mh;
		    }
	       }
	  }
	if ((chosen_desc->text.max_x) || (chosen_desc->text.max_y))
	  {
	     int mw = 0, mh = 0;

	     tw = th = 0;
	     if (!chosen_desc->text.max_x)
	       {
		  evas_object_resize(ep->object, params->w, params->h);
		  evas_object_textblock_size_formatted_get(ep->object, &tw, &th);
	       }
	     else
	       evas_object_textblock_size_native_get(ep->object, &tw, &th);
	     evas_object_textblock_style_insets_get(ep->object, &ins_l, &ins_r, &ins_t, &ins_b);
	     mw = ins_l + tw + ins_r;
	     mh = ins_t + th + ins_b;
	     if (chosen_desc->text.max_x)
	       {
		  if (mw > maxw) maxw = mw;
	       }
	     if (chosen_desc->text.max_y)
	       {
		  if (mh > maxw) maxh = mh;
	       }
	  }
     }
   else if ((chosen_desc) && (ep->part->type == EDJE_PART_TYPE_TEXT))
     {
	const char	*text;
	const char	*font;
	char            *sfont = NULL;
	int		 size;
	Evas_Coord	 tw, th;
	int		 inlined_font = 0;

	/* Update a object_text part */

	if (chosen_desc->text.id_source >= 0)
	  ep->text.source = ed->table_parts[chosen_desc->text.id_source % ed->table_parts_size];
	else
	  ep->text.source = NULL;

	if (chosen_desc->text.id_text_source >= 0)
	  ep->text.text_source = ed->table_parts[chosen_desc->text.id_text_source % ed->table_parts_size];
	else
	  ep->text.text_source = NULL;

	if (ep->text.text_source)
	  text = ep->text.text_source->chosen_description->text.text;
	else
	  text = chosen_desc->text.text;

	if (ep->text.source)
	  font = _edje_text_class_font_get(ed, ep->text.source->chosen_description, &size, &sfont);
	else
	  font = _edje_text_class_font_get(ed, chosen_desc, &size, &sfont);

	if (!font) font = "";

	if (ep->text.text_source)
	  {
	     if (ep->text.text_source->text.text) text = ep->text.text_source->text.text;
	  }
	else
	  {
	     if (ep->text.text) text = ep->text.text;
	  }

	if (ep->text.source)
	  {
	     if (ep->text.source->text.font) font = ep->text.source->text.font;
	     if (ep->text.source->text.size > 0) size = ep->text.source->text.size;
	  }
	else
	  {
	     if (ep->text.font) font = ep->text.font;
	     if (ep->text.size > 0) size = ep->text.size;
	  }
	if (!text) text = "";

        /* check if the font is embedded in the .eet */
	if (ed->file->font_hash)
	  {
	     Edje_Font_Directory_Entry *fnt;

	     fnt = evas_hash_find(ed->file->font_hash, font);

	     if (fnt)
	       {
		  font = fnt->path;
		  inlined_font = 1;
	       }
	  }
	if (ep->part->scale)
	  evas_object_scale_set(ep->object, _edje_scale);
	if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
	else evas_object_text_font_source_set(ep->object, NULL);

	if ((_edje_fontset_append) && (font))
	  {
	     char *font2;

	     font2 = malloc(strlen(font) + 1 + strlen(_edje_fontset_append) + 1);
	     if (font2)
	       {
		  strcpy(font2, font);
		  strcat(font2, ",");
		  strcat(font2, _edje_fontset_append);
		  evas_object_text_font_set(ep->object, font2, size);
		  free(font2);
	       }
	  }
	else
	  evas_object_text_font_set(ep->object, font, size);
	if ((chosen_desc->text.min_x) || (chosen_desc->text.min_y) ||
	    (chosen_desc->text.max_x) || (chosen_desc->text.max_y))
	  {
	     int mw, mh;
	     Evas_Text_Style_Type style;
	     const Evas_Text_Style_Type styles[] = {
		EVAS_TEXT_STYLE_PLAIN,
		EVAS_TEXT_STYLE_PLAIN,
		EVAS_TEXT_STYLE_OUTLINE,
		EVAS_TEXT_STYLE_SOFT_OUTLINE,
		EVAS_TEXT_STYLE_SHADOW,
		EVAS_TEXT_STYLE_SOFT_SHADOW,
		EVAS_TEXT_STYLE_OUTLINE_SHADOW,
		EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW,
		EVAS_TEXT_STYLE_FAR_SHADOW,
		EVAS_TEXT_STYLE_FAR_SOFT_SHADOW,
		EVAS_TEXT_STYLE_GLOW
	     };

	     if (ep->part->effect < EDJE_TEXT_EFFECT_LAST)
	       style = styles[ep->part->effect];
	     else
	       style = EVAS_TEXT_STYLE_PLAIN;

	     evas_object_text_style_set(ep->object, style);
	     evas_object_text_text_set(ep->object, text);
	     evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	     if (chosen_desc->text.max_x)
	       {
		  int l, r;
		  evas_object_text_style_pad_get(ep->object, &l, &r, NULL, NULL);
		  mw = tw + l + r;
		  if ((maxw < 0) || (mw < maxw)) maxw = mw;
	       }
	     if (chosen_desc->text.max_y)
	       {
		  int t, b;
		  evas_object_text_style_pad_get(ep->object, NULL, NULL, &t, &b);
		  mh = th + t + b;
		  if ((maxh < 0) || (mh < maxh)) maxh = mh;
	       }
	     if (chosen_desc->text.min_x)
	       {
		  int l, r;
		  evas_object_text_style_pad_get(ep->object, &l, &r, NULL, NULL);
		  mw = tw + l + r;
		  if (mw > minw) minw = mw;
	       }
	     if (chosen_desc->text.min_y)
	       {
		  int t, b;
		  evas_object_text_style_pad_get(ep->object, NULL, NULL, &t, &b);
		  mh = th + t + b;
		  if (mh > minh) minh = mh;
	       }
	  }
	if (sfont) free(sfont);
     }
   /* rememebr what our size is BEFORE we go limit it */
   params->req.x = params->x;
   params->req.y = params->y;
   params->req.w = params->w;
   params->req.h = params->h;
   /* adjust for min size */
   if (flags & FLAG_X)
     {
	if (minw >= 0)
	  {
	     if (params->w < minw)
	       {
		  params->x = params->x +
		    ((params->w - minw) * desc->align.x);
		  params->w = minw;
	       }
	  }
     }
   if (flags & FLAG_Y)
     {
	if (minh >= 0)
	  {
	     if (params->h < minh)
	       {
		  params->y = params->y +
		    ((params->h - minh) * desc->align.y);
		  params->h = minh;
	       }
	  }
     }
   /* adjust for max size */
   if (flags & FLAG_X)
     {
	if (maxw >= 0)
	  {
	     if (params->w > maxw)
	       {
		  params->x = params->x +
		    ((params->w - maxw) * desc->align.x);
		  params->w = maxw;
	       }
	  }
     }
   if (flags & FLAG_Y)
     {
	if (maxh >= 0)
	  {
	     if (params->h > maxh)
	       {
		  params->y = params->y +
		    ((params->h - maxh) * desc->align.y);
		  params->h = maxh;
	       }
	  }
     }
   /* confine */
   if (confine_to)
     {
	int offset;
	int step;
	double v;

	/* complex dragable params */
	if (flags & FLAG_X)
	  {
	     v = ep->drag.size.x * confine_to->w;

	     if ((minw > 0) && (v < minw)) params->w = minw;
	     else if ((maxw >= 0) && (v > maxw)) params->w = maxw;
	     else params->w = v;

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
		  if (step < 1) step = 1;
		  params->x = confine_to->x +
		    ((offset / step) * step);
	       }
	     params->req_drag.x = params->x;
	     params->req_drag.w = params->w;
	  }
	if (flags & FLAG_Y)
	  {
	     v = ep->drag.size.y * confine_to->h;

	     if ((minh > 0) && (v < minh)) params->h = minh;
	     else if ((maxh >= 0) && (v > maxh)) params->h = maxh;
	     else params->h = v;

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
		  if (step < 1) step = 1;
		  params->y = confine_to->y +
		    ((offset / step) * step);
	       }
	     params->req_drag.y = params->y;
	     params->req_drag.h = params->h;
	  }
	/* limit to confine */
	if (flags & FLAG_X)
	  {
	     if (params->x < confine_to->x)
	       {
		  params->x = confine_to->x;
	       }
	     if ((params->x + params->w) > (confine_to->x + confine_to->w))
	       {
		  params->x = confine_to->x + (confine_to->w - params->w);
	       }
	  }
	if (flags & FLAG_Y)
	  {
	     if (params->y < confine_to->y)
	       {
		  params->y = confine_to->y;
	       }
	     if ((params->y + params->h) > (confine_to->y + confine_to->h))
	       {
		  params->y = confine_to->y + (confine_to->h - params->h);
	       }
	  }
     }
   else
     {
	/* simple dragable params */
	if (flags & FLAG_X)
	  {
	     params->x += ep->drag.x + ep->drag.tmp.x;
	     params->req_drag.x = params->x;
	     params->req_drag.w = params->w;
	  }
	if (flags & FLAG_Y)
	  {
	     params->y += ep->drag.y + ep->drag.tmp.y;
	     params->req_drag.y = params->y;
	     params->req_drag.h = params->h;
	  }
     }
   /* fill */

   if (ep->part->type == EDJE_PART_TYPE_GRADIENT && desc->gradient.use_rel && (!desc->gradient.type || !strcmp(desc->gradient.type, "linear")))
     {
	int x2, y2;
	int dx, dy;
	double m;
	int angle;

	params->fill.x = desc->gradient.rel1.offset_x + (params->w * desc->gradient.rel1.relative_x);
	params->fill.y = desc->gradient.rel1.offset_y + (params->h * desc->gradient.rel1.relative_y);

	x2 = desc->gradient.rel2.offset_x + (params->w * desc->gradient.rel2.relative_x);
	y2 = desc->gradient.rel2.offset_y + (params->h * desc->gradient.rel2.relative_y);

	params->fill.w = 1; /* doesn't matter for linear grads */

	dy = y2 - params->fill.y;
	dx = x2 - params->fill.x;
	params->fill.h = sqrt(dx * dx + dy * dy);

	params->fill.spread = desc->fill.spread;

	if (dx == 0 && dy == 0)
	  {
	     angle = 0;
	  }
	else if (dx == 0)
	  {
	     if (dy > 0) angle = 0;
	     else angle = 180;
	  }
	else if (dy == 0)
	  {
	     if (dx > 0) angle = 270;
	     else angle = 90;
	  }
	else
	  {
	     m = (double)dx / (double)dy;
	     angle = atan(m) * 180 / M_PI;
	     if (dy < 0)
	       angle = 180 - angle;
	     else
	       angle = 360 - angle;
	  }
	params->fill.angle = angle;
     }
   else
     {
	params->smooth = desc->fill.smooth;
	if (flags & FLAG_X)
	  {
	     int fw;

             if (desc->fill.type == EDJE_FILL_TYPE_TILE)
	       evas_object_image_size_get(ep->object, &fw, NULL);
	     else
	       fw = params->w;

	     params->fill.x = desc->fill.pos_abs_x + (fw * desc->fill.pos_rel_x);
	     params->fill.w = desc->fill.abs_x + (fw * desc->fill.rel_x);
	  }
	if (flags & FLAG_Y)
	  {
	     int fh;
             if (desc->fill.type == EDJE_FILL_TYPE_TILE)
	       evas_object_image_size_get(ep->object, NULL, &fh);
	     else
	       fh = params->h;

	     params->fill.y = desc->fill.pos_abs_y + (fh * desc->fill.pos_rel_y);
	     params->fill.h = desc->fill.abs_y + (fh * desc->fill.rel_y);
	  }
	params->fill.angle = desc->fill.angle;
	params->fill.spread = desc->fill.spread;
     }
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

   if ((desc->color_class) && (*desc->color_class))
     {
	Edje_Color_Class *cc;

	cc = _edje_color_class_find(ed, desc->color_class);
	if (cc)
	  {
	     params->color.r = (((int)cc->r + 1) * desc->color.r) >> 8;
	     params->color.g = (((int)cc->g + 1) * desc->color.g) >> 8;
	     params->color.b = (((int)cc->b + 1) * desc->color.b) >> 8;
	     params->color.a = (((int)cc->a + 1) * desc->color.a) >> 8;
	     params->color2.r = (((int)cc->r2 + 1) * desc->color2.r) >> 8;
	     params->color2.g = (((int)cc->g2 + 1) * desc->color2.g) >> 8;
	     params->color2.b = (((int)cc->b2 + 1) * desc->color2.b) >> 8;
	     params->color2.a = (((int)cc->a2 + 1) * desc->color2.a) >> 8;
	     params->color3.r = (((int)cc->r3 + 1) * desc->color3.r) >> 8;
	     params->color3.g = (((int)cc->g3 + 1) * desc->color3.g) >> 8;
	     params->color3.b = (((int)cc->b3 + 1) * desc->color3.b) >> 8;
	     params->color3.a = (((int)cc->a3 + 1) * desc->color3.a) >> 8;
	  }
     }


   /* visible */
   params->visible = desc->visible;
   /* border */
   if (flags & FLAG_X)
     {
	params->border.l = desc->border.l;
	params->border.r = desc->border.r;
     }
   if (flags & FLAG_Y)
     {
	params->border.t = desc->border.t;
	params->border.b = desc->border.b;
     }
   /* text.align */
   if (flags & FLAG_X)
     {
	params->text.align.x = desc->text.align.x;
     }
   if (flags & FLAG_Y)
     {
	params->text.align.y = desc->text.align.y;
     }
   params->text.elipsis = desc->text.elipsis;
   params->gradient.id = desc->gradient.id;
   params->gradient.type = desc->gradient.type;
}

static void
_edje_gradient_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *p3, Edje_Part_Description *chosen_desc)
{
   evas_object_gradient_fill_angle_set(ep->object, p3->fill.angle);
   evas_object_gradient_fill_spread_set(ep->object, p3->fill.spread);
   evas_object_gradient_fill_set(ep->object, p3->fill.x, p3->fill.y,
				 p3->fill.w, p3->fill.h);

   if (p3->gradient.type && p3->gradient.type[0])
     evas_object_gradient_type_set(ep->object, p3->gradient.type, NULL);

   if (ed->file->spectrum_dir && ed->file->spectrum_dir->entries &&
       p3->gradient.id != ep->gradient_id)
     {
	Edje_Spectrum_Directory_Entry *se;
	Evas_List *l;

	se = evas_list_nth(ed->file->spectrum_dir->entries, p3->gradient.id);
	if (se)
	  {
	     evas_object_gradient_clear(ep->object);
	     for (l = se->color_list; l; l = l->next)
	       {
		  Edje_Spectrum_Color *sc = l->data;
		  evas_object_gradient_color_stop_add(ep->object, sc->r,
						      sc->g, sc->b, 255,
						      sc->d);
		  evas_object_gradient_alpha_stop_add(ep->object,
						      sc->a, sc->d);
	       }
	     ep->gradient_id = p3->gradient.id;
	  }
     }
}

static int
_edje_nitoa_rec(char *string, int len, unsigned int value)
{
   const char	*array = "0123456789";
   int		 length;
   int		 quotient;
   int		 modulo;

   if (len <= 0) return 0;
   if (value == 0) return 0;

   quotient = value / 10;
   modulo = value % 10;

   length = _edje_nitoa_rec(string, len - 1, quotient);

   if (length + 1 > len) return length;

   string[length] = array[modulo];

   return length + 1;
}

static int
_edje_nitoa(char *string, int len, int value)
{
   int	length;

   if (len <= 0) return 0;
   if (len == 1)
     {
	*string = '\0';
	return 1;
     }

   if (value < 0)
     {
	*string = '-';

	++string;
	--len;
     }

   if (value == 0)
     {
	strncpy(string, "0", len);
	return 1;
     }

   length = _edje_nitoa_rec(string, len, value);
   string[length] = '\0';

   return length;
}

static void
_edje_image_recalc_apply(Edje *ed, Edje_Real_Part *ep, Edje_Calc_Params *p3, Edje_Part_Description *chosen_desc, double pos)
{
   char buf[4096];
   int image_id;
   int image_count, image_num;

   evas_object_image_fill_set(ep->object, p3->fill.x, p3->fill.y,
			      p3->fill.w, p3->fill.h);
   evas_object_image_smooth_scale_set(ep->object, p3->smooth);

   evas_object_image_border_set(ep->object, p3->border.l, p3->border.r,
				p3->border.t, p3->border.b);
   evas_object_image_border_center_fill_set(ep->object,
					    !(chosen_desc->border.no_fill));
   image_id = ep->param1.description->image.id;
   if (image_id < 0)
     {
	Edje_Image_Directory_Entry *ie;

	if (!ed->file->image_dir) ie = NULL;
	else ie = evas_list_nth(ed->file->image_dir->entries, (-image_id) - 1);
	if ((ie) &&
	    (ie->source_type == EDJE_IMAGE_SOURCE_TYPE_EXTERNAL) &&
	    (ie->entry))
	  {
	     evas_object_image_file_set(ep->object, ie->entry, NULL);
	  }
     }
   else
     {
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

	     imid = evas_list_nth(ep->param2.description->image.tween_list,
				  image_num - 1);
	     if (imid) image_id = imid->id;
	  }
	if (image_id < 0)
	  {
	     printf("EDJE ERROR: part \"%s\" has description, "
		    "\"%s\" %3.3f with a missing image id!!!\n",
		    ep->part->name,
		    ep->param1.description->state.name,
		    ep->param1.description->state.value);
	  }
	else
	  {
	     /* Replace snprint("images/%i") == memcpy + itoa */
#define IMAGES "images/"
	     memcpy(buf, IMAGES, strlen(IMAGES));
	     _edje_nitoa(buf + strlen(IMAGES), sizeof(buf) - strlen(IMAGES), image_id);

	     evas_object_image_file_set(ep->object, ed->file->path, buf);
	     if (evas_object_image_load_error_get(ep->object) != EVAS_LOAD_ERROR_NONE)
	       {
		  printf("EDJE: Error loading image collection \"%s\" from "
			 "file \"%s\". Missing EET Evas loader module?\n",
			 buf, ed->file->path);
		  switch (evas_object_image_load_error_get(ep->object))
		    {
		     case EVAS_LOAD_ERROR_GENERIC:
			printf("Error type: EVAS_LOAD_ERROR_GENERIC\n");
			break;
		     case EVAS_LOAD_ERROR_DOES_NOT_EXIST:
			printf("Error type: EVAS_LOAD_ERROR_DOES_NOT_EXIST\n");
			break;
		     case EVAS_LOAD_ERROR_PERMISSION_DENIED:
			printf("Error type: EVAS_LOAD_ERROR_PERMISSION_DENIED\n");
			break;
		     case EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED:
			printf("Error type: EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED\n");
			break;
		     case EVAS_LOAD_ERROR_CORRUPT_FILE:
			printf("Error type: EVAS_LOAD_ERROR_CORRUPT_FILE\n");
			break;
		     case EVAS_LOAD_ERROR_UNKNOWN_FORMAT:
			printf("Error type: EVAS_LOAD_ERROR_UNKNOWN_FORMAT\n");
			break;
		    }
	       }
	  }
     }
}


static void
_edje_part_recalc(Edje *ed, Edje_Real_Part *ep, int flags)
{
   Edje_Calc_Params p1, p2, p3, *pf;
   Edje_Part_Description *chosen_desc;
   double pos = 0.0;

   if ((ep->calculated & FLAG_XY) == FLAG_XY)
     {
	return;
     }
   if (ep->calculating & flags)
     {
#if 1
	const char *axes = "NONE", *faxes = "NONE";

	if ((ep->calculating & FLAG_X) &&
	    (ep->calculating & FLAG_Y))
	  axes = "XY";
	else if ((ep->calculating & FLAG_X))
	  axes = "X";
	else if ((ep->calculating & FLAG_Y))
	  axes = "Y";

	if ((flags & FLAG_X) &&
	    (flags & FLAG_Y))
	  faxes = "XY";
	else if ((flags & FLAG_X))
	  faxes = "X";
	else if ((flags & FLAG_Y))
	  faxes = "Y";
	printf("EDJE ERROR: Circular dependency when calculating part \"%s\"\n"
	       "            Already calculating %s [%02x] axes\n"
	       "            Need to calculate %s [%02x] axes\n",
	       ep->part->name,
	       axes, ep->calculating,
	       faxes, flags);
#endif
	return;
     }
   if (flags & FLAG_X)
     {
	ep->calculating |= flags & FLAG_X;
	if (ep->param1.rel1_to_x)  _edje_part_recalc(ed, ep->param1.rel1_to_x, FLAG_X);
	if (ep->param1.rel2_to_x)  _edje_part_recalc(ed, ep->param1.rel2_to_x, FLAG_X);
	if (ep->param2.rel1_to_x)  _edje_part_recalc(ed, ep->param2.rel1_to_x, FLAG_X);
	if (ep->param2.rel2_to_x)  _edje_part_recalc(ed, ep->param2.rel2_to_x, FLAG_X);
     }
   if (flags & FLAG_Y)
     {
	ep->calculating |= flags & FLAG_Y;
	if (ep->param1.rel1_to_y)  _edje_part_recalc(ed, ep->param1.rel1_to_y, FLAG_Y);
	if (ep->param1.rel2_to_y)  _edje_part_recalc(ed, ep->param1.rel2_to_y, FLAG_Y);
	if (ep->param2.rel1_to_y)  _edje_part_recalc(ed, ep->param2.rel1_to_y, FLAG_Y);
	if (ep->param2.rel2_to_y)  _edje_part_recalc(ed, ep->param2.rel2_to_y, FLAG_Y);
     }
   if (ep->confine_to)        _edje_part_recalc(ed, ep->confine_to, flags);
//   if (ep->text.source)       _edje_part_recalc(ed, ep->text.source, flags);
//   if (ep->text.text_source)  _edje_part_recalc(ed, ep->text.text_source, flags);

   /* actually calculate now */
   chosen_desc = ep->chosen_description;
   if (!chosen_desc)
     {
	ep->calculating = FLAG_NONE;
	ep->calculated |= flags;
	return;
     }
   if (ep->param1.description)
     _edje_part_recalc_single(ed, ep, ep->param1.description, chosen_desc, ep->param1.rel1_to_x, ep->param1.rel1_to_y, ep->param1.rel2_to_x, ep->param1.rel2_to_y, ep->confine_to, &p1, flags);
   if (ep->param2.description)
     {
	int beginning_pos, part_type;

	_edje_part_recalc_single(ed, ep, ep->param2.description, chosen_desc, ep->param2.rel1_to_x, ep->param2.rel1_to_y, ep->param2.rel2_to_x, ep->param2.rel2_to_y, ep->confine_to, &p2, flags);

	pos = ep->description_pos;
	beginning_pos = (pos < 0.5);
	part_type = ep->part->type;

	/* visible is special */
	if ((p1.visible) && (!p2.visible))
	  p3.visible = (pos != 1.0);
	else if ((!p1.visible) && (p2.visible))
	  p3.visible = (pos != 0.0);
	else
	  p3.visible = p1.visible;

	p3.smooth = (beginning_pos) ? p1.smooth : p2.smooth;

	/* FIXME: do x and y separately base on flag */
#define INTP(_x1, _x2, _p) (((_x1) == (_x2)) ? (_x1) : ((_x1) + (((_x2) - (_x1)) * (_p))))
	p3.x = INTP(p1.x, p2.x, pos);
	p3.y = INTP(p1.y, p2.y, pos);
	p3.w = INTP(p1.w, p2.w, pos);
	p3.h = INTP(p1.h, p2.h, pos);

	p3.req.x = INTP(p1.req.x, p2.req.x, pos);
	p3.req.y = INTP(p1.req.y, p2.req.y, pos);
	p3.req.w = INTP(p1.req.w, p2.req.w, pos);
	p3.req.h = INTP(p1.req.h, p2.req.h, pos);

	if (ep->part->dragable.x)
	  {
	     p3.req_drag.x = INTP(p1.req_drag.x, p2.req_drag.x, pos);
	     p3.req_drag.w = INTP(p1.req_drag.w, p2.req_drag.w, pos);
	  }
	if (ep->part->dragable.y)
	  {
	     p3.req_drag.y = INTP(p1.req_drag.y, p2.req_drag.y, pos);
	     p3.req_drag.h = INTP(p1.req_drag.h, p2.req_drag.h, pos);
	  }

	p3.color.r = INTP(p1.color.r, p2.color.r, pos);
	p3.color.g = INTP(p1.color.g, p2.color.g, pos);
	p3.color.b = INTP(p1.color.b, p2.color.b, pos);
	p3.color.a = INTP(p1.color.a, p2.color.a, pos);

	switch (part_type)
	  {
	   case EDJE_PART_TYPE_IMAGE:
	   case EDJE_PART_TYPE_GRADIENT:
	      p3.fill.x = INTP(p1.fill.x, p2.fill.x, pos);
	      p3.fill.y = INTP(p1.fill.y, p2.fill.y, pos);
	      p3.fill.w = INTP(p1.fill.w, p2.fill.w, pos);
	      p3.fill.h = INTP(p1.fill.h, p2.fill.h, pos);
	      if (part_type == EDJE_PART_TYPE_GRADIENT)
		{
		   p3.fill.angle = INTP(p1.fill.angle, p2.fill.angle, pos);
		   p3.fill.spread = (beginning_pos) ? p1.fill.spread : p2.fill.spread;
		   p3.gradient = (beginning_pos) ? p1.gradient : p2.gradient;
		}
	      else
		{
		   p3.border.l = INTP(p1.border.l, p2.border.l, pos);
		   p3.border.r = INTP(p1.border.r, p2.border.r, pos);
		   p3.border.t = INTP(p1.border.t, p2.border.t, pos);
		   p3.border.b = INTP(p1.border.b, p2.border.b, pos);
		}
	      break;
	   case EDJE_PART_TYPE_TEXT:
	   case EDJE_PART_TYPE_TEXTBLOCK:
	      p3.color2.r = INTP(p1.color2.r, p2.color2.r, pos);
	      p3.color2.g = INTP(p1.color2.g, p2.color2.g, pos);
	      p3.color2.b = INTP(p1.color2.b, p2.color2.b, pos);
	      p3.color2.a = INTP(p1.color2.a, p2.color2.a, pos);

	      p3.color3.r = INTP(p1.color3.r, p2.color3.r, pos);
	      p3.color3.g = INTP(p1.color3.g, p2.color3.g, pos);
	      p3.color3.b = INTP(p1.color3.b, p2.color3.b, pos);
	      p3.color3.a = INTP(p1.color3.a, p2.color3.a, pos);

	      p3.text.align.x = INTP(p1.text.align.x, p2.text.align.x, pos);
	      p3.text.align.y = INTP(p1.text.align.y, p2.text.align.y, pos);
	      p3.text.elipsis = INTP(p1.text.elipsis, p2.text.elipsis, pos);
	      break;
	  }

	pf = &p3;
     }
   else
     {
	pf = &p1;
     }

   ep->req = pf->req;

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
	/* Common move, resize and color_set for all part. */
	switch (ep->part->type)
	  {
	   case EDJE_PART_TYPE_RECTANGLE:
	   case EDJE_PART_TYPE_IMAGE:
	   case EDJE_PART_TYPE_TEXTBLOCK:
	   case EDJE_PART_TYPE_GRADIENT:
	      evas_object_color_set(ep->object,
				    (pf->color.r * pf->color.a) / 255,
				    (pf->color.g * pf->color.a) / 255,
				    (pf->color.b * pf->color.a) / 255,
				    pf->color.a);
	      if (!pf->visible)
		{
		   evas_object_hide(ep->object);
		   break;
		}
	      evas_object_show(ep->object);
	      /* move and resize are needed for all previous object => no break here. */
	   case EDJE_PART_TYPE_SWALLOW:
	   case EDJE_PART_TYPE_GROUP:
	      /* visibility and color have no meaning on SWALLOW and GROUP part. */
	      evas_object_move(ep->object, ed->x + pf->x, ed->y + pf->y);
	      evas_object_resize(ep->object, pf->w, pf->h);
	      if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
	        _edje_entry_real_part_configure(ep);
	      break;
	   case EDJE_PART_TYPE_TEXT:
	      /* This is correctly handle in _edje_text_recalc_apply at the moment. */
	      break;
	  }

	/* Some object need special recalc. */
	switch (ep->part->type)
	  {
	   case EDJE_PART_TYPE_TEXT:
	      _edje_text_recalc_apply(ed, ep, pf, chosen_desc);
	      break;
	   case EDJE_PART_TYPE_IMAGE:
	      _edje_image_recalc_apply(ed, ep, pf, chosen_desc, pos);
	      break;
	   case EDJE_PART_TYPE_GRADIENT:
	      _edje_gradient_recalc_apply(ed, ep, pf, chosen_desc);
	      break;
	   case EDJE_PART_TYPE_RECTANGLE:
	   case EDJE_PART_TYPE_SWALLOW:
	   case EDJE_PART_TYPE_GROUP:
	   case EDJE_PART_TYPE_TEXTBLOCK:
	      /* Nothing special to do for this type of object. */
	      break;
	  }

	if (ep->swallowed_object)
	  {
//// the below really is wrong - swallow color shouldnt affect swallowed object
//// color - the edje color as a WHOLE should though - and that should be
//// done via the clipper anyway. this created bugs when objects had their
//// colro set and were swallowed - then had their color changed.
//	     evas_object_color_set(ep->swallowed_object,
//				   (pf->color.r * pf->color.a) / 255,
//				   (pf->color.g * pf->color.a) / 255,
//				   (pf->color.b * pf->color.a) / 255,
//				   pf->color.a);
	     if (pf->visible)
	       {
		  evas_object_move(ep->swallowed_object, ed->x + pf->x, ed->y + pf->y);
		  evas_object_resize(ep->swallowed_object, pf->w, pf->h);
		  evas_object_show(ep->swallowed_object);
	       }
	     else
	       evas_object_hide(ep->swallowed_object);
	  }
     }

   ep->x = pf->x;
   ep->y = pf->y;
   ep->w = pf->w;
   ep->h = pf->h;

   ep->calculated |= flags;
   ep->calculating = FLAG_NONE;
}
