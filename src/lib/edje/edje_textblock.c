#include "edje_private.h"

static double
_edje_part_recalc_single_textblock_scale_range_adjust(Edje_Part_Description_Text *chosen_desc, double base_scale, double scale)
{
   double size, min, max;

   if (chosen_desc->text.size == 0)
     return scale;

   min = base_scale * chosen_desc->text.size_range_min;
   max = chosen_desc->text.size_range_max * base_scale;
   size = chosen_desc->text.size * scale;

   if ((size > max) && (max > 0))
     scale = max / (double)chosen_desc->text.size;
   else if (size < min)
     scale = min / (double)chosen_desc->text.size;

   return scale;
}

static inline const char *
_edje_part_recalc_textblock_text_get(Edje *ed, Edje_Real_Part *ep,
      Edje_Part_Description_Text *chosen_desc)
{
   const char *text = NULL;

   if (chosen_desc->text.id_source >= 0)
     {
        Edje_Part_Description_Text *et;

        ep->typedata.text->source = ed->table_parts[chosen_desc->text.id_source % ed->table_parts_size];
     }
   else
     {
        ep->typedata.text->source = NULL;
     }

   if (chosen_desc->text.id_text_source >= 0)
     {
        Edje_Part_Description_Text *et;
        Edje_Real_Part *rp;

        ep->typedata.text->text_source = ed->table_parts[chosen_desc->text.id_text_source % ed->table_parts_size];

        et = _edje_real_part_text_text_source_description_get(ep, &rp);
        text = edje_string_get(&et->text.text);

        if (rp->typedata.text->text) text = rp->typedata.text->text;
     }
   else
     {
        ep->typedata.text->text_source = NULL;
        text = NULL;
        if (chosen_desc->text.domain)
          {
             if (!chosen_desc->text.text.translated)
                chosen_desc->text.text.translated = _set_translated_string(ed, ep);
             if (chosen_desc->text.text.translated)
                text = chosen_desc->text.text.translated;
          }
        if (!text)
           text = edje_string_get(&chosen_desc->text.text);
        if (ep->typedata.text->text) text = ep->typedata.text->text;
     }

   return text;
}

static inline Edje_Style *
_edje_part_recalc_textblock_style_get(Edje *ed, Edje_Real_Part *ep,
      Edje_Part_Description_Text *chosen_desc)
{
   Edje_Style *stl = NULL;
   const char *tmp, *style = "";
   Eina_List *l;

   if (chosen_desc->text.id_source >= 0)
     {
        Edje_Part_Description_Text *et;

        ep->typedata.text->source = ed->table_parts[chosen_desc->text.id_source % ed->table_parts_size];

        et = _edje_real_part_text_source_description_get(ep, NULL);
        tmp = edje_string_get(&et->text.style);
        if (tmp) style = tmp;
     }
   else
     {
        ep->typedata.text->source = NULL;

        tmp = edje_string_get(&chosen_desc->text.style);
        if (tmp) style = tmp;
     }

   EINA_LIST_FOREACH(ed->file->styles, l, stl)
     {
        if ((stl->name) && (!strcmp(stl->name, style))) break;
        stl = NULL;
     }

   return stl;
}

