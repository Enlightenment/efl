/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <string.h>

#include "edje_private.h"


/* returns with and height for this part.
 *
 * depending on the value of the use_alternate_font_metrics flag, it will
 * either use evas_object_geometry_get() or the _advance_get() functions.
 *
 * The latter is useful if you want to make sure that width and height
 * are the same value for the same number of characters in the text.
 * This usually only makes sense for monospaced fonts.
 *
 * In future changes to this file, you probably should use this wrapper
 * function everywhere instead of calling evas_object_geometry_get()
 * directly.
 */
static inline void
part_get_geometry(Edje_Real_Part *rp, Evas_Coord *w, Evas_Coord *h)
{
   if (!rp->part->use_alternate_font_metrics)
     evas_object_geometry_get(rp->object, NULL, NULL, w, h);
   else
     {
	if (w) *w = evas_object_text_horiz_advance_get(rp->object);
	if (h) *h = evas_object_text_vert_advance_get(rp->object);
     }
}

void
_edje_text_init(void)
{
}

void
_edje_text_part_on_add(Edje *ed, Edje_Real_Part *ep)
{
   Eina_List *tmp;
   Edje_Part *pt = ep->part;
   Edje_Part_Description *desc;

   if (ep->part->type != EDJE_PART_TYPE_TEXT) return;

   /* if text class exists for this part, add the edje to the tc member list */
   if ((pt->default_desc) && (pt->default_desc->text.text_class))
     _edje_text_class_member_add(ed, pt->default_desc->text.text_class);

   /* If any other classes exist add them */
   EINA_LIST_FOREACH(pt->other_desc, tmp, desc)
     if ((desc) && (desc->text.text_class))
       _edje_text_class_member_add(ed, desc->text.text_class);
}

void
_edje_text_part_on_add_clippers(Edje *ed, Edje_Real_Part *ep)
{
   Eina_List *l;
   Evas_Object *o;

   EINA_LIST_FOREACH(ep->extra_objects, l, o)
     {
	if (ep->part->clip_to_id >= 0)
	  {
	     ep->clip_to = ed->table_parts[ep->part->clip_to_id % ed->table_parts_size];
	     if (ep->clip_to)
	       {
		  evas_object_pass_events_set(ep->clip_to->object, 1);
		  evas_object_clip_set(o, ep->clip_to->object);
	       }
	  }
     }
}

void
_edje_text_part_on_del(Edje *ed, Edje_Part *pt)
{
   Eina_List *tmp;
   Edje_Part_Description *desc;

   if ((pt->default_desc) && (pt->default_desc->text.text_class))
     _edje_text_class_member_del(ed, pt->default_desc->text.text_class);

   EINA_LIST_FOREACH(pt->other_desc, tmp, desc)
     if (desc->text.text_class)
       _edje_text_class_member_del(ed, desc->text.text_class);
}

void
_edje_text_real_part_on_del(Edje *ed, Edje_Real_Part *ep)
{
   while (ep->extra_objects)
     {
	Evas_Object *o;

	o = eina_list_data_get(ep->extra_objects);
	ep->extra_objects = eina_list_remove(ep->extra_objects, o);
	evas_object_del(o);
     }
}

static void
_edje_text_fit_set(char *buf, const char *text, int c1, int c2)
{
   /* helper function called from _edje_text_fit_x().
    * note that we can use strcpy()/strcat() safely, the buffer lengths
    * are checked in the caller.
    */

   if (c1 >= 0)
     {
	strcpy(buf, "...");

	if (c2 >= 0)
	  {
	     strncat(buf, text + c1, c2 - c1);
	     strcat(buf, "...");
	  }
	else
	  strcat(buf, text + c1);
     }
   else
     {
	if (c2 >= 0)
	  {
	     strncpy(buf, text, c2);
	     buf[c2] = 0;
	     strcat(buf, "...");
	  }
	else
	  strcpy(buf, text);
     }
}

