/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Edje.h"
#include "edje_private.h"

#ifdef HAVE_STDINT_H
#include <stdint.h> 
#endif
#include <assert.h>

Edje_Text_Style _edje_text_styles[EDJE_TEXT_EFFECT_LAST];

void
_edje_text_init(void)
{
   int i, j, n = 0;
   const char vals[5][5] = 
     {
	  {0, 1, 2, 1, 0},
	  {1, 3, 4, 3, 1},
	  {2, 4, 5, 4, 2},
	  {1, 3, 4, 3, 1},
	  {0, 1, 2, 1, 0}
     };
   
   memset(_edje_text_styles, 0, sizeof(_edje_text_styles));
   
   _edje_text_styles[EDJE_TEXT_EFFECT_NONE].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_NONE].members[0].alpha = 255;
   
   _edje_text_styles[EDJE_TEXT_EFFECT_PLAIN].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_PLAIN].members[0].alpha = 255;

   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].offset.x = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].offset.y = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.r = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.b = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].num = 5;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[0].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].y = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].x = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].alpha = 255;
   
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].offset.x = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].offset.y = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.l = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.r = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.t = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.b = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[0].alpha = 255;
   for (j = 0; j < 5; j++)
     {
	for (i = 0; i < 5; i++)
	  {
	     if (((i == 2) && (j == 2)) || (vals[i][j] == 0)) continue;
	     n = _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].num;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].color = 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].x = i - 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].y = j - 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].alpha = vals[i][j] * 50;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].num = n + 1;
	  }
     }
   
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].pad.r = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].pad.b = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].num = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[0].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].color = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].alpha = 255;

   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].offset.x = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].offset.y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.r = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.b = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[0].alpha = 255;
   for (j = 0; j < 5; j++)
     {
	for (i = 0; i < 5; i++)
	  {
	     if (vals[i][j] == 0) continue;
	     n = _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].num;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].color = 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].x = i - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].y = j - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].alpha = vals[i][j] * 50;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].num = n + 1;
	  }
     }

   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].offset.x = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].offset.y = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.r = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.b = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].num = 6;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[0].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].color = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].x = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].y = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].y = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].x = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].alpha = 255;
   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].offset.x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].offset.y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.r = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.b = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[0].alpha = 255;
   for (j = 0; j < 5; j++)
     {
	for (i = 0; i < 5; i++)
	  {
	     if (vals[i][j] == 0) continue;
	     n = _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].color = 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].x = i - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].y = j - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].alpha = vals[i][j] * 50;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num = n + 1;
	  }
     }
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].y = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].x = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num += 4;
}