static inline void
_edje_part_recalc_textblock_fit(Edje_Real_Part *ep,
      Edje_Part_Description_Text *chosen_desc,
      Edje_Calc_Params *params,
      FLOAT_T sc, Evas_Coord *tw, Evas_Coord *th)
{
   double base_s = 1.0;
   double orig_s;
   double s = base_s;

   if (ep->part->scale) base_s = TO_DOUBLE(sc);
   efl_gfx_entity_scale_set(ep->object, base_s);
   efl_canvas_text_size_native_get(ep->object, tw, th);

   orig_s = base_s;
   /* Now make it bigger so calculations will be more accurate
    * and less influenced by hinting... */
     {
        orig_s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s,
              orig_s * TO_INT(params->eval.w) / *tw);
        efl_gfx_entity_scale_set(ep->object, orig_s);
        efl_canvas_text_size_native_get(ep->object, tw, th);
     }
   if (chosen_desc->text.fit_x)
     {
        if (*tw > 0)
          {
             s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s,
                   orig_s * TO_INT(params->eval.w) / *tw);
             efl_gfx_entity_scale_set(ep->object, s);
             efl_canvas_text_size_native_get(ep->object, NULL, NULL);
          }
     }
   if (chosen_desc->text.fit_y)
     {
        if (*th > 0)
          {
             double tmp_s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s,
                   orig_s * TO_INT(params->eval.h) / *th);
             /* If we already have X fit, restrict Y to be no bigger
              * than what we got with X. */
             if (!((chosen_desc->text.fit_x) && (tmp_s > s)))
               {
                  s = tmp_s;
               }

             efl_gfx_entity_scale_set(ep->object, s);
             efl_canvas_text_size_native_get(ep->object, NULL, NULL);
          }
     }

   /* Final tuning, try going down 90% at a time, hoping it'll
    * actually end up being correct. */
     {
        int i = 5;   /* Tries before we give up. */
        Evas_Coord fw, fh;
        efl_canvas_text_size_native_get(ep->object, &fw, &fh);

        /* If we are still too big, try reducing the size to
         * 95% each try. */
        while ((i > 0) &&
              ((chosen_desc->text.fit_x && (fw > TO_INT(params->eval.w))) ||
               (chosen_desc->text.fit_y && (fh > TO_INT(params->eval.h)))))
          {
             double tmp_s = _edje_part_recalc_single_textblock_scale_range_adjust(chosen_desc, base_s, s * 0.95);

             /* Break if we are not making any progress. */
             if (EQ(tmp_s, s))
                break;
             s = tmp_s;

             efl_gfx_entity_scale_set(ep->object, s);
             efl_canvas_text_size_native_get(ep->object, &fw, &fh);
             i--;
          }
     }
}


/*
 * Legacy function for min/max calculation of textblock part.
 * It can't calculate min/max properly in many cases.
 *
 * To keep backward compatibility, it will be used for old version of EDJ files.
 * You can't see proper min/max result accroding to documents with this function.
 */
static void
_edje_part_recalc_single_textblock_min_max_calc_legacy(Edje_Real_Part *ep,
                                                       Edje_Part_Description_Text *chosen_desc,
                                                       Edje_Calc_Params *params,
                                                       int *minw, int *minh,
                                                       int *maxw, int *maxh)
{
   Evas_Coord tw, th, ins_l, ins_r, ins_t, ins_b;
   unsigned char minx2 = 0, miny2 = 0, maxx2 = 0, maxy2 = 0;

   minx2 = chosen_desc->text.min_x;
   miny2 = chosen_desc->text.min_y;
   maxx2 = chosen_desc->text.max_x;
   maxy2 = chosen_desc->text.max_y;

   // Do not use size from new api if min/max are non-zero in the theme
   if (!chosen_desc->text.min_x && !chosen_desc->text.min_y &&
         !chosen_desc->text.max_x && !chosen_desc->text.max_y)
     {
        minx2 = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_X;
        miny2 = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_Y;
        maxx2 = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MAX_X;
        maxy2 = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MAX_Y;
     }

   /* Legacy code for Textblock min/max calculation */
   if (minx2 || miny2)
     {
        int mw = 0, mh = 0;

        tw = th = 0;
        if (!minx2)
          {
             efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(TO_INT(params->eval.w),  TO_INT(params->eval.h)));
             efl_canvas_text_size_formatted_get(ep->object, &tw, &th);
          }
        else
          evas_object_textblock_size_native_get(ep->object, &tw, &th);
        evas_object_textblock_style_insets_get(ep->object, &ins_l,
                                               &ins_r, &ins_t, &ins_b);
        mw = ins_l + tw + ins_r;
        mh = ins_t + th + ins_b;
        if (minw && minx2)
          {
             if (mw > *minw) *minw = mw;
          }
        if (minh && miny2)
          {
             if (mh > *minh) *minh = mh;
          }
     }

   if ((maxx2) || (maxy2))
     {
        int mw = 0, mh = 0;

        tw = th = 0;
        if (!maxx2)
          {
             efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(TO_INT(params->eval.w),  TO_INT(params->eval.h)));
             efl_canvas_text_size_formatted_get(ep->object, &tw, &th);
          }
        else
          evas_object_textblock_size_native_get(ep->object, &tw, &th);
        evas_object_textblock_style_insets_get(ep->object, &ins_l, &ins_r,
                                               &ins_t, &ins_b);
        mw = ins_l + tw + ins_r;
        mh = ins_t + th + ins_b;
        if (maxw && maxx2)
          {
             if (mw > *maxw) *maxw = mw;
             if (minw && (*maxw < *minw)) *maxw = *minw;
          }
        if (maxh && maxy2)
          {
             if (mh > *maxh) *maxh = mh;
             if (minh && (*maxh < *minh)) *maxh = *minh;
          }
     }
}