static const char *
_edje_text_fit_x(Edje *ed, Edje_Real_Part *ep,
                 Edje_Calc_Params *params,
                 const char *text, const char *font, int size,
                 Evas_Coord sw, int *free_text)
{
   Evas_Coord tw = 0, th = 0, p;
   int l, r;
   char *buf;
   int c1 = -1, c2 = -1, loop = 0, extra;
   size_t orig_len;
   double sc;

   sc = ed->scale;
   if (sc == 0.0) sc = _edje_scale;
   
   *free_text = 0;
   if (sw <= 1) return "";

   if (ep->part->scale) evas_object_scale_set(ep->object, sc);
   evas_object_text_font_set(ep->object, font, size);
   evas_object_text_text_set(ep->object, text);

   part_get_geometry(ep, &tw, &th);
   evas_object_text_style_pad_get(ep->object, &l, &r, NULL, NULL);

   p = ((sw - tw) * params->text.elipsis);

   /* chop chop */
   if (tw > sw)
     {
	if (params->text.elipsis != 0.0)
	  c1 = evas_object_text_char_coords_get(ep->object,
		-p + l, th / 2,
		NULL, NULL, NULL, NULL);
	if (params->text.elipsis != 1.0)
	  c2 = evas_object_text_char_coords_get(ep->object,
		-p + sw - r, th / 2,
		NULL, NULL, NULL, NULL);
	if ((c1 < 0) && (c2 < 0))
	  {
	     c1 = 0;
	     c2 = 0;
	  }
     }

   if (!(((c1 >= 0) || (c2 >= 0)) && (tw > sw)))
     return text;

   if ((c1 == 0) && (c2 == 0))
     return text;

   orig_len = strlen(text);

   /* don't overflow orig_len by adding extra
    * FIXME: we might want to set a max string length somewhere...
    */
   extra = 1 + 3 + 3; /* terminator, leading and trailing ellipsis */
   orig_len = MIN(orig_len, 8192 - extra);

   if (!(buf = malloc(orig_len + extra)))
     return text;

   while (((c1 >= 0) || (c2 >= 0)) && (tw > sw))
     {
	loop++;
	if (sw <= 0.0)
	  {
	     buf[0] = 0;
	     break;
	  }
	if ((c1 >= 0) && (c2 >= 0))
	  {
	     if ((loop & 0x1))
	       {
		  if (c1 >= 0)
		    c1 = evas_string_char_next_get(text, c1, NULL);
	       }
	     else
	       {
		  if (c2 >= 0)
		    {
		       c2 = evas_string_char_prev_get(text, c2, NULL);
		       if (c2 < 0)
			 {
			    buf[0] = 0;
			    break;
			 }
		    }
	       }
	  }
	else
	  {
	     if (c1 >= 0)
	       c1 = evas_string_char_next_get(text, c1, NULL);
	     else if (c2 >= 0)
	       {
		  c2 = evas_string_char_prev_get(text, c2, NULL);
		  if (c2 < 0)
		    {
		       buf[0] = 0;
		       break;
		    }
	       }
	  }
	if ((c1 >= 0) && (c2 >= 0))
	  {
	     if (c1 >= c2)
	       {
		  buf[0] = 0;
		  break;
	       }
	  }
	else if ((c1 > 0 && c1 >= orig_len) || c2 == 0)
	  {
	     buf[0] = 0;
	     break;
	  }

	buf[0] = 0;

	_edje_text_fit_set(buf, text, c1, c2);

	evas_object_text_text_set(ep->object, buf);
	part_get_geometry(ep, &tw, &th);
     }

   *free_text = 1;

   return buf;
}

static const char *
_edje_text_font_get(const char *base, const char *new, char **free_later)
{
   const char *base_style, *new_style, *aux;
   int font_len, style_len;

   if (base && (!new))
     return base;
   else if ((!base) && new)
     return new;

   base_style = strstr(base, ":style=");
   if (!base_style)
     return new;

   new_style = strstr(new, ":style=");
   if (new_style)
     return new;

   font_len = strlen(new);
   aux = strchr(base_style, ',');
   style_len = (aux) ?  (aux - base_style) : strlen(base_style);

   *free_later = malloc(font_len + style_len + 1);
   memcpy(*free_later, new, font_len);
   memcpy(*free_later + font_len, base_style, style_len);
   (*free_later)[font_len + style_len] = '\0';

   return *free_later;
}

const char *
_edje_text_class_font_get(Edje *ed, Edje_Part_Description *chosen_desc, int *size, char **free_later)
{
   Edje_Text_Class *tc;
   const char *text_class_name, *font;

   font = chosen_desc->text.font;
   *size = chosen_desc->text.size;

   text_class_name = chosen_desc->text.text_class;
   if ((!text_class_name) || (!text_class_name[0]))
     return font;

   tc = _edje_text_class_find(ed, text_class_name);
   if (!tc)
     return font;

   font = _edje_text_font_get(chosen_desc->text.font, tc->font, free_later);
   *size = _edje_text_size_calc(*size, tc);

   return font;
}