void
_edje_text_part_on_add(Edje *ed, Edje_Real_Part *ep)
{
   int i;
   Evas_List *tmp;
   Edje_Part *pt = ep->part;

   if (ep->part->type != EDJE_PART_TYPE_TEXT) return;
   if (ep->part->effect >= EDJE_TEXT_EFFECT_LAST) return;
   for (i = 1; i < _edje_text_styles[ep->part->effect].num; i++)
     {
	Evas_Object *o;
	
	o = evas_object_text_add(ed->evas);
//	evas_object_text_font_source_set(o, ed->path);
	evas_object_smart_member_add(o, ed->obj);
	evas_object_pass_events_set(o, 1);
	evas_object_clip_set(o, ed->clipper);
	evas_object_show(o);
	ep->extra_objects = evas_list_append(ep->extra_objects, o);

     }

   if ((pt->default_desc) && (pt->default_desc->text.text_class))
     _edje_text_class_member_add(ed, pt->default_desc->text.text_class);
   for (tmp = pt->other_desc; tmp; tmp = tmp->next)
     {
        Edje_Part_Description *desc;

	desc = tmp->data;
	if ((desc) && (desc->text.text_class)) _edje_text_class_member_add(ed, desc->text.text_class);
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
     {
        _edje_text_class_member_del(ed, pt->default_desc->text.text_class);
	free(pt->default_desc->text.text_class);
	pt->default_desc->text.text_class = NULL;
     }
   
   if (pt->default_desc && pt->default_desc->color_class)
     {
        _edje_color_class_member_del(ed, pt->default_desc->color_class);
	free (pt->default_desc->color_class);
	pt->default_desc->color_class = NULL;
     }

   for (tmp = pt->other_desc; tmp; tmp = tmp->next)
     {
	 Edje_Part_Description *desc;

	 desc = tmp->data;
	 if (desc->text.text_class)
	   {
	      _edje_text_class_member_del(ed, desc->text.text_class);
	      free(desc->text.text_class);
	      desc->text.text_class = NULL;
	   }
	
	if (desc->color_class)
	  {
	     _edje_color_class_member_del(ed, desc->color_class);
	     free(desc->color_class);
	     desc->color_class = NULL;
	  }
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
_edje_text_fit_set(char *buf, char *text, int c1, int c2)
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

static char *
_edje_text_fit_x(Edje *ed, Edje_Real_Part *ep,
                 Edje_Calc_Params *params,
                 char *text, char *font, int size,
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

   p = ((sw - tw) * params->text.align.x);

   /* chop chop */
   if (tw > sw)
     {
	if (params->text.align.x != 0.0)
	  c1 = evas_object_text_char_coords_get(ep->object,
		-p, th / 2,
		NULL, NULL, NULL, NULL);
	if (params->text.align.x != 1.0)
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

static void
_edje_text_finalize(Edje *ed, Edje_Real_Part *ep,
                    Edje_Calc_Params *params, Evas_Object *o, int i)
{
   unsigned char r = 0, g = 0, b = 0, a = 0;
   int got_color = 1;

   evas_object_move(o,
	 ed->x + params->x + ep->offset.x + _edje_text_styles[ep->part->effect].members[i].x,
	 ed->y + params->y + ep->offset.y + _edje_text_styles[ep->part->effect].members[i].y);

   switch (_edje_text_styles[ep->part->effect].members[i].color)
     {
      case 0:
	 r = params->color.r;
	 g = params->color.g;
	 b = params->color.b;
	 a = params->color.a;
	 break;
      case 1:
	 r = params->color2.r;
	 g = params->color2.g;
	 b = params->color2.b;
	 a = params->color2.a;
	 break;
      case 2:
	 r = params->color3.r;
	 g = params->color3.g;
	 b = params->color3.b;
	 a = params->color3.a;
	 break;
      default:
	 got_color = 0;
	 break;
     }

   /* if this fails, we probably need to extend
    * the switch statement :)
    */
   assert (got_color);

   if (got_color)
     {
	a = (a * (_edje_text_styles[ep->part->effect].members[i].alpha + 1)) / 256;
	evas_object_color_set(o, r, g, b, a);
     }

   if (params->visible)
     evas_object_show(o);
   else
     evas_object_hide(o);
}

void
_edje_text_recalc_apply(Edje *ed, Edje_Real_Part *ep,
			Edje_Calc_Params *params,
			Edje_Part_Description *chosen_desc)
{
   char   *text;
   char   *font, *font2 = NULL;
   int     size;
   Evas_Coord  tw, th;
   Evas_Coord  ox, oy, sw, sh;
   char     font_buf[4096];
   int      inlined_font = 0, free_text = 0;
   

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
   
   if (ep->text.text) text = ep->text.text;
   if (ep->text.font) font = ep->text.font;
   if (ep->text.size > 0) size = ep->text.size;

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
   
   ox = _edje_text_styles[ep->part->effect].offset.x;
   oy = _edje_text_styles[ep->part->effect].offset.y;
   sw = params->w - (_edje_text_styles[ep->part->effect].pad.l + _edje_text_styles[ep->part->effect].pad.r);
   sh = params->h - (_edje_text_styles[ep->part->effect].pad.t + _edje_text_styles[ep->part->effect].pad.b);
   if ((ep->text.cache.in_size == size) &&
       (ep->text.cache.in_w == sw) &&
       (ep->text.cache.in_h == sh) &&
       (ep->text.cache.in_str) &&
       (text) &&
       (!strcmp(ep->text.cache.in_str, text)) &&
       (ep->text.cache.align_x == params->text.align.x) &&
       (ep->text.cache.align_y == params->text.align.y))
     {
	text = ep->text.cache.out_str;
	size = ep->text.cache.out_size;
	goto arrange_text;
     }
   if (ep->text.cache.in_str) free(ep->text.cache.in_str);
   ep->text.cache.in_str = strdup(text);
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

	text = _edje_text_fit_x(ed, ep, params, text, font, size, sw,
	                        &free_text);
     }

   if (ep->text.cache.out_str) free(ep->text.cache.out_str);
   ep->text.cache.out_str = strdup(text);
   ep->text.cache.in_w = sw;
   ep->text.cache.in_h = sh;
   ep->text.cache.out_size = size;
   ep->text.cache.align_x = params->text.align.x;
   ep->text.cache.align_y = params->text.align.y;
   arrange_text:
   
   if (inlined_font) evas_object_text_font_source_set(ep->object, ed->path);
   else evas_object_text_font_source_set(ep->object, NULL);
   
   evas_object_text_font_set(ep->object, font, size);
   evas_object_text_text_set(ep->object, text);
   evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
   ep->offset.x = ox + ((sw - tw) * params->text.align.x);
   ep->offset.y = oy + ((sh - th) * params->text.align.y);
   
   evas_object_move(ep->object,
		    ed->x + params->x + ep->offset.x,
		    ed->y + params->y + ep->offset.y);
   evas_object_color_set(ep->object, 
			 params->color.r, 
			 params->color.g, 
			 params->color.b,
			 (params->color.a * (_edje_text_styles[ep->part->effect].members[0].alpha + 1)) / 256);
   if (params->visible) evas_object_show(ep->object);
   else evas_object_hide(ep->object);

     {
	Evas_List *l;
	int i;
	
	for (i = 1, l = ep->extra_objects; l; l = l->next, i++)
	  {
	     Evas_Object *o;
	     
	     o = l->data;
	     if (inlined_font) evas_object_text_font_source_set(o, ed->path);
	     else evas_object_text_font_source_set(o, NULL);
	     
	     evas_object_text_font_set(o, font, size);
	     evas_object_text_text_set(o, text);

	     _edje_text_finalize(ed, ep, params, o, i);
	  }
     }

   if (free_text)
     free(text);
   if (font2)
     free(font2);
}
