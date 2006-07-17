/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Edje.h"
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
	ep->calculated = FLAG_NONE;
	ep->calculating = FLAG_NONE;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
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
   if (ep->drag.x != x)
     {
	ep->drag.x = x;
	ed->dirty = 1;
     }

   if (ep->drag.y != y)
     {
	ep->drag.y = y;
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
	double aspect;
	double new_w, new_h, want_x, want_y, want_w, want_h;
   
	want_x = params->x;
	want_w = new_w = params->w;
	
	want_y = params->y;
	want_h = new_h = params->h;
	
	aspect = (double)params->w / (double)params->h;
	if (desc->aspect.prefer == EDJE_ASPECT_PREFER_NONE) /* keep both dimensions in check */
	  {
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
	  } /* prefer vertical size as determiner */
	else if (desc->aspect.prefer == EDJE_ASPECT_PREFER_VERTICAL) /* keep both dimensions in check */
	  {
	     /* adjust for max aspect (width / height) */
	     if ((desc->aspect.max > 0.0) && (aspect > desc->aspect.max))
	       new_w = (params->h * desc->aspect.max);
	     /* adjust for min aspect (width / height) */
	     if ((desc->aspect.min > 0.0) && (aspect < desc->aspect.min))
	       new_w = (params->h * desc->aspect.min);
	  } /* prefer horizontal size as determiner */
	else if (desc->aspect.prefer == EDJE_ASPECT_PREFER_HORIZONTAL) /* keep both dimensions in check */
	  {
	     /* adjust for max aspect (width / height) */
	     if ((desc->aspect.max > 0.0) && (aspect > desc->aspect.max))
	       new_h = (params->w / desc->aspect.max);
	     /* adjust for min aspect (width / height) */
	     if ((desc->aspect.min > 0.0) && (aspect < desc->aspect.min))
	       new_h = (params->w / desc->aspect.min);
	  }
	else if (desc->aspect.prefer == EDJE_ASPECT_PREFER_BOTH) /* keep both dimensions in check */
	  {
	     /* adjust for max aspect (width / height) */
	     if ((desc->aspect.max > 0.0) && (aspect > desc->aspect.max))
	       {
		  new_w = (params->h * desc->aspect.max);
		  new_h = (params->w / desc->aspect.max);
	       }
	     /* adjust for min aspect (width / height) */
	     if ((desc->aspect.min > 0.0) && (aspect < desc->aspect.min))
	       {
		  new_w = (params->h * desc->aspect.min);
		  new_h = (params->w / desc->aspect.min);
	       }
	  }
	/* do real adjustment */
	if (desc->aspect.prefer == EDJE_ASPECT_PREFER_BOTH)
	  {
	     /* fix h and vary w */
	     if (new_w > params->w)
	       params->w = new_w;
	     /* fix w and vary h */
	     else
	       params->h = new_h;
	  }
	else
	  {
	     if ((params->h - new_h) > (params->w - new_w))
	       {
		  if (params->h < new_h)
		    params->h = new_h;
		  else if (params->h > new_h)
		    params->h = new_h;
		  if (desc->aspect.prefer == EDJE_ASPECT_PREFER_VERTICAL)
		    params->w = new_w;
	       }
	     else
	       {
		  if (params->w < new_w)
		    params->w = new_w;
		  else if (params->w > new_w)
		    params->w = new_w;
		  if (desc->aspect.prefer == EDJE_ASPECT_PREFER_HORIZONTAL)
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
   if (flags & FLAG_X)
     {
	minw = desc->min.w;
	if (ep->swallow_params.min.w > desc->min.w) minw = ep->swallow_params.min.w;
     }
   if (flags & FLAG_Y)
     {
	minh = desc->min.h;
	if (ep->swallow_params.min.h > desc->min.h) minh = ep->swallow_params.min.h;
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
	     style = ep->text.source->chosen_description->text.style;
	  }
	else
	  {
	     ep->text.source = NULL;
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
	
	if (stl)
	  {
	     const char *ptxt;
	     
	     if (evas_object_textblock_style_get(ep->object) != stl->style)
	       evas_object_textblock_style_set(ep->object, stl->style);
	     ptxt = evas_object_textblock_text_markup_get(ep->object);
	     if (((!ptxt) && (text)) || 
		 ((ptxt) && (text) && (strcmp(ptxt, text))) ||
		 ((ptxt) && (!text)))
	       evas_object_textblock_text_markup_set(ep->object, text);
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
     }
   else if ((chosen_desc) && (ep->part->type == EDJE_PART_TYPE_TEXT))
     {
	const char	*text;
	const char	*font;
	int		 size;
	Evas_Coord	 tw, th;
	char		 buf[4096];
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
	  {
	     font = ep->text.source->chosen_description->text.font;
	     size = ep->text.source->chosen_description->text.size;
	  }
	else
	  {
	     font = chosen_desc->text.font;
	     size = chosen_desc->text.size;
	  }
	if (ep->text.source)
	  {
	     if ((ep->text.source->chosen_description->text.text_class) && 
		 (*ep->text.source->chosen_description->text.text_class))
	       {
		  Edje_Text_Class *tc;
		  
		  tc = _edje_text_class_find(ed, ep->text.source->chosen_description->text.text_class);
		  if (tc)
		    {
		       if (tc->font) font = tc->font;
		       if (tc->size > 0) size = tc->size;
		    }
	       }
	  }
	else
	  {
	     if ((chosen_desc->text.text_class) && (*chosen_desc->text.text_class))
	       {
		  Edje_Text_Class *tc;
		  
		  tc = _edje_text_class_find(ed, chosen_desc->text.text_class);
		  if (tc)
		    {
		       if (tc->font) font = tc->font;
		       if (tc->size > 0) size = tc->size;
		    }
	       }
	  }
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
        /* FIXME: we should cache this result */
        if (ed->file->font_dir)
	  {
	     Evas_List *l;
	     
	     for (l = ed->file->font_dir->entries; l; l = l->next)
	       {
		  Edje_Font_Directory_Entry *fnt = l->data;

		  if ((fnt->entry) && (!strcmp(fnt->entry, font)))
		    {
		       strcpy(buf, "fonts/");
		       strncpy(buf + 6, font, sizeof(buf) - 7);
		       buf[sizeof(buf) - 1] = 0;
		       font = buf;
		       inlined_font = 1;
		       break;
		    }
	       }
	  }
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
	if ((chosen_desc->text.min_x) || (chosen_desc->text.min_y))
	  {
	     int mw, mh;
	     Evas_Text_Style_Type style;
	     
	     style = EVAS_TEXT_STYLE_PLAIN;
	     if ((ep->part->effect == EDJE_TEXT_EFFECT_NONE) ||
		 (ep->part->effect == EDJE_TEXT_EFFECT_PLAIN))
	       style = EVAS_TEXT_STYLE_PLAIN;
	     else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE)
	       style = EVAS_TEXT_STYLE_OUTLINE;
	     else if (ep->part->effect == EDJE_TEXT_EFFECT_SOFT_OUTLINE)
	       style = EVAS_TEXT_STYLE_SOFT_OUTLINE;
	     else if (ep->part->effect == EDJE_TEXT_EFFECT_SHADOW)
	       style = EVAS_TEXT_STYLE_SHADOW;
	     else if (ep->part->effect == EDJE_TEXT_EFFECT_SOFT_SHADOW)
	       style = EVAS_TEXT_STYLE_SOFT_SHADOW;
	     else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE_SHADOW)
	       style = EVAS_TEXT_STYLE_OUTLINE_SHADOW;
	     else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW)
	       style = EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW;
	     evas_object_text_style_set(ep->object, style);
	     evas_object_text_text_set(ep->object, text);
	     evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
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
   if (flags & FLAG_X)
     {
	maxw = desc->max.w;
	if ((ep->swallow_params.max.w >= 0) &&
	    (ep->swallow_params.max.w < maxw)) maxw = ep->swallow_params.max.w;
     }
   if (flags & FLAG_Y)
     {
	maxh = desc->max.h;
	if ((ep->swallow_params.max.h >= 0) &&
	    (ep->swallow_params.max.h < maxh)) maxh = ep->swallow_params.max.h;
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
   params->smooth = desc->fill.smooth;
   if (flags & FLAG_X)
     {
	params->fill.x = desc->fill.pos_abs_x + (params->w * desc->fill.pos_rel_x);
	params->fill.w = desc->fill.abs_x + (params->w * desc->fill.rel_x);
     }
   if (flags & FLAG_Y)
     {
	params->fill.y = desc->fill.pos_abs_y + (params->h * desc->fill.pos_rel_y);
	params->fill.h = desc->fill.abs_y + (params->h * desc->fill.rel_y);
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
}

static void
_edje_part_recalc(Edje *ed, Edje_Real_Part *ep, int flags)
{
   Edje_Calc_Params p1, p2, p3;
   Edje_Part_Description *chosen_desc;
   double pos = 0.0;

   if ((ep->calculated & FLAG_XY) == FLAG_XY)
     {
	return;
     }
   if (ep->calculating & flags)
     {
#if 1
	char *axes = "NONE", *faxes = "NONE";
	
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
	_edje_part_recalc_single(ed, ep, ep->param2.description, chosen_desc, ep->param2.rel1_to_x, ep->param2.rel1_to_y, ep->param2.rel2_to_x, ep->param2.rel2_to_y, ep->confine_to, &p2, flags);

	pos = ep->description_pos;
	
	/* visible is special */
	if ((p1.visible) && (!p2.visible))
	  p3.visible = (pos != 1.0);
	else if ((!p1.visible) && (p2.visible))
	  p3.visible = (pos != 0.0);
	else
	  p3.visible = p1.visible;

	p3.smooth = (pos < 0.5) ? p1.smooth : p2.smooth;

	/* FIXME: do x and y separately base on flag */
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
	p3.text.elipsis = (p1.text.elipsis * (1.0 - pos)) + (p2.text.elipsis * (pos));
     }
   else
     {
	p3 = p1;
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
	     evas_object_image_border_center_fill_set(ep->object, !(chosen_desc->border.no_fill));
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
	     if (image_id < 0)
	       {
		  printf("EDJE ERROR: part \"%s\" has description, \"%s\" %3.3f with a missing image id!!!\n",
			 ep->part->name,
			 ep->param1.description->state.name,
			 ep->param1.description->state.value
			 );
	       }
	     else
	       {
		  snprintf(buf, sizeof(buf), "images/%i", image_id);
		  evas_object_image_file_set(ep->object, ed->file->path, buf);
		  if (evas_object_image_load_error_get(ep->object) != EVAS_LOAD_ERROR_NONE)
		    {
		       printf("EDJE: Error loading image collection \"%s\" from file \"%s\". Missing EET Evas loader module?\n",
			      buf, ed->file->path);
		       if (evas_object_image_load_error_get(ep->object) == EVAS_LOAD_ERROR_GENERIC)
			 printf("Error type: EVAS_LOAD_ERROR_GENERIC\n");
		       else if (evas_object_image_load_error_get(ep->object) == EVAS_LOAD_ERROR_DOES_NOT_EXIST)
			 printf("Error type: EVAS_LOAD_ERROR_DOES_NOT_EXIST\n");
		       else if (evas_object_image_load_error_get(ep->object) == EVAS_LOAD_ERROR_PERMISSION_DENIED)
			 printf("Error type: EVAS_LOAD_ERROR_PERMISSION_DENIED\n");
		       else if (evas_object_image_load_error_get(ep->object) == EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED)
			 printf("Error type: EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED\n");
		       else if (evas_object_image_load_error_get(ep->object) == EVAS_LOAD_ERROR_CORRUPT_FILE)
			 printf("Error type: EVAS_LOAD_ERROR_CORRUPT_FILE\n");
		       else if (evas_object_image_load_error_get(ep->object) == EVAS_LOAD_ERROR_UNKNOWN_FORMAT)
			 printf("Error type: EVAS_LOAD_ERROR_UNKNOWN_FORMAT\n");
		    }
	       }
	     evas_object_color_set(ep->object, p3.color.r, p3.color.g, p3.color.b, p3.color.a);
	     if (p3.visible) evas_object_show(ep->object);
	     else evas_object_hide(ep->object);
	  }
	else if (ep->part->type == EDJE_PART_TYPE_SWALLOW)
	  {
	     evas_object_move(ep->object, ed->x + p3.x, ed->y + p3.y);
	     evas_object_resize(ep->object, p3.w, p3.h);
	  }
	else if (ep->part->type == EDJE_PART_TYPE_TEXTBLOCK)
	  {
	     evas_object_move(ep->object, ed->x + p3.x, ed->y + p3.y);
	     evas_object_resize(ep->object, p3.w, p3.h);
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
   
   ep->calculated |= flags;
   ep->calculating = FLAG_NONE;
}