static void
_edje_part_recalc_single_textblock_min_max_calc(Edje_Real_Part *ep,
                                                Edje_Part_Description_Text *chosen_desc,
                                                Edje_Calc_Params *params,
                                                int *minw, int *minh,
                                                int *maxw, int *maxh)
{
   Evas_Coord tw, th, ins_l, ins_r, ins_t, ins_b;
   Evas_Coord min_calc_w = 0, min_calc_h = 0;

   unsigned char dminx, dminy, dmaxx, dmaxy;

   dminx = chosen_desc->text.min_x;
   dminy = chosen_desc->text.min_y;
   dmaxx = chosen_desc->text.max_x;
   dmaxy = chosen_desc->text.max_y;

   // Do not use size from new api if min/max are non-zero in the theme
   if (!chosen_desc->text.min_x && !chosen_desc->text.min_y &&
         !chosen_desc->text.max_x && !chosen_desc->text.max_y)
     {
        dminx = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_X;
        dminy = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MIN_Y;
        dmaxx = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MAX_X;
        dmaxy = ep->typedata.text->expand & EFL_CANVAS_LAYOUT_PART_TEXT_EXPAND_MAX_Y;
     }

   /* min_calc_* values need to save calculated minumum size
    * for maximum size calculation */
   if (minw) min_calc_w = *minw;
   if (minh) min_calc_h = *minh;

   if (dminx || dminy)
     {
        evas_object_textblock_style_insets_get(ep->object, &ins_l,
                                               &ins_r, &ins_t, &ins_b);

        tw = th = 0;
        if (!dminx)
          {
             /* text.min: 0 1
              * text.max: X X */
             int temp_h = TO_INT(params->eval.h);
             int temp_w = TO_INT(params->eval.w);

             if (min_calc_w > temp_w)
               temp_w = min_calc_w;
             if ((!dmaxx) &&
                 maxw && (*maxw > -1) && (*maxw < temp_w))
               temp_w = *maxw;

             if (dmaxy)
               {
                  /* text.min: 0 1
                   * text.max: X 1 */
                  temp_h = INT_MAX / 10000;
               }
             else if (maxh && (*maxh > TO_INT(params->eval.h)))
               {
                  /* text.min: 0 1
                   * text.max: X 0
                   * And there is a limit for height. */
                  temp_h = *maxh;
               }

             /* If base width for calculation is 0,
              * don't get meaningless height for multiline */
             if (temp_w > 0)
               {
                  efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(temp_w,  temp_h));
                  efl_canvas_text_size_formatted_get(ep->object, &tw, &th);

                  tw += ins_l + ins_r;
                  th += ins_t + ins_b;
               }
             else
               {
                  efl_canvas_text_size_native_get(ep->object, NULL, &th);

                  th += ins_t + ins_b;
               }
          }
        else
          {
             /* text.min: 1 X
              * text.max: X X */
             if (dminy && (!dmaxx) &&
                 maxw && (*maxw > -1))
               {
                  /* text.min: 1 1
                   * text.max: 0 X */
                  int temp_w, temp_h;

                  temp_w = *maxw;
                  temp_h = INT_MAX / 10000;

                  if (min_calc_w > temp_w)
                    temp_w = min_calc_w;

                  if ((!dmaxy) && maxh && (*maxh > -1))
                    {
                       /* text.min: 1 1
                        * text.max: 0 0
                        * There is limit for height. */
                       temp_h = *maxh;
                    }

                  efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(temp_w,  temp_h));
                  efl_canvas_text_size_formatted_get(ep->object, &tw, &th);

                  tw += ins_l + ins_r;
                  th += ins_t + ins_b;

                  /* If base width for calculation is 0,
                   * don't get meaningless height for multiline */
                  if (temp_w <= 0)
                    {
                       efl_canvas_text_size_native_get(ep->object, NULL, &th);

                       th += ins_t + ins_b;
                    }
               }
             else
               {
                  /* text.min: 1 X
                   * text.max: 1 X
                   * Or,
                   * text.min: 1 X
                   * text.max: 0 X without max width.
                   * It is a singleline Textblock. */
                  efl_canvas_text_size_native_get(ep->object, &tw, &th);

                  tw += ins_l + ins_r;
                  th += ins_t + ins_b;

                  if (!dmaxx &&
                      (maxw && (*maxw > -1) && (*maxw < tw)))
                    {
                       /* text.min: 1 0
                        * text.max: 0 X */
                       tw = *maxw;
                    }
               }
          }

        if (tw > min_calc_w) min_calc_w = tw;
        if (th > min_calc_h) min_calc_h = th;
        if (dminx && minw) *minw = min_calc_w;
        if (dminy && minh) *minh = min_calc_h;
     }

   if ((dmaxx) || (dmaxy))
     {
        evas_object_textblock_style_insets_get(ep->object, &ins_l, &ins_r,
                                               &ins_t, &ins_b);

        tw = th = 0;
        if (!dmaxx)
          {
             /* text.min: X X
              * text.max: 0 1 */
             int temp_w, temp_h;

             if (dminy)
               {
                  /* text.min: X 1
                   * text.max: 0 1
                   * Already calculated in text for height. */
                  tw = TO_INT(params->eval.w);
                  if (min_calc_w > tw)
                    tw = min_calc_w;

                  th = min_calc_h;
               }
             else
               {
                  /* text.min: X 0
                   * text.max: 0 1 */
                  temp_w = TO_INT(params->eval.w);
                  temp_h = TO_INT(params->eval.h);

                  if (min_calc_w > temp_w)
                    temp_w = min_calc_w;
                  if (maxw && (*maxw > -1) && (*maxw < temp_w))
                    temp_w = *maxw;
                  if (min_calc_h > temp_h)
                    temp_h = min_calc_h;

                  /* If base width for calculation is 0,
                   * don't get meaningless height for multiline */
                  if (temp_w > 0)
                    {
                       efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(temp_w,  temp_h));
                       efl_canvas_text_size_formatted_get(ep->object, &tw, &th);

                       tw += ins_l + ins_r;
                       th += ins_t + ins_b;
                    }
                  else
                    {
                       efl_canvas_text_size_native_get(ep->object, NULL, &th);

                       th += ins_t + ins_b;
                    }
               }
          }
        else
          {
             /* text.max: 1 X */
             if (dminx)
               {
                  /* text.min: 1 X
                   * text.max: 1 X
                   * Singleline. */
                  efl_canvas_text_size_native_get(ep->object, &tw, &th);

                  tw += ins_l + ins_r;
                  th += ins_t + ins_b;
               }
             else
               {
                  /* text.min: 0 X
                   * text.max: 1 X */
                  if (dmaxy)
                    {
                       /* text.min: 0 X
                        * text.max: 1 1 */
                       int temp_w, temp_h;

                       temp_w = TO_INT(params->eval.w);
                       temp_h = TO_INT(params->eval.h);

                       if (min_calc_w > temp_w)
                         temp_w = min_calc_w;
                       if (min_calc_h > temp_h)
                         temp_h = min_calc_h;

                       if (dminy)
                         {
                            /* text.min: 0 1
                             * text.max: 1 1
                             * There is no need to calculate it again. */
                            tw = min_calc_w;
                            th = min_calc_h;
                         }
                       else
                         {
                            /* text.min: 0 0
                             * text.max: 1 1 */

                            efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(temp_w,  temp_h));
                            efl_canvas_text_size_formatted_get(ep->object, &tw, &th);

                            tw += ins_l + ins_r;
                            th += ins_t + ins_b;

                            /* If base width for calculation is 0,
                             * don't get meaningless height for multiline */
                            if (temp_w <= 0)
                              {
                                 efl_canvas_text_size_native_get(ep->object, NULL, &th);

                                 th += ins_t + ins_b;
                              }
                         }
                    }
                  else
                    {
                       /* text.min: 0 X
                        * text.max: 1 0 */
                       int temp_w, temp_h;

                       temp_w = TO_INT(params->eval.w);
                       if (min_calc_w > temp_w)
                         temp_w = min_calc_w;

                       temp_h = efl_gfx_entity_size_get(ep->object).h;
                       efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(temp_w,  temp_h));
                       efl_canvas_text_size_formatted_get(ep->object, &tw, &th);

                       tw += ins_l + ins_r;
                       th += ins_t + ins_b;

                       /* If base width for calculation is 0,
                        * don't get meaningless height for multiline */
                       if (temp_w <= 0)
                         {
                            efl_canvas_text_size_native_get(ep->object, NULL, &th);

                            th += ins_t + ins_b;
                         }
                    }
               }
          }

        if (maxw && dmaxx)
          {
             if (tw > *maxw) *maxw = tw;
             if (minw && (*maxw < *minw)) *maxw = *minw;
          }
        if (maxh && dmaxy)
          {
             if (th > *maxh) *maxh = th;
             if (minh && (*maxh < *minh)) *maxh = *minh;
          }
     }
}

