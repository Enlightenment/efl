#include "edje_private.h"

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
   Eina_Size2D size;
   Evas_Coord ins_l, ins_r, ins_t, ins_b;
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

        size.w = size.h = 0;
        if (!minx2)
          {
             efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(TO_INT(params->eval.w),  TO_INT(params->eval.h)));
             size = efl_canvas_textblock_size_formatted_get(ep->object);
          }
        else
          size = efl_canvas_textblock_size_native_get(ep->object);
        evas_object_textblock_style_insets_get(ep->object, &ins_l,
                                               &ins_r, &ins_t, &ins_b);
        mw = ins_l + size.w + ins_r;
        mh = ins_t + size.h + ins_b;
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

        size.w = size.h = 0;
        if (!maxx2)
          {
             efl_gfx_entity_size_set(ep->object, EINA_SIZE2D(TO_INT(params->eval.w),  TO_INT(params->eval.h)));
             size = efl_canvas_textblock_size_formatted_get(ep->object);
          }
        else
          size = efl_canvas_textblock_size_native_get(ep->object);
        evas_object_textblock_style_insets_get(ep->object, &ins_l, &ins_r,
                                               &ins_t, &ins_b);
        mw = ins_l + size.w + ins_r;
        mh = ins_t + size.h + ins_b;
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
   Eina_Size2D size;
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
                  size = efl_canvas_textblock_size_formatted_get(ep->object);
                  tw = size.w;
                  th = size.h;

                  tw += ins_l + ins_r;
                  th += ins_t + ins_b;
               }
             else
               {
                  size = efl_canvas_textblock_size_native_get(ep->object);
                  th = size.h;

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
                  size = efl_canvas_textblock_size_formatted_get(ep->object);
                  tw = size.w;
                  th = size.h;

                  tw += ins_l + ins_r;
                  th += ins_t + ins_b;

                  /* If base width for calculation is 0,
                   * don't get meaningless height for multiline */
                  if (temp_w <= 0)
                    {
                       size = efl_canvas_textblock_size_native_get(ep->object);
                       th = size.h;

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
                  size = efl_canvas_textblock_size_native_get(ep->object);
                  tw = size.w;
                  th = size.h;

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
                       size = efl_canvas_textblock_size_formatted_get(ep->object);
                       tw = size.w;
                       th = size.h;

                       tw += ins_l + ins_r;
                       th += ins_t + ins_b;
                    }
                  else
                    {
                       size = efl_canvas_textblock_size_native_get(ep->object);
                       th = size.h;

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
                  size = efl_canvas_textblock_size_native_get(ep->object);
                  tw = size.w;
                  th = size.h;

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
                            size = efl_canvas_textblock_size_formatted_get(ep->object);
                            tw = size.w;
                            th = size.h;

                            tw += ins_l + ins_r;
                            th += ins_t + ins_b;

                            /* If base width for calculation is 0,
                             * don't get meaningless height for multiline */
                            if (temp_w <= 0)
                              {
                                 size = efl_canvas_textblock_size_native_get(ep->object);
                                 th = size.h;

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
                       size = efl_canvas_textblock_size_formatted_get(ep->object);
                       tw = size.w;
                       th = size.h;

                       tw += ins_l + ins_r;
                       th += ins_t + ins_b;

                       /* If base width for calculation is 0,
                        * don't get meaningless height for multiline */
                       if (temp_w <= 0)
                         {
                            size = efl_canvas_textblock_size_native_get(ep->object);
                            th = size.h;

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

Eina_Bool
_edje_part_textblock_style_text_set(Edje *ed,
                                    Edje_Real_Part *ep,
                                    Edje_Part_Description_Text *chosen_desc)
{
   const char *text = "";
   const char *style = "";
   Evas_Textblock_Style *stl = NULL;
   const char *tmp;

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

        if (!text) text = edje_string_get(&chosen_desc->text.text);

        if (ep->typedata.text->text) text = ep->typedata.text->text;
     }

   stl = _edje_textblock_style_get(ed, style);
   if (stl)
     {
        if (evas_object_textblock_style_get(ep->object) != stl)
          evas_object_textblock_style_set(ep->object, stl);
        // FIXME: need to account for editing
        if (ep->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
          {
             // do nothing - should be done elsewhere
          }
        else
          {
             evas_object_textblock_text_markup_set(ep->object, text);
          }

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static char*
strrstr(const char* haystack, const char* violate)
{
   char *s_ret = NULL;
   char *tmp = NULL;
   const char *_haystack = haystack;
   size_t len = strlen(violate);
   while((tmp = strstr(_haystack, violate))){
     s_ret = tmp;
     _haystack = tmp + len;
   }
   return s_ret;
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

        if (ep->part->scale)
          evas_object_scale_set(ep->object, TO_DOUBLE(sc));

        //Gets textblock's style and text to set evas_object_textblock properties.
        //If there is no style for it. don't need to calc.
        if (_edje_part_textblock_style_text_set(ed, ep, chosen_desc))
          {
             if ((chosen_desc->text.fit_x) || (chosen_desc->text.fit_y))
               {
                  unsigned int size_array[255];
                  size_t size_array_len = 0;
                  Eina_List *l;
                  unsigned int *value;
                  Evas_Textblock_Style *st = _edje_textblock_style_get(ed, chosen_desc->text.style.str);
                  const char *text_style = evas_textblock_style_get(st);
                  char *s_font_size = (text_style) ? strrstr(text_style,"font_size=") : NULL;
                  if (s_font_size && s_font_size[10])
                    {
                      int font_size = (int) strtol(&s_font_size[10], NULL, 10);
                      if (font_size > 0)
                        {
                          chosen_desc->text.size_range_max = font_size;
                          if (chosen_desc->text.size_range_min > chosen_desc->text.size_range_max)
                            chosen_desc->text.size_range_min = chosen_desc->text.size_range_max;
                        }
                    }
                  EINA_LIST_FOREACH(chosen_desc->text.fit_size_array, l, value)
                    {
                       size_array[size_array_len++] = *value;
                    }
                  unsigned int mode = TEXTBLOCK_FIT_MODE_NONE;

                  if (chosen_desc->text.fit_x)
                    mode |= TEXTBLOCK_FIT_MODE_WIDTH;
                  if (chosen_desc->text.fit_y)
                    mode |= TEXTBLOCK_FIT_MODE_HEIGHT;
                  evas_textblock_fit_options_set(ep->object, mode);
                  evas_textblock_fit_step_size_set(ep->object, chosen_desc->text.fit_step);
                  if (size_array_len > 0)
                    {
                       evas_textblock_fit_size_array_set(ep->object,size_array,size_array_len);
                    }
                  else if ( chosen_desc->text.size_range_min || chosen_desc->text.size_range_max)
                    {
                       evas_textblock_fit_size_range_set(ep->object, chosen_desc->text.size_range_min,  chosen_desc->text.size_range_max);
                    }
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
