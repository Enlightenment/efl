/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Edje.h"
#include "edje_private.h"

#ifdef HAVE_STDINT_H
#include <stdint.h> 
#endif
#include <assert.h>

void
_edje_text_init(void)
{
}

void
_edje_text_part_on_add(Edje *ed, Edje_Real_Part *ep)
{
   Evas_List *tmp;
   Edje_Part *pt = ep->part;

   if (ep->part->type != EDJE_PART_TYPE_TEXT) return;

   /* if text class exists for this part, add the edje to the tc member list */
   if ((pt->default_desc) && (pt->default_desc->text.text_class))
     _edje_text_class_member_add(ed, pt->default_desc->text.text_class);
   
   /* If any other classes exist add them */
   for (tmp = pt->other_desc; tmp; tmp = tmp->next)
     {
        Edje_Part_Description *desc;

	desc = tmp->data;
	if ((desc) && (desc->text.text_class)) 
	  _edje_text_class_member_add(ed, desc->text.text_class);
     }
}

void
_edje_text_part_on_add_clippers(Edje *ed, Edje_Real_Part *ep)
{
   Evas_List *l;
   
   for (l = ep->extra_objects; l; l = l->next)
     {
	Evas_Object *o;
	
	o = l->data;
	if (ep->part->clip_to_id >= 0)
	  {
	     ep->clip_to = evas_list_nth(ed->parts, ep->part->clip_to_id);
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
   Evas_List *tmp;

   if ((pt->default_desc) && (pt->default_desc->text.text_class))
     _edje_text_class_member_del(ed, pt->default_desc->text.text_class);
  
   for (tmp = pt->other_desc; tmp; tmp = tmp->next)
     {
	 Edje_Part_Description *desc;

	 desc = tmp->data;
	 if (desc->text.text_class)
	   _edje_text_class_member_del(ed, desc->text.text_class);
     }
}

void
_edje_text_real_part_on_del(Edje *ed, Edje_Real_Part *ep)
{
   while (ep->extra_objects)
     {
	Evas_Object *o;
	
	o = ep->extra_objects->data;
	ep->extra_objects = evas_list_remove(ep->extra_objects, o);
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
   char *buf;
   int c1 = -1, c2 = -1, loop = 0, extra;
   size_t orig_len;

   *free_text = 0;

   evas_object_text_font_set(ep->object, font, size);
   evas_object_text_text_set(ep->object, text);

   evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);

   p = ((sw - tw) * params->text.elipsis);

   /* chop chop */
   if (tw > sw)
     {
	if (params->text.elipsis != 0.0)
	  c1 = evas_object_text_char_coords_get(ep->object,
		-p, th / 2,
		NULL, NULL, NULL, NULL);
	if (params->text.elipsis != 1.0)
	  c2 = evas_object_text_char_coords_get(ep->object,
		-p + sw, th / 2,
		NULL, NULL, NULL, NULL);
	if ((c1 < 0) && (c2 < 0))
	  {
	     c1 = 0;
	     c2 = 0;
	  }
     }

   if (!((c1 >= 0 || c2 >= 0) && (tw > sw)))
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
	evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
     }

   *free_text = 1;

   return buf;
}

void
_edje_text_recalc_apply(Edje *ed, Edje_Real_Part *ep,
			Edje_Calc_Params *params,
			Edje_Part_Description *chosen_desc)
{
   const char	*text;
   const char	*font;
   char		*font2 = NULL;
   int		 size;
   Evas_Coord	 tw, th;
   Evas_Coord	 sw, sh;
   char		 font_buf[4096];
   int		 inlined_font = 0, free_text = 0;
   

   text = chosen_desc->text.text;
   font = chosen_desc->text.font;
   size = chosen_desc->text.size;

   if ((chosen_desc->text.text_class) && (chosen_desc->text.text_class[0] != 0))
     {
	Edje_Text_Class *tc;
	
	tc = _edje_text_class_find(ed, chosen_desc->text.text_class);
	if (tc)
	  {
	     if (tc->font) font = tc->font;
	     if (tc->size > 0) size = tc->size;
	  }
     }
   
   if (ep->text.text) text = (char *) ep->text.text;
   if (ep->text.font) font = ep->text.font;
   if (ep->text.size > 0) size = ep->text.size;

   if (!text) text = "";
   if (!font) font = "";
   
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
		  strcpy(font_buf, "fonts/");
		  strncpy(font_buf + 6, font, sizeof(font_buf) - 7);
		  font_buf[sizeof(font_buf) - 1] = 0;
		  font = font_buf;
		  inlined_font = 1;
		  break;
	       }
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
   if ((ep->text.cache.in_size == size) &&
       (ep->text.cache.in_w == sw) &&
       (ep->text.cache.in_h == sh) &&
       (ep->text.cache.in_str) &&
       (text) &&
       (!strcmp(ep->text.cache.in_str, text)) &&
       (ep->text.cache.align_x == params->text.align.x) &&
       (ep->text.cache.align_y == params->text.align.y) &&
       (ep->text.cache.elipsis == params->text.elipsis))
     {
	text = (char *) ep->text.cache.out_str;
	size = ep->text.cache.out_size;
	
	if (!text) text = "";
   
	goto arrange_text;
     }
   if (ep->text.cache.in_str) evas_stringshare_del(ep->text.cache.in_str);
   ep->text.cache.in_str = evas_stringshare_add(text);
   ep->text.cache.in_size = size;
   if (chosen_desc->text.fit_x)
     {
        if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
	else evas_object_text_font_source_set(ep->object, NULL);

	evas_object_text_font_set(ep->object, font, size);
	evas_object_text_text_set(ep->object, text);
	evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
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
		  
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
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
		  
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
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
	
	evas_object_text_font_set(ep->object, font, size);
	evas_object_text_text_set(ep->object, text);
	evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);

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
		  
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
		  if ((size > 0) && (th == 0)) break;
	       }
	     size -= dif;
	  }
	else if (th > sh)
	  {
	     int dif;
	     
	     dif = (th - sh) / 4;
	     if (dif < 1) dif = 1;
	     while ((th > sh) && (sw >= 0.0))
	       {
		  size -= dif;
		  if (size <= 0) break;
		  if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
		  else evas_object_text_font_source_set(ep->object, NULL);
		  
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
		  if ((size > 0) && (th == 0)) break;
	       }
	  }
     }
   if (size < 1) size = 1;

   if (!chosen_desc->text.fit_x)
     {
	if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
	else evas_object_text_font_source_set(ep->object, NULL);

	text = _edje_text_fit_x(ed, ep, params, text, font, size, sw, &free_text);
     }

   if (ep->text.cache.out_str) evas_stringshare_del(ep->text.cache.out_str);
   ep->text.cache.out_str = evas_stringshare_add(text);
   ep->text.cache.in_w = sw;
   ep->text.cache.in_h = sh;
   ep->text.cache.out_size = size;
   ep->text.cache.align_x = params->text.align.x;
   ep->text.cache.align_y = params->text.align.y;
   ep->text.cache.elipsis = params->text.elipsis;
   arrange_text:
   
   if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
   else evas_object_text_font_source_set(ep->object, NULL);
   
   evas_object_text_font_set(ep->object, font, size);
   evas_object_text_text_set(ep->object, text);
   evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
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
			      params->color.r,
			      params->color.g,
			      params->color.b,
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
					        params->color2.r,
						params->color2.g,
						params->color2.b,
						params->color2.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_SOFT_OUTLINE)
	  {
	     style = EVAS_TEXT_STYLE_SOFT_OUTLINE;
	     evas_object_text_outline_color_set(ep->object,
					        params->color2.r,
						params->color2.g,
						params->color2.b,
						params->color2.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_SHADOW;
	     evas_object_text_shadow_color_set(ep->object,
					       params->color3.r,
					       params->color3.g,
					       params->color3.b,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_SOFT_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_SOFT_SHADOW;
	     evas_object_text_shadow_color_set(ep->object,
					       params->color3.r,
					       params->color3.g,
					       params->color3.b,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_OUTLINE_SHADOW;
	     evas_object_text_outline_color_set(ep->object,
					        params->color2.r,
					        params->color2.g,
					        params->color2.b,
					        params->color2.a);
	     evas_object_text_shadow_color_set(ep->object,
					       params->color3.r,
					       params->color3.g,
					       params->color3.b,
					       params->color3.a);
	  }
	else if (ep->part->effect == EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW)
	  {
	     style = EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW;
	     evas_object_text_outline_color_set(ep->object,
					        params->color2.r,
					        params->color2.g,
					        params->color2.b,
					        params->color2.a);
	     evas_object_text_shadow_color_set(ep->object,
					       params->color3.r,
					       params->color3.g,
					       params->color3.b,
					       params->color3.a);
	  }
	evas_object_text_style_set(ep->object, style);
     }

   if (free_text)
     free((char *)text);
   if (font2)
     free(font2);
}