void
_edje_part_recalc_single_textblock(FLOAT_T sc,
                                   Edje *ed,
                                   Edje_Real_Part *ep,
                                   Edje_Part_Description_Text *chosen_desc,
                                   Edje_Calc_Params *params,
                                   int *minw, int *minh,
                                   int *maxw, int *maxh)
{
   if ((ep->type != EDJE_RP_TYPE_TEXT) ||
       (!ep->typedata.text))
     return;

   if (chosen_desc)
     {
        Evas_Coord tw, th;
        const char *font, *font_source, *tmp;
        Eina_List *l;
        int size;
        const char *text = "";
        const char *style = "";
        Edje_Style *stl = NULL;

        text = _edje_part_recalc_textblock_text_get(ed, ep, chosen_desc);
        stl = _edje_part_recalc_textblock_style_get(ed, ep, chosen_desc);

        if (ep->part->scale)
          evas_object_scale_set(ep->object, TO_DOUBLE(sc));

        if (stl)
          {
             if (evas_object_textblock_style_get(ep->object) != stl->style)
               evas_object_textblock_style_set(ep->object, stl->style);
             // FIXME: need to account for editing
             if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
               {
                  // do nothing - should be done elsewhere
               }
             else
               {
                  evas_object_textblock_text_markup_set(ep->object, text);
               }

             if ((chosen_desc->text.fit_x) || (chosen_desc->text.fit_y))
               {
                  _edje_part_recalc_textblock_fit(ep, chosen_desc, params, sc,
                        &tw, &th);
               }

             if ((ed->file->efl_version.major >= 1) && (ed->file->efl_version.minor >= 19))
               {
                  _edje_part_recalc_single_textblock_min_max_calc(ep,
                                                                  chosen_desc,
                                                                  params,
                                                                  minw, minh,
                                                                  maxw, maxh);
               }
             else
               {
                  _edje_part_recalc_single_textblock_min_max_calc_legacy(ep,
                                                                         chosen_desc,
                                                                         params,
                                                                         minw, minh,
                                                                         maxw, maxh);
               }
          }

        evas_object_textblock_valign_set(ep->object, TO_DOUBLE(chosen_desc->text.align.y));
     }
}
