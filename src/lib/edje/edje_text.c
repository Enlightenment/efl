#include "edje_private.h"
#define _ELLIP_STR "\xE2\x80\xA6"

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
     eo_do(rp->object, evas_obj_size_get(w, h));
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
   Edje_Part *pt = ep->part;
   Edje_Part_Description_Text *desc;
   unsigned int i;

   if (ep->part->type != EDJE_PART_TYPE_TEXT) return;

   /* if text class exists for this part, add the edje to the tc member list */
   desc = (Edje_Part_Description_Text *) pt->default_desc;
   if ((pt->default_desc) && (desc->text.text_class))
     _edje_text_class_member_add(ed, desc->text.text_class);

   /* If any other classes exist add them */
   for (i = 0; i < pt->other.desc_count; ++i)
     {
        desc = (Edje_Part_Description_Text *) pt->other.desc[i];
        if ((desc) && (desc->text.text_class))
          _edje_text_class_member_add(ed, desc->text.text_class);
     }
}

void
_edje_text_part_on_del(Edje *ed, Edje_Part *pt)
{
   Edje_Part_Description_Text *desc;
   unsigned int i;

   if (!pt) return;
   if (pt->type != EDJE_PART_TYPE_TEXT
       && pt->type != EDJE_PART_TYPE_TEXTBLOCK)
     return;

   desc = (Edje_Part_Description_Text *) pt->default_desc;
   if ((pt->default_desc) && (desc->text.text_class))
     _edje_text_class_member_del(ed, desc->text.text_class);

   for (i = 0; i < pt->other.desc_count; ++i)
     {
        desc = (Edje_Part_Description_Text *) pt->other.desc[i];
        if (desc->text.text_class)
          _edje_text_class_member_del(ed, desc->text.text_class);
     }
}

static const char *
_edje_text_fit_x(Edje *ed, Edje_Real_Part *ep,
                 Edje_Calc_Params *params,
                 Edje_Part_Description_Text *chosen_desc,
                 const char *text, const char *font, int size,
                 Evas_Coord sw, Evas_Coord sh, int *free_text)
{
   FLOAT_T sc;

   sc = DIV(ed->scale, ed->file->base_scale);
   if (sc == ZERO) sc = DIV(_edje_scale, ed->file->base_scale);

   *free_text = 0;
   if (sw <= 1) return "";

   if ((params->type.text.ellipsis < 0) || (chosen_desc->text.min_x))
     return text;

   if (ep->part->scale) evas_object_scale_set(ep->object, TO_DOUBLE(sc));

   eo_do(ep->object,
         evas_obj_text_ellipsis_set(chosen_desc->text.min_x ? -1 : params->type.text.ellipsis),
         efl_interface_text_properties_font_set(font, size),
         evas_obj_text_set(text),
         evas_obj_size_set(sw, sh));

   return text;
}

const char *
_edje_text_font_get(const char *base, const char *new, char **free_later)
{
   const char *base_style, *new_style, *aux;
   size_t font_len, style_len;

   if (base && (!new))
     return base;
   else if (!base)
     return new;
   base_style = strstr(base, ":style=");
   if (!base_style)
     return new;

   new_style = strstr(new, ":style=");
   if (new_style)
     return new;

   font_len = strlen(new);
   aux = strchr(base_style, ',');
   style_len = (aux) ? (size_t)(aux - base_style) : strlen(base_style);

   *free_later = malloc(font_len + style_len + 1);
   memcpy(*free_later, new, font_len);
   memcpy(*free_later + font_len, base_style, style_len);
   (*free_later)[font_len + style_len] = '\0';

   return *free_later;
}

const char *
_edje_text_class_font_get(Edje *ed, Edje_Part_Description_Text *chosen_desc, int *size, char **free_later)
{
   Edje_Text_Class *tc;
   const char *text_class_name, *font;

   font = edje_string_get(&chosen_desc->text.font);
   *size = chosen_desc->text.size;

   text_class_name = chosen_desc->text.text_class;
   if ((!text_class_name) || (!text_class_name[0]))
     return font;

   tc = _edje_text_class_find(ed, text_class_name);
   if (!tc)
     return font;

   font = _edje_text_font_get(edje_string_get(&chosen_desc->text.font), tc->font, free_later);
   *size = _edje_text_size_calc(*size, tc);

   return font;
}