void
_edje_text_recalc_apply(Edje *ed, Edje_Real_Part *ep,
			Edje_Calc_Params *params,
			Edje_Part_Description *chosen_desc)
{
   const char	*text;
   const char	*font;
   char		*font2 = NULL;
   char         *sfont = NULL;
   int		 size;
   Evas_Coord	 tw, th;
   Evas_Coord	 sw, sh;
   int		 inlined_font = 0, free_text = 0;
   double        sc;

   sc = ed->scale;
   if (sc == 0.0) sc = _edje_scale;
   text = chosen_desc->text.text;
   font = _edje_text_class_font_get(ed, chosen_desc, &size, &sfont);

   if (ep->text.text) text = (char *) ep->text.text;
   if (ep->text.font) font = ep->text.font;
   if (ep->text.size > 0) size = ep->text.size;

   if (ep->text.text_source)
     {
	text = ep->text.text_source->chosen_description->text.text;
	if (ep->text.text_source->text.text) text = ep->text.text_source->text.text;
     }
   if (ep->text.source)
     {
	font = ep->text.source->chosen_description->text.font;
	size = ep->text.source->chosen_description->text.size;
	if (ep->text.source->text.font) font = ep->text.source->text.font;
	if (ep->text.source->text.size > 0) size = ep->text.source->text.size;
     }

   if (!text) text = "";
   if (!font) font = "";

   /* check if the font is embedded in the .eet */
   if (ed->file->font_hash)
     {
	Edje_Font_Directory_Entry *fnt = eina_hash_find(ed->file->font_hash, font);

	if (fnt)
	  {
	     font = fnt->path;
	     inlined_font = 1;
	  }
     }

   if ((_edje_fontset_append) && (font))
     {
	font2 = malloc(strlen(font) + 1 + strlen(_edje_fontset_append) + 1);
	if (font2)
	  {
	     strcpy(font2, font);
	     strcat(font2, ",");
	     strcat(font2, _edje_fontset_append);
	     font = font2;
	  }
     }
     {
	int l, r, t, b;

	evas_object_text_style_pad_get(ep->object, &l, &r, &t, &b);
	sw = params->w;
	sh = params->h;
     }

   size = params->text.size;

   if ((ep->text.cache.in_size == size) &&
       (ep->text.cache.in_w == sw) &&
       (ep->text.cache.in_h == sh) &&
       (ep->text.cache.in_str) &&
       (text) &&
       (!strcmp(ep->text.cache.in_str, text)) &&
       (ep->text.cache.align_x == params->text.align.x) &&
       (ep->text.cache.align_y == params->text.align.y) &&
       (ep->text.cache.elipsis == params->text.elipsis) &&
       (ep->text.cache.fit_x == chosen_desc->text.fit_x) &&
       (ep->text.cache.fit_y == chosen_desc->text.fit_y))
     {
	text = (char *)ep->text.cache.out_str;
	size = ep->text.cache.out_size;

	if (!text) text = "";

	goto arrange_text;
     }
   if (ep->text.cache.in_str) eina_stringshare_del(ep->text.cache.in_str);
   ep->text.cache.in_str = eina_stringshare_add(text);
   ep->text.cache.in_size = size;
   if (chosen_desc->text.fit_x)
     {
        if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
	else evas_object_text_font_source_set(ep->object, NULL);

	if (ep->part->scale) evas_object_scale_set(ep->object, sc);
	evas_object_text_font_set(ep->object, font, size);
	evas_object_text_text_set(ep->object, text);
	part_get_geometry(ep, &tw, &th);
	if (tw > sw)
	  {
	     int psize;

	     psize = size;
	     while ((tw > sw) && (size > 0) && (tw != 0))
	       {
		  psize = size;
		  size = (size * sw) / tw;
		  if ((psize - size) <= 0) size = psize - 1;
		  if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
		  else evas_object_text_font_source_set(ep->object, NULL);

		  if (ep->part->scale) evas_object_scale_set(ep->object, sc);
		  evas_object_text_font_set(ep->object, font, size);
		  part_get_geometry(ep, &tw, &th);
		  if ((size > 0) && (tw == 0)) break;
	       }
	  }
	else if (tw < sw)
	  {
	     int psize;

	     psize = size;
	     while ((tw < sw) && (size > 0) && (tw != 0))
	       {
		  psize = size;
		  size = (size * sw) / tw;
		  if ((psize - size) >= 0) size = psize + 1;
		  if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
		  else evas_object_text_font_source_set(ep->object, NULL);

		  if (ep->part->scale) evas_object_scale_set(ep->object, sc);
		  evas_object_text_font_set(ep->object, font, size);
		  part_get_geometry(ep, &tw, &th);
		  if ((size > 0) && (tw == 0)) break;
	       }
	  }
     }
   if (chosen_desc->text.fit_y)
     {
	/* if we fit in the x axis, too, size already has a somewhat
	 * meaningful value, so don't overwrite it with the starting
	 * value in that case
	 */
	if (!chosen_desc->text.fit_x) size = sh;

        if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
	else evas_object_text_font_source_set(ep->object, NULL);

	if (ep->part->scale) evas_object_scale_set(ep->object, sc);
	evas_object_text_font_set(ep->object, font, size);
	evas_object_text_text_set(ep->object, text);
	part_get_geometry(ep, &tw, &th);

	/* only grow the font size if we didn't already reach the max size
	 * for the x axis
	 */
	if (!chosen_desc->text.fit_x && th < sh)
	  {
	     int dif;

	     dif = (th - sh) / 4;
	     if (dif < 1) dif = 1;
	     while ((th < sh) && (sw > 0))
	       {
		  size += dif;
		  if (size <= 0) break;
		  if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
		  else evas_object_text_font_source_set(ep->object, NULL);

		  if (ep->part->scale) evas_object_scale_set(ep->object, sc);
		  evas_object_text_font_set(ep->object, font, size);
		  part_get_geometry(ep, &tw, &th);
		  if ((size > 0) && (th == 0)) break;
	       }
	     size -= dif;
	  }
	else if (th > sh)
	  {
	     int current;

	     if (ep->part->scale) evas_object_scale_set(ep->object, sc);
	     evas_object_text_font_set(ep->object, font, 10);
	     part_get_geometry(ep, &tw, &th);

	     if (th == sh)
	       current = 10;
	     else
	       {
		  int bottom, top;

		  if (th < sh)
		    bottom = 10;
		  else if (th > sh)
		    {
		       bottom = 1;
		       top = 10;
		    }
		  else bottom = 0; /* XXX shut up GCC, th == sh is handled before! */

		  top = size;
		  /* search one that fits (binary search) */
		  do
		    {
		       current = (top + bottom) / 2;

		       if (ep->part->scale) evas_object_scale_set(ep->object, sc);
		       evas_object_text_font_set(ep->object, font, current);
		       part_get_geometry(ep, &tw, &th);

		       if      (th < sh) bottom = current + 1;
		       else if (th > sh) top    = current - 1;
		    } while ((bottom < top) && (th != sh));
	       }

	     /* search the larger one that fits (linear search) */
	     do
	       {
		  current++;

		  if (ep->part->scale) evas_object_scale_set(ep->object, sc);
		  evas_object_text_font_set(ep->object, font, current);
		  part_get_geometry(ep, &tw, &th);
	       } while (th <= sh);
	     size = current - 1;
	  }
     }
   if (size < 1) size = 1;

   if (!chosen_desc->text.fit_x)
     {
	if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
	else evas_object_text_font_source_set(ep->object, NULL);

	text = _edje_text_fit_x(ed, ep, params, text, font, size, sw, &free_text);
     }

   if (ep->text.cache.out_str) eina_stringshare_del(ep->text.cache.out_str);
   ep->text.cache.out_str = eina_stringshare_add(text);
   ep->text.cache.in_w = sw;
   ep->text.cache.in_h = sh;
   ep->text.cache.out_size = size;
   ep->text.cache.align_x = params->text.align.x;
   ep->text.cache.align_y = params->text.align.y;
   ep->text.cache.elipsis = params->text.elipsis;
   ep->text.cache.fit_x = chosen_desc->text.fit_x;
   ep->text.cache.fit_y = chosen_desc->text.fit_y;
   arrange_text:

   if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
   else evas_object_text_font_source_set(ep->object, NULL);

   if (ep->part->scale) evas_object_scale_set(ep->object, sc);
   evas_object_text_font_set(ep->object, font, size);
   evas_object_text_text_set(ep->object, text);
   part_get_geometry(ep, &tw, &th);
   ep->offset.x = ((sw - tw) * params->text.align.x);
   ep->offset.y = ((sh - th) * params->text.align.y);

   evas_object_move(ep->object,
		    ed->x + params->x + ep->offset.x,
		    ed->y + params->y + ep->offset.y);

   if (params->visible) evas_object_show(ep->object);
   else evas_object_hide(ep->object);
     {
	Evas_Text_Style_Type style;

	style = EVAS_TEXT_STYLE_PLAIN;

	evas_object_color_set(ep->object,
			      (params->color.r * params->color.a) / 255,
			      (params->color.g * params->color.a) / 255,
			      (params->color.b * params->color.a) / 255,
			      params->color.a);

	if ((ep->part->effect == EDJE_TEXT_EFFECT_NONE) ||
	      (ep->part->effect == EDJE_TEXT_EFFECT_PLAIN))
	  {
	     style = EVAS_TEXT_STYLE_PLAIN;
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE)
	  {
	     style = EVAS_TEXT_STYLE_OUTLINE;
	     evas_object_text_outline_color_set(ep->object,
					        (params->color2.r * params->color2.a) / 255,
					        (params->color2.g * params->color2.a) / 255,
					        (params->color2.b * params->color2.a) / 255,
						params->color2.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_SOFT_OUTLINE)
	  {
	     style = EVAS_TEXT_STYLE_SOFT_OUTLINE;
	     evas_object_text_outline_color_set(ep->object,
						(params->color2.r * params->color2.a) / 255,
						(params->color2.g * params->color2.a) / 255,
						(params->color2.b * params->color2.a) / 255,
						params->color2.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_SHADOW;
	     evas_object_text_shadow_color_set(ep->object,
					       (params->color3.r * params->color3.a) / 255,
					       (params->color3.g * params->color3.a) / 255,
					       (params->color3.b * params->color3.a) / 255,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_SOFT_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_SOFT_SHADOW;
	     evas_object_text_shadow_color_set(ep->object,
					       (params->color3.r * params->color3.a) / 255,
					       (params->color3.g * params->color3.a) / 255,
					       (params->color3.b * params->color3.a) / 255,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_OUTLINE_SHADOW;
	     evas_object_text_outline_color_set(ep->object,
						(params->color2.r * params->color2.a) / 255,
						(params->color2.g * params->color2.a) / 255,
						(params->color2.b * params->color2.a) / 255,
						params->color2.a);
	     evas_object_text_shadow_color_set(ep->object,
					       (params->color3.r * params->color3.a) / 255,
					       (params->color3.g * params->color3.a) / 255,
					       (params->color3.b * params->color3.a) / 255,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW;
	     evas_object_text_outline_color_set(ep->object,
						(params->color2.r * params->color2.a) / 255,
						(params->color2.g * params->color2.a) / 255,
						(params->color2.b * params->color2.a) / 255,
						params->color2.a);
	     evas_object_text_shadow_color_set(ep->object,
					       (params->color3.r * params->color3.a) / 255,
					       (params->color3.g * params->color3.a) / 255,
					       (params->color3.b * params->color3.a) / 255,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_FAR_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_FAR_SHADOW;
	     evas_object_text_shadow_color_set(ep->object,
					       (params->color3.r * params->color3.a) / 255,
					       (params->color3.g * params->color3.a) / 255,
					       (params->color3.b * params->color3.a) / 255,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_FAR_SOFT_SHADOW;
	     evas_object_text_shadow_color_set(ep->object,
					       (params->color3.r * params->color3.a) / 255,
					       (params->color3.g * params->color3.a) / 255,
					       (params->color3.b * params->color3.a) / 255,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_GLOW)
	  {
	     style = EVAS_TEXT_STYLE_GLOW;
	     evas_object_text_glow_color_set(ep->object,
						(params->color2.r * params->color2.a) / 255,
						(params->color2.g * params->color2.a) / 255,
						(params->color2.b * params->color2.a) / 255,
						params->color2.a);
	     evas_object_text_glow2_color_set(ep->object,
					       (params->color3.r * params->color3.a) / 255,
					       (params->color3.g * params->color3.a) / 255,
					       (params->color3.b * params->color3.a) / 255,
					       params->color3.a);
	  }
	evas_object_text_style_set(ep->object, style);
     }

   if (free_text)
     free((char *)text);
   if (font2)
     free(font2);
   if (sfont)
     free(sfont);
}

Evas_Font_Size
_edje_text_size_calc(Evas_Font_Size size, Edje_Text_Class *tc)
{
   int val;

   if (tc->size == 0)
     {
	val = size;
     }
   else if (tc->size > 0.0)
     {
	val = tc->size;
     }
   else
     {
	val = (size * -tc->size) / 100;
     }
   return val;
}