void
_edje_text_recalc_apply(Edje *ed, Edje_Real_Part *ep,
                        Edje_Calc_Params *params,
                        Edje_Part_Description_Text *chosen_desc,
			Eina_Bool calc_only)
{
   const char	*text = NULL;
   const char	*font;
   char		*font2 = NULL;
   char         *sfont = NULL;
   int		 size;
   const char   *filter, *source_name;
   Eina_List    *filter_sources = NULL, *prev_sources = NULL, *li;
   Evas_Coord	 tw, th;
   Evas_Coord	 sw, sh;
   int		 inlined_font = 0, free_text = 0;
   Eina_Bool     same_text = EINA_FALSE;
   FLOAT_T       sc;

   if ((ep->type != EDJE_RP_TYPE_TEXT) ||
       (!ep->typedata.text)) return;
   sc = DIV(ed->scale, ed->file->base_scale);
   if (sc == ZERO) sc = DIV(_edje_scale, ed->file->base_scale);
   text = edje_string_get(&chosen_desc->text.text);
   font = _edje_text_class_font_get(ed, chosen_desc, &size, &sfont);
   filter = chosen_desc->text.filter.str;

   if (ep->typedata.text->text) text = ep->typedata.text->text;
   if (ep->typedata.text->font) font = ep->typedata.text->font;
   if (ep->typedata.text->size > 0) size = ep->typedata.text->size;
   if (ep->typedata.text->filter) filter = ep->typedata.text->filter;
   if (ep->typedata.text->filter_sources != chosen_desc->text.filter_sources)
     {
        prev_sources = ep->typedata.text->filter_sources;
        filter_sources = chosen_desc->text.filter_sources;
        ep->typedata.text->filter_sources = chosen_desc->text.filter_sources;
     }

   if (ep->typedata.text->text_source)
     {
        Edje_Part_Description_Text *et;
        Edje_Real_Part *rp2;

        et = _edje_real_part_text_text_source_description_get(ep, &rp2);
        text = edje_string_get(&(et->text.text));
        if (rp2->typedata.text->text) text = rp2->typedata.text->text;
     }
   if (ep->typedata.text->source)
     {
        Edje_Part_Description_Text *et;
        Edje_Real_Part *rp2;

        et = _edje_real_part_text_source_description_get(ep, &rp2);
        font = edje_string_get(&(et->text.font));
        size = et->text.size;
        if (rp2->typedata.text->font) font = rp2->typedata.text->font;
        if (rp2->typedata.text->size > 0) size = rp2->typedata.text->size;
     }

   if (!text) text = "";
   if (!font) font = "";

   /* check if the font is embedded in the .eet */
   if (ed->file->fonts)
     {
        Edje_Font_Directory_Entry *fnt = eina_hash_find(ed->file->fonts, font);

        if (fnt)
          {
             size_t len = strlen(font) + sizeof("edje/fonts/") + 1;
             font2 = alloca(len);
             sprintf(font2, "edje/fonts/%s", font);
             font = font2;
             inlined_font = 1;
             font2 = NULL;
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

   sw = TO_INT(params->eval.w);
   sh = TO_INT(params->eval.h);

   if (params->type.text.size)
     size = params->type.text.size;
   if (!text) text = "";

   if ((text == ep->typedata.text->cache.in_str)
       || (text && ep->typedata.text->cache.in_str && !strcmp(ep->typedata.text->cache.in_str, text)))
     {
        text = ep->typedata.text->cache.in_str;
        same_text = EINA_TRUE;
     }

   if ((ep->typedata.text->cache.in_size == size) &&
       (ep->typedata.text->cache.in_w == sw) &&
       (ep->typedata.text->cache.in_h == sh) &&
       (ep->typedata.text->cache.in_str) &&
       same_text &&
       (ep->typedata.text->cache.align_x == params->type.text.align.x) &&
       (ep->typedata.text->cache.align_y == params->type.text.align.y) &&
       (ep->typedata.text->cache.ellipsis == params->type.text.ellipsis) &&
       (ep->typedata.text->cache.fit_x == chosen_desc->text.fit_x) &&
       (ep->typedata.text->cache.fit_y == chosen_desc->text.fit_y) &&
       (ep->typedata.text->cache.in_font == font))
     {
        text = ep->typedata.text->cache.out_str;
        size = ep->typedata.text->cache.out_size;

        goto arrange_text;
     }
   if (!same_text)
     {
        eina_stringshare_replace(&ep->typedata.text->cache.in_str, text);
     }
   ep->typedata.text->cache.in_size = size;
   if (chosen_desc->text.fit_x && (ep->typedata.text->cache.in_str && eina_stringshare_strlen(ep->typedata.text->cache.in_str) > 0))
     {
        eo_do(ep->object,

              if (inlined_font) efl_interface_text_properties_font_source_set(ed->path);
              else efl_interface_text_properties_font_source_set(NULL);

              if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));

              efl_interface_text_properties_font_set(font, size);
              evas_obj_text_set(text));

        part_get_geometry(ep, &tw, &th);
        /* Find the wanted font size */
        if ((tw != sw) && (size > 0) && (tw != 0))
          {
             size = (size * sw) / tw;

             eo_do(ep->object,

                   if (inlined_font) efl_interface_text_properties_font_source_set(ed->path);
                   else efl_interface_text_properties_font_source_set(NULL);

                   if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));

                   efl_interface_text_properties_font_set(font, size));

             part_get_geometry(ep, &tw, &th);
          }

        /* FIXME: This should possibly be replaced by more proper handling,
         * but it's still way better than what was here before. */
        if (tw > sw) size--;
     }
   if (chosen_desc->text.fit_y && (ep->typedata.text->cache.in_str && eina_stringshare_strlen(ep->typedata.text->cache.in_str) > 0))
     {
        /* if we fit in the x axis, too, size already has a somewhat
         * meaningful value, so don't overwrite it with the starting
         * value in that case
         */
        if (!chosen_desc->text.fit_x) size = sh;

        eo_do(ep->object,

              if (inlined_font) efl_interface_text_properties_font_source_set(ed->path);
              else efl_interface_text_properties_font_source_set(NULL);

              if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));

              efl_interface_text_properties_font_set(font, size);
              evas_obj_text_set(text));
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

                  eo_do(ep->object,

                        if (inlined_font) efl_interface_text_properties_font_source_set(ed->path);
                        else efl_interface_text_properties_font_source_set(NULL);

                        if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));
                        efl_interface_text_properties_font_set(font, size));

                  part_get_geometry(ep, &tw, &th);
                  if ((size > 0) && (th == 0)) break;
               }
             size -= dif;
          }
        else if (th > sh)
          {
             int current;

             eo_do(ep->object,
                   if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));
                   efl_interface_text_properties_font_set(font, 10));

             part_get_geometry(ep, &tw, &th);

             if (th == sh)
               current = 10;
             else
               {
                  int bottom, top;

                  if (th < sh) bottom = 10;
                  else if (th > sh) bottom = 1;
                  else bottom = 0; /* XXX shut up GCC, th == sh is handled before! */

                  top = size;
                  /* search one that fits (binary search) */
                  do
                    {
                       current = (top + bottom) / 2;

                       eo_do(ep->object,
                             if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));
                             efl_interface_text_properties_font_set(font, current));

                       part_get_geometry(ep, &tw, &th);

                       if      (th < sh) bottom = current + 1;
                       else if (th > sh) top    = current - 1;
                    } while ((bottom < top) && (th != sh));
               }

             /* search the larger one that fits (linear search) */
             do
               {
                  current++;

                  eo_do(ep->object,
                        if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));
                        efl_interface_text_properties_font_set(font, current));

                  part_get_geometry(ep, &tw, &th);
               } while (th <= sh);
             size = current - 1;
          }
     }

   /* Make sure the size is in range */
   if (size < 1)
     size = 1;
   else if ((size > chosen_desc->text.size_range_max) &&
            (chosen_desc->text.size_range_max > 0))
     size = chosen_desc->text.size_range_max;
   else if (size < chosen_desc->text.size_range_min)
     size = chosen_desc->text.size_range_min;

   /* Handle ellipsis */
   if (!chosen_desc->text.min_x)
     {
        eo_do(ep->object,
              if (inlined_font) efl_interface_text_properties_font_source_set(ed->path);
              else efl_interface_text_properties_font_source_set(NULL));

        text = _edje_text_fit_x(ed, ep, params, chosen_desc,
                                text, font, size,
                                sw, sh, &free_text);
     }
   /* when evas ellipsis support was added in efl 1.8 and used to replace
    * previous support, SOMEONE, who I shall call "cedric", borked ellipsis
    * defaults. as a result, edje_cc continued using 0.0 (left-most) as its default value
    * for ellipsis while evas used -1.0 (no ellipsizing).
    * this was moderately okay for a time because nobody was using it or GROUP parts
    * with text in them very frequently, and so nobody noticed that the mismatch was breaking
    * sizing in some cases when the edje ellipsis value failed to be applied,
    * which ocurred any time text.min_x was set; in this case, ellipsis would NEVER be
    * correctly applied, and instead the text object would only ever get the first
    * ellipsis_set(0), permanently breaking the part.
    * the only way to fix this while preserving previous behavior was to bump
    * the edje file minor version and then check it here to ignore "unset" ellipsis
    * values from old file versions.
    * the downside is that this will break old files which have text.min_x set to 0...maybe.
    *
    * -zmike
    * 22 April 2014
    */
   else if (((ed->file->version >= 3) && (ed->file->minor >= 6)) ||
            params->type.text.ellipsis)
     evas_object_text_ellipsis_set(ep->object,
           chosen_desc->text.min_x ? -1 : params->type.text.ellipsis);

   eina_stringshare_replace(&ep->typedata.text->cache.out_str, text);
   ep->typedata.text->cache.in_w = sw;
   ep->typedata.text->cache.in_h = sh;
   ep->typedata.text->cache.in_font = font;
   ep->typedata.text->cache.out_size = size;
   ep->typedata.text->cache.align_x = params->type.text.align.x;
   ep->typedata.text->cache.align_y = params->type.text.align.y;
   ep->typedata.text->cache.ellipsis = params->type.text.ellipsis;
   ep->typedata.text->cache.fit_x = chosen_desc->text.fit_x;
   ep->typedata.text->cache.fit_y = chosen_desc->text.fit_y;
arrange_text:

   eo_do(ep->object,
         if (inlined_font) efl_interface_text_properties_font_source_set(ed->path);
         else efl_interface_text_properties_font_source_set(NULL);

         if (ep->part->scale) evas_obj_scale_set(TO_DOUBLE(sc));

         efl_interface_text_properties_font_set(font, size),
         evas_obj_text_set(text));
   part_get_geometry(ep, &tw, &th);

   /* filters */
   eo_do(ep->object,
         EINA_LIST_FOREACH(prev_sources, li, source_name)
           evas_obj_text_filter_source_set(source_name, NULL);

         EINA_LIST_FOREACH(filter_sources, li, source_name)
           {
              Edje_Real_Part *rp = _edje_real_part_get(ed, source_name);
              evas_obj_text_filter_source_set(source_name, rp ? rp->object : NULL);
           };
         evas_obj_text_filter_program_set(filter));

   /* Handle alignment */
     {
        FLOAT_T align_x;
        if (params->type.text.align.x < FROM_INT(0))
          {
             if (evas_object_text_direction_get(ep->object) ==
                 EVAS_BIDI_DIRECTION_RTL)
               {
                  align_x = FROM_INT(1);
               }
             else
               {
                  align_x = FROM_INT(0);
               }
          }
        else
          {
             align_x = params->type.text.align.x;
          }
        ep->typedata.text->offset.x = TO_INT(SCALE(align_x, (sw - tw)));
        ep->typedata.text->offset.y = TO_INT(SCALE(params->type.text.align.y, (sh - th)));
     }

   if (!calc_only)
     {
        eo_do(ep->object,
              evas_obj_position_set(ed->x + TO_INT(params->eval.x) + ep->typedata.text->offset.x,
                               ed->y + TO_INT(params->eval.y) + ep->typedata.text->offset.y);

              evas_obj_visibility_set(params->visible));
     }


     {
        Evas_Text_Style_Type style;
        Edje_Text_Effect effect;

        style = EVAS_TEXT_STYLE_PLAIN;

        evas_object_color_set(ep->object,
                              (params->color.r * params->color.a) / 255,
                              (params->color.g * params->color.a) / 255,
                              (params->color.b * params->color.a) / 255,
                              params->color.a);
        effect = ep->part->effect;
        switch (effect & EDJE_TEXT_EFFECT_MASK_BASIC)
          {
           case EDJE_TEXT_EFFECT_NONE:
           case EDJE_TEXT_EFFECT_PLAIN:
              style = EVAS_TEXT_STYLE_PLAIN;
              break;
           case EDJE_TEXT_EFFECT_OUTLINE:
              style = EVAS_TEXT_STYLE_OUTLINE;
              evas_object_text_outline_color_set(ep->object,
                                                 (params->type.text.color2.r * params->type.text.color2.a) / 255,
                                                 (params->type.text.color2.g * params->type.text.color2.a) / 255,
                                                 (params->type.text.color2.b * params->type.text.color2.a) / 255,
                                                 params->type.text.color2.a);
              break;
           case EDJE_TEXT_EFFECT_SOFT_OUTLINE:
              style = EVAS_TEXT_STYLE_SOFT_OUTLINE;
              evas_object_text_outline_color_set(ep->object,
                                                 (params->type.text.color2.r * params->type.text.color2.a) / 255,
                                                 (params->type.text.color2.g * params->type.text.color2.a) / 255,
                                                 (params->type.text.color2.b * params->type.text.color2.a) / 255,
                                                 params->type.text.color2.a);
              break;
           case EDJE_TEXT_EFFECT_SHADOW:
              style = EVAS_TEXT_STYLE_SHADOW;
              evas_object_text_shadow_color_set(ep->object,
                                                (params->type.text.color3.r * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.g * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.b * params->type.text.color3.a) / 255,
                                                params->type.text.color3.a);
              break;
           case EDJE_TEXT_EFFECT_SOFT_SHADOW:
              style = EVAS_TEXT_STYLE_SOFT_SHADOW;
              evas_object_text_shadow_color_set(ep->object,
                                                (params->type.text.color3.r * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.g * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.b * params->type.text.color3.a) / 255,
                                                params->type.text.color3.a);
              break;
           case EDJE_TEXT_EFFECT_OUTLINE_SHADOW:
              style = EVAS_TEXT_STYLE_OUTLINE_SHADOW;
              eo_do(ep->object,
                    evas_obj_text_outline_color_set((params->type.text.color2.r * params->type.text.color2.a) / 255,
                                                    (params->type.text.color2.g * params->type.text.color2.a) / 255,
                                                    (params->type.text.color2.b * params->type.text.color2.a) / 255,
                                                    params->type.text.color2.a),
                    evas_obj_text_shadow_color_set((params->type.text.color3.r * params->type.text.color3.a) / 255,
                                                   (params->type.text.color3.g * params->type.text.color3.a) / 255,
                                                   (params->type.text.color3.b * params->type.text.color3.a) / 255,
                                                   params->type.text.color3.a));
              break;
           case EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW:
              style = EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW;
              eo_do(ep->object,
                    evas_obj_text_outline_color_set((params->type.text.color2.r * params->type.text.color2.a) / 255,
                                                    (params->type.text.color2.g * params->type.text.color2.a) / 255,
                                                    (params->type.text.color2.b * params->type.text.color2.a) / 255,
                                                    params->type.text.color2.a),
                    evas_obj_text_shadow_color_set((params->type.text.color3.r * params->type.text.color3.a) / 255,
                                                   (params->type.text.color3.g * params->type.text.color3.a) / 255,
                                                   (params->type.text.color3.b * params->type.text.color3.a) / 255,
                                                   params->type.text.color3.a));
              break;
           case EDJE_TEXT_EFFECT_FAR_SHADOW:
              style = EVAS_TEXT_STYLE_FAR_SHADOW;
              evas_object_text_shadow_color_set(ep->object,
                                                (params->type.text.color3.r * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.g * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.b * params->type.text.color3.a) / 255,
                                                params->type.text.color3.a);
              break;
           case EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW:
              style = EVAS_TEXT_STYLE_FAR_SOFT_SHADOW;
              evas_object_text_shadow_color_set(ep->object,
                                                (params->type.text.color3.r * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.g * params->type.text.color3.a) / 255,
                                                (params->type.text.color3.b * params->type.text.color3.a) / 255,
                                                params->type.text.color3.a);
              break;
           case EDJE_TEXT_EFFECT_GLOW:
              style = EVAS_TEXT_STYLE_GLOW;
              eo_do(ep->object,
                    evas_obj_text_glow_color_set((params->type.text.color2.r * params->type.text.color2.a) / 255,
                                                 (params->type.text.color2.g * params->type.text.color2.a) / 255,
                                                 (params->type.text.color2.b * params->type.text.color2.a) / 255,
                                                 params->type.text.color2.a),
                    evas_obj_text_glow2_color_set((params->type.text.color3.r * params->type.text.color3.a) / 255,
                                                  (params->type.text.color3.g * params->type.text.color3.a) / 255,
                                                  (params->type.text.color3.b * params->type.text.color3.a) / 255,
                                                  params->type.text.color3.a));
              break;
           default:
              style = EVAS_TEXT_STYLE_PLAIN;
              break;
          }

        switch (effect & EDJE_TEXT_EFFECT_MASK_SHADOW_DIRECTION)
          {
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_RIGHT:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT);
              break;
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM);
              break;
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_BOTTOM_LEFT:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT);
              break;
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_LEFT:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT);
              break;
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_LEFT:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT);
              break;
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP);
              break;
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_TOP_RIGHT:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT);
              break;
           case EDJE_TEXT_EFFECT_SHADOW_DIRECTION_RIGHT:
              EVAS_TEXT_STYLE_SHADOW_DIRECTION_SET
                 (style, EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT);
              break;
           default:
              break;
          }
        evas_object_text_style_set(ep->object, style);
     }

   if (free_text) free((char *)text);
   if (font2) free(font2);
   if (sfont) free(sfont);
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
