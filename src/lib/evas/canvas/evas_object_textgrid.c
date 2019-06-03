#include "evas_common_private.h" /* Includes evas_bidi_utils stuff. */
#include "evas_private.h"

#define MY_CLASS EVAS_TEXTGRID_CLASS

#define MY_CLASS_NAME "Evas_Textgrid"

/* private magic number for text objects */
static const char o_type[] = "textgrid";

/* private struct for line object internal data */
typedef struct _Evas_Textgrid_Data         Evas_Textgrid_Data;
typedef struct _Evas_Object_Textgrid_Cell  Evas_Object_Textgrid_Cell;
typedef struct _Evas_Object_Textgrid_Color Evas_Object_Textgrid_Color;

typedef struct _Evas_Object_Textgrid_Row   Evas_Object_Textgrid_Row;
typedef struct _Evas_Object_Textgrid_Rect  Evas_Object_Textgrid_Rect;
typedef struct _Evas_Object_Textgrid_Text  Evas_Object_Textgrid_Text;
typedef struct _Evas_Object_Textgrid_Line  Evas_Object_Textgrid_Line;

struct _Evas_Textgrid_Data
{
   struct {
      int                         w, h;
      int                         char_width;
      int                         char_height;
      Evas_Object_Textgrid_Row   *rows;
      Evas_Textgrid_Cell         *cells;

      const char                 *font_source;
      const char                 *font_name;
      Evas_Font_Size              font_size;
      Evas_Font_Description      *font_description_normal;

      Eina_Array                  palette_standard;
      Eina_Array                  palette_extended;
      Efl_Text_Font_Bitmap_Scalable bitmap_scalable;
   } cur, prev;

   int                            ascent;

   Evas_Font_Set                 *font_normal;
   Evas_Font_Set                 *font_bold;
   Evas_Font_Set                 *font_italic;
   Evas_Font_Set                 *font_bolditalic;

   unsigned int                   changed : 1;
   unsigned int                   core_change : 1;
   unsigned int                   row_change : 1;
   unsigned int                   pal_change : 1;
};

struct _Evas_Object_Textgrid_Color
{
   unsigned char r, g, b, a;
};

struct _Evas_Object_Textgrid_Row
{
   int ch1, ch2; // change region, -1 == none
   int rects_num, texts_num, lines_num;
   int rects_alloc, texts_alloc, lines_alloc;
   Evas_Object_Textgrid_Rect *rects; // rects + colors
   Evas_Object_Textgrid_Text *texts; // text
   Evas_Object_Textgrid_Line *lines; // underlines, strikethroughs
};

struct _Evas_Object_Textgrid_Rect
{
   unsigned char r, g, b, a;
   int x, w;
};

struct _Evas_Object_Textgrid_Text
{
   Evas_Text_Props text_props;
   unsigned char r, g, b, a;
   int           x      : 30;
   unsigned char bold   :  1;
   unsigned char italic :  1;
};

struct _Evas_Object_Textgrid_Line
{
   unsigned char r, g, b, a;
   int x, w, y;
};

/* private methods for textgrid objects */
static void evas_object_textgrid_init(Evas_Object *eo_obj);
static void evas_object_textgrid_render(Evas_Object *eo_obj,
                                        Evas_Object_Protected_Data *obj,
                                        void *type_private_data,
                                        void *engine, void *output, void *context, void *surface,
                                        int x, int y, Eina_Bool do_async);
static void evas_object_textgrid_render_pre(Evas_Object *eo_obj,
					    Evas_Object_Protected_Data *obj,
					    void *type_private_data);
static void evas_object_textgrid_render_post(Evas_Object *eo_obj,
					     Evas_Object_Protected_Data *obj,
					     void *type_private_data);

static void *evas_object_textgrid_engine_data_get(Evas_Object *eo_obj);

static int evas_object_textgrid_is_opaque(Evas_Object *eo_obj,
					  Evas_Object_Protected_Data *obj,
					  void *type_private_data);
static int evas_object_textgrid_was_opaque(Evas_Object *eo_obj,
					   Evas_Object_Protected_Data *obj,
					   void *type_private_data);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
   evas_object_textgrid_render,
   evas_object_textgrid_render_pre,
   evas_object_textgrid_render_post,
   evas_object_textgrid_engine_data_get,
   /* these are optional. NULL = nothing */
   NULL,
   NULL,
   evas_object_textgrid_is_opaque,
   evas_object_textgrid_was_opaque,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL // render_prepare
};

/* all nice and private */
static void
evas_object_textgrid_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = efl_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;

   Evas_Textgrid_Data *o = obj->private_data;
   o->prev.bitmap_scalable = o->cur.bitmap_scalable = EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR;
   o->prev = o->cur;
   eina_array_step_set(&o->cur.palette_standard, sizeof (Eina_Array), 16);
   eina_array_step_set(&o->cur.palette_extended, sizeof (Eina_Array), 16);
}

static void
evas_object_textgrid_row_clear(Evas_Textgrid_Data *o EINA_UNUSED,
                               Evas_Object_Textgrid_Row *r)
{
   int i;

   if (r->rects)
     {
        free(r->rects);
        r->rects = NULL;
        r->rects_num = 0;
        r->rects_alloc = 0;
     }
   if (r->texts)
     {
        for (i = 0; i < r->texts_num; i++)
          evas_common_text_props_content_unref(&(r->texts[i].text_props));
        free(r->texts);
        r->texts = NULL;
        r->texts_num = 0;
        r->texts_alloc = 0;
     }
   if (r->lines)
     {
        free(r->lines);
        r->lines = NULL;
        r->lines_num = 0;
        r->lines_alloc = 0;
     }
}

static void
evas_object_textgrid_rows_clear(Evas_Object *eo_obj)
{
   int i;

   Evas_Textgrid_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o->cur.rows) return;
   for (i = 0; i < o->cur.h; i++)
     {
        evas_object_textgrid_row_clear(o, &(o->cur.rows[i]));
        o->cur.rows[i].ch1 = 0;
        o->cur.rows[i].ch2 = o->cur.w - 1;
     }
}

static void
evas_object_textgrid_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj EINA_UNUSED)
{
   Evas_Object_Textgrid_Color *c;
   Evas_Textgrid_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);

   /* free obj */
   evas_object_textgrid_rows_clear(eo_obj);
   if (o->cur.rows) free(o->cur.rows);
   if (o->cur.font_name) eina_stringshare_del(o->cur.font_name);
   if (o->cur.font_source) eina_stringshare_del(o->cur.font_source);

   if (o->cur.font_description_normal)
     evas_font_desc_unref(o->cur.font_description_normal);
   if (o->font_normal) evas_font_free(o->font_normal);
   if (o->font_bold) evas_font_free(o->font_bold);
   if (o->font_italic) evas_font_free(o->font_italic);
   if (o->font_bolditalic) evas_font_free(o->font_bolditalic);

   if (o->cur.cells) free(o->cur.cells);
   while ((c = eina_array_pop(&o->cur.palette_standard)))
     free(c);
   eina_array_flush(&o->cur.palette_standard);
   while ((c = eina_array_pop(&o->cur.palette_extended)))
     free(c);
   eina_array_flush(&o->cur.palette_extended);
}

EOLIAN static void
_evas_textgrid_efl_object_destructor(Eo *eo_obj, Evas_Textgrid_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_textgrid_free(eo_obj, obj);
   efl_data_unref(eo_obj, obj->private_data);
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

static void
evas_object_textgrid_row_rect_append(Evas_Object_Textgrid_Row *row,
                                     int x, int w,
                                     int r, int g, int b, int a)
{
   row->rects_num++;
   if (row->rects_num > row->rects_alloc)
     {
        Evas_Object_Textgrid_Rect *t;

        row->rects_alloc += 8; // dont expect many rects per line
        t = realloc(row->rects, sizeof(Evas_Object_Textgrid_Rect) * row->rects_alloc);
        if (!t)
          {
             row->rects_num--;
             return;
          }
        row->rects = t;
     }
   row->rects[row->rects_num - 1].x = x;
   row->rects[row->rects_num - 1].w = w;
   row->rects[row->rects_num - 1].r = r;
   row->rects[row->rects_num - 1].g = g;
   row->rects[row->rects_num - 1].b = b;
   row->rects[row->rects_num - 1].a = a;
}

static Evas_Font_Set *
_textgrid_font_get(Evas_Textgrid_Data *o,
                   Eina_Bool is_bold,
                   Eina_Bool is_italic)
{
   if ((!is_bold) && (!is_italic))
     return o->font_normal;
   /* bold */
   else if ((is_bold) && (!is_italic))
     {
        if (o->font_bold)
          return o->font_bold;
        else
          return o->font_normal;
     }
   /* italic */
   else if ((!is_bold) && (is_italic))
     {
        if (o->font_italic)
          return o->font_italic;
        else
          return o->font_normal;
     }
   /* bolditalic */
   else
     {
        if (o->font_bolditalic)
          return o->font_bolditalic;
        else if (o->font_italic)
          return o->font_italic;
        else if (o->font_bold)
          return o->font_bold;
        else
          return o->font_normal;
     }
}

static void
evas_object_textgrid_row_text_append(Evas_Object_Textgrid_Row *row,
                                     Evas_Object_Protected_Data *obj,
                                     Evas_Textgrid_Data *o,
                                     int x,
                                     Eina_Unicode codepoint,
                                     int r, int g, int b, int a,
                                     Eina_Bool is_bold,
                                     Eina_Bool is_italic)
{
   Evas_Script_Type script;
   Evas_Font_Instance *script_fi = NULL;
   Evas_Font_Instance *cur_fi = NULL;
   Evas_Object_Textgrid_Text *text;
   Evas_Font_Set *font;

   row->texts_num++;
   if (row->texts_num > row->texts_alloc)
     {
        Evas_Object_Textgrid_Text *t;

        row->texts_alloc += 32; // expect more text per line
        t = realloc(row->texts, sizeof(Evas_Object_Textgrid_Text) * row->texts_alloc);
        if (!t)
          {
             row->texts_num--;
             return;
          }
        row->texts = t;
     }

   script = evas_common_language_script_type_get(&codepoint, 1);
   text = &row->texts[row->texts_num - 1];
   text->bold = is_bold;
   text->italic = is_italic;
   font = _textgrid_font_get(o, is_bold, is_italic);
   ENFN->font_run_end_get(ENC, font, &script_fi, &cur_fi,
                          script, &codepoint, 1);
   memset(&(text->text_props), 0, sizeof(Evas_Text_Props));
   evas_common_text_props_script_set(&(text->text_props), script);
   ENFN->font_text_props_info_create(ENC, script_fi, &codepoint,
                                     &(text->text_props), NULL, 0, 1,
                                     EVAS_TEXT_PROPS_MODE_NONE,
                                     o->cur.font_description_normal->lang);

   text->x = x;
   text->r = r;
   text->g = g;
   text->b = b;
   text->a = a;
}

static void
evas_object_textgrid_row_line_append(Evas_Object_Textgrid_Row *row, int x, int w, int y, int r, int g, int b, int a)
{
   row->lines_num++;
   if (row->lines_num > row->lines_alloc)
     {
        Evas_Object_Textgrid_Line *t;

        row->lines_alloc += 8; // dont expect many lines per line
        t = realloc(row->lines, sizeof(Evas_Object_Textgrid_Line) * row->lines_alloc);
        if (!t)
          {
             row->lines_num--;
             return;
          }
        row->lines = t;
     }
   row->lines[row->lines_num - 1].x = x;
   row->lines[row->lines_num - 1].w = w;
   row->lines[row->lines_num - 1].y = y;
   row->lines[row->lines_num - 1].r = r;
   row->lines[row->lines_num - 1].g = g;
   row->lines[row->lines_num - 1].b = b;
   row->lines[row->lines_num - 1].a = a;
}

static Eina_Bool
_drop_glyphs_ref(const void *container EINA_UNUSED, void *data, void *fdata)
{
   Evas_Font_Array_Data *fad = data;
   Evas_Public_Data     *pd = fdata;

   evas_common_font_glyphs_unref(fad->glyphs);
   eina_array_pop(&pd->glyph_unref_queue);

   return EINA_TRUE;
}

static void
evas_object_textgrid_render(Evas_Object *eo_obj EINA_UNUSED,
                            Evas_Object_Protected_Data *obj,
                            void *type_private_data,
                            void *engine, void *output, void *context, void *surface,
                            int x, int y, Eina_Bool do_async)
{
   Evas_Textgrid_Cell *cells;
   Evas_Object_Textgrid_Color *c;
   Eina_Array *palette;
   int xx, yy, xp, yp, w, h, ww, hh;
   int rr = 0, rg = 0, rb = 0, ra = 0, rx = 0, rw = 0, run;

   /* render object to surface with context, and offset by x,y */
   Evas_Textgrid_Data *o = type_private_data;
   ENFN->context_multiplier_unset(engine, context);
   ENFN->context_render_op_set(engine, context, obj->cur->render_op);

   if (!(o->font_normal) || (!o->cur.cells)) return;

   w = o->cur.char_width;
   h = o->cur.char_height;
   ww = obj->cur->geometry.w;
   hh = obj->cur->geometry.h;

   // generate row data from cells (and only deal with rows that updated)
   for (yy = 0, cells = o->cur.cells; yy < o->cur.h; yy++)
     {
        Evas_Object_Textgrid_Row *row = &(o->cur.rows[yy]);

        if (row->ch1 < 0)
          {
             cells += o->cur.w;
             continue;
          }
        row->ch1 = -1;
        row->ch2 = 0;
        run = 0;
        xp = 0;
        for (xx = 0; xx < o->cur.w; xx++, cells++)
          {
             if (cells->bg_extended) palette = &(o->cur.palette_extended);
             else palette = &(o->cur.palette_standard);
             if (cells->bg >= eina_array_count(palette)) c = NULL;
             else c = eina_array_data_get(palette, cells->bg);
             if ((c) && (c->a > 0))
               {
                  if (!run)
                    {
                       run = 1;
                       rr = c->r;
                       rg = c->g;
                       rb = c->b;
                       ra = c->a;
                       rx = xp;
                       rw = w;
                    }
                  else if ((c->r != rr) || (c->g != rg) ||
                           (c->b != rb) || (c->a != ra))
                    {
                       evas_object_textgrid_row_rect_append(row, rx, rw,
                                                            rr, rg, rb, ra);
                       rr = c->r;
                       rg = c->g;
                       rb = c->b;
                       ra = c->a;
                       rx = xp;
                       rw = w;
                    }
                  else rw += w;
               }
             else if (run)
               {
                  run = 0;
                  evas_object_textgrid_row_rect_append(row, rx, rw,
                                                       rr, rg, rb, ra);
               }
             if ((cells->codepoint > 0) || (cells->underline) ||
                 (cells->strikethrough))
               {
                  if (cells->fg_extended) palette = &(o->cur.palette_extended);
                  else palette = &(o->cur.palette_standard);
                  if (cells->fg >= eina_array_count(palette)) c = NULL;
                  else c = eina_array_data_get(palette, cells->fg);
                  if ((c) && (c->a > 0))
                    {
                       if (cells->codepoint > 0)
                         evas_object_textgrid_row_text_append(row, obj,
                                                              o, xp,
                                                              cells->codepoint,
                                                              c->r, c->g, c->b, c->a,
                                                              cells->bold,
                                                              cells->italic);
                       // XXX: underlines and strikethroughs don't get
                       // merged into horizontal runs like bg rects above
                       if (cells->underline)
                         evas_object_textgrid_row_line_append(row, xp, w,
                                                              o->ascent + 1,
                                                              c->r, c->g, c->b, c->a);
                       if (cells->strikethrough)
                         evas_object_textgrid_row_line_append(row, xp, w,
                                                              ((3 * o->ascent) / 4),
                                                              c->r, c->g, c->b, c->a);
                    }
               }
             xp += w;
          }
        if (run)
          {
             run = 0;
             evas_object_textgrid_row_rect_append(row, rx, rw,
                                                  rr, rg, rb, ra);
          }
     }
   yp = obj->cur->geometry.y + y;
   // draw the row data that is generated from the cell array
   for (yy = 0, cells = o->cur.cells; yy < o->cur.h; yy++)
     {
        Evas_Object_Textgrid_Row *row = &(o->cur.rows[yy]);
        Evas_Font_Array          *texts;

        xp = obj->cur->geometry.x + x;
        for (xx = 0; xx < row->rects_num; xx++)
          {
             ENFN->context_color_set(engine, context,
                                     row->rects[xx].r, row->rects[xx].g,
                                     row->rects[xx].b, row->rects[xx].a);
             ENFN->context_cutout_target(engine, context,
                                         xp + row->rects[xx].x, yp,
                                         row->rects[xx].w, h);
             ENFN->rectangle_draw(engine, output, context, surface,
                                  xp + row->rects[xx].x, yp,
                                  row->rects[xx].w, h,
                                  do_async);
          }

        if (row->texts_num)
          {
             if ((do_async) && (ENFN->multi_font_draw))
               {
                  Evas_Font_Set *font, *current_font;
                  Eina_Bool async_unref;
                  Evas_Object_Textgrid_Text *text;

                  xx = 0;
                  do
                    {
                       texts = malloc(sizeof(*texts));
                       if (!texts)
                         {
                            ERR("Failed to allocate Evas_Font_Array.");
                            return;
                         }
                       texts->array = eina_inarray_new(sizeof(Evas_Font_Array_Data), 1);
                       texts->refcount = 1;

                       text = &row->texts[xx];
                       font = _textgrid_font_get(o, text->bold, text->italic);

                       do
                         {
                            Evas_Font_Array_Data *fad;
                            Evas_Text_Props *props;

                            current_font = font;

                            props = &text->text_props;
                            evas_common_font_draw_prepare(props);

                            evas_common_font_glyphs_ref(props->glyphs);
                            evas_unref_queue_glyph_put(obj->layer->evas,
                                                       props->glyphs);

                            fad = eina_inarray_grow(texts->array, 1);
                            if (!fad)
                              {
                                 ERR("Failed to allocate Evas_Font_Array_Data.");
                                 eina_inarray_free(texts->array);
                                 free(texts);
                                 return;
                              }
                            fad->color.r = text->r;
                            fad->color.g = text->g;
                            fad->color.b = text->b;
                            fad->color.a = text->a;
                            fad->x = text->x;
                            fad->glyphs = props->glyphs;

                            fad++;

                            xx++;
                            if (xx >= row->texts_num)
                              break;
                            text = &row->texts[xx];
                            font = _textgrid_font_get(o, text->bold,
                                                      text->italic);
                         }
                       while (font == current_font);

                       ENFN->context_cutout_target(engine, context,
                                                   xp - w, yp + o->ascent - h,
                                                   w * 3, h * 3);
                       async_unref =
                         ENFN->multi_font_draw(engine, output, context, surface,
                                               current_font,
                                               xp,
                                               yp + o->ascent,
                                               ww, hh, ww, hh, texts, do_async);
                       if (async_unref)
                         evas_unref_queue_texts_put(obj->layer->evas, texts);
                       else
                         {
                            eina_inarray_foreach(texts->array, _drop_glyphs_ref,
                                                 obj->layer->evas);
                            eina_inarray_free(texts->array);
                            free(texts);
                         }
                    }
                  while (xx < row->texts_num);
               }
             else
               {
                  for (xx = 0; xx < row->texts_num; xx++)
                    {
                       Evas_Text_Props *props;
                       unsigned int     r, g, b, a;
                       Evas_Object_Textgrid_Text *text = &row->texts[xx];
                       int              tx = xp + text->x;
                       int              ty = yp + o->ascent;
                       Evas_Font_Set *font;

                       props = &text->text_props;

                       r = text->r;
                       g = text->g;
                       b = text->b;
                       a = text->a;

                       ENFN->context_color_set(engine, context,
                                               r, g, b, a);
                       font = _textgrid_font_get(o, text->bold, text->italic);
                       ENFN->context_cutout_target(engine, context,
                                                   tx - w, ty - h,
                                                   w * 3, h * 3);
                       evas_font_draw_async_check(obj, engine, output, context, surface,
                                                  font, tx, ty, ww, hh,
                                                  ww, hh, props, do_async);
                    }
               }
          }

        for (xx = 0; xx < row->lines_num; xx++)
          {
             ENFN->context_color_set(engine, context,
                                     row->lines[xx].r, row->lines[xx].g,
                                     row->lines[xx].b, row->lines[xx].a);
             ENFN->context_cutout_target(engine, context,
                                         xp + row->lines[xx].x, yp + row->lines[xx].y,
                                         row->lines[xx].w, 1);
             ENFN->rectangle_draw(engine, output, context, surface,
                                  xp + row->lines[xx].x, yp + row->lines[xx].y,
                                  row->lines[xx].w, 1,
                                  do_async);
          }
        yp += h;
     }
}

static void
evas_object_textgrid_render_pre(Evas_Object *eo_obj,
				Evas_Object_Protected_Data *obj,
				void *type_private_data)
{
   int is_v, was_v;
   Evas_Textgrid_Data *o = type_private_data;

   /* don't pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;
   /* pre-render phase.  This does anything an object needs to do just before */
   /* rendering.  That could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video, etc. */
   /* When this is done the object needs to figure if it changed and */
   /* if so what and where and add the appropriate redraw rectangles */

   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur->clipper)
     {
	if (obj->cur->cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur->clipper);
	obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
					    obj->cur->clipper,
					    obj->cur->clipper->private_data);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
	goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, eo_obj);
   /* if we restacked (layer or just within a layer) and don't clip anyone */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   if (obj->cur->render_op != obj->prev->render_op)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }
   if (!EINA_DBL_EQ(obj->cur->scale, obj->prev->scale))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
	goto done;
     }

   if (o->changed)
     {
        if (o->core_change)
          {
             if ((o->cur.h != o->prev.h) ||
                 (o->cur.w != o->prev.w) ||
                 (o->cur.font_size != o->prev.font_size) ||
                 ((o->cur.font_name) && (o->prev.font_name) &&
                     (strcmp(o->cur.font_name, o->prev.font_name))) ||
                 ((o->cur.font_name) && (!o->prev.font_name)) ||
                 ((!o->cur.font_name) && (o->prev.font_name)))
               {
                  evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                                      eo_obj, obj);
                  goto done;
               }
          }
        if (o->pal_change)
          {
             evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, eo_obj, obj);
             goto done;
          }
        if (o->row_change)
          {
             int i;

             for (i = 0; i < o->cur.h; i++)
               {
                  Evas_Object_Textgrid_Row *r = &(o->cur.rows[i]);
                  if (r->ch1 >= 0)
                    {
                       Evas_Coord chx, chy, chw, chh;

                       chx = r->ch1 * o->cur.char_width;
                       chy = i * o->cur.char_height;
                       chw = (r->ch2 - r->ch1 + 1) * o->cur.char_width;
                       chh = o->cur.char_height;

                       chx -= o->cur.char_width;
                       chy -= o->cur.char_height;
                       chw += o->cur.char_width * 2;
                       chh += o->cur.char_height * 2;

                       chx += obj->cur->geometry.x;
                       chy += obj->cur->geometry.y;
                       RECTS_CLIP_TO_RECT(chx, chy, chw, chh,
                                          obj->cur->cache.clip.x,
                                          obj->cur->cache.clip.y,
                                          obj->cur->cache.clip.w,
                                          obj->cur->cache.clip.h);
                       evas_add_rect(&obj->layer->evas->clip_changes,
                                     chx, chy, chw, chh);
                    }
               }
          }
     }

   done:
   o->core_change = 0;
   o->row_change = 0;
   o->pal_change = 0;
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, eo_obj, is_v, was_v);
}

static void
evas_object_textgrid_render_post(Evas_Object *eo_obj EINA_UNUSED,
                                 Evas_Object_Protected_Data *obj,
                                 void *type_private_data)
{
   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   Evas_Textgrid_Data *o = type_private_data;
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(obj);
   o->prev = o->cur;
}

static void *
evas_object_textgrid_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Textgrid_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->font_normal; /* TODO: why ? */
}

static int
evas_object_textgrid_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
			       Evas_Object_Protected_Data *obj EINA_UNUSED,
			       void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object is
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

static int
evas_object_textgrid_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
				Evas_Object_Protected_Data *obj EINA_UNUSED,
				void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object was
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

EOLIAN static void
_evas_textgrid_efl_gfx_entity_scale_set(Evas_Object *eo_obj, Evas_Textgrid_Data *o,
                                     double scale)
{
   int font_size;
   const char *font_name;

   if (EINA_DBL_EQ(efl_gfx_entity_scale_get(eo_obj), scale)) return;
   efl_gfx_entity_scale_set(efl_super(eo_obj, MY_CLASS), scale);

   font_name = eina_stringshare_add(o->cur.font_name);
   font_size = o->cur.font_size;
   if (o->cur.font_name) eina_stringshare_del(o->cur.font_name);
   o->cur.font_name = NULL;
   o->prev.font_name = NULL;
   o->cur.font_size = 0;
   o->prev.font_size = 0;
   evas_object_textgrid_font_set(eo_obj, font_name, font_size);
   eina_stringshare_del(font_name);
}

/*********************  LOCAL *********************/

/*********************  API *********************/

EAPI Evas_Object *
evas_object_textgrid_add(Evas *e)
{
   e = evas_find(e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(e, EVAS_CANVAS_CLASS), NULL);
   return efl_add(EVAS_TEXTGRID_CLASS, e, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_evas_textgrid_efl_object_constructor(Eo *eo_obj, Evas_Textgrid_Data *class_data EINA_UNUSED)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   evas_object_textgrid_init(eo_obj);

   return eo_obj;
}

EOLIAN static void
_evas_textgrid_grid_size_set(Eo *eo_obj, Evas_Textgrid_Data *o, int w, int h)
{
   int i;
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if ((h <= 0) || (w <= 0)) return;

   if ((o->cur.w == w) && (o->cur.h == h)) return;

   evas_object_async_block(obj);
   evas_object_textgrid_rows_clear(eo_obj);
   if (o->cur.rows)
     {
        free(o->cur.rows);
        o->cur.rows = NULL;
     }
   if (o->cur.cells)
     {
        free(o->cur.cells);
        o->cur.cells = NULL;
     }
   o->cur.cells = calloc(w * h, sizeof(Evas_Textgrid_Cell));
   if (!o->cur.cells) return;
   o->cur.rows = calloc(h, sizeof(Evas_Object_Textgrid_Row));
   if (!o->cur.rows)
     {
        free(o->cur.cells);
        o->cur.cells = NULL;
        return;
     }
   for (i = 0; i < h; i++)
     {
        o->cur.rows[i].ch1 = 0;
        o->cur.rows[i].ch2 = w - 1;
     }
   o->cur.w = w;
   o->cur.h = h;
   o->changed = 1;
   o->core_change = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_textgrid_grid_size_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int *w, int *h)
{
   if (w) *w = o->cur.w;
   if (h) *h = o->cur.h;
}

EOLIAN static void
_evas_textgrid_efl_text_font_font_source_set(Eo *eo_obj, Evas_Textgrid_Data *o, const char *font_source)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if ((!font_source) || (!*font_source))
     return;

   if ((o->cur.font_source) && (font_source) &&
       (!strcmp(o->cur.font_source, font_source))) return;

   evas_object_async_block(obj);
   eina_stringshare_replace(&o->cur.font_source, font_source);
   o->changed = 1;
   o->core_change = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static const char*
_evas_textgrid_efl_text_font_font_source_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o)
{
   return o->cur.font_source;
}

static int
_alternate_font_weight_slant(Evas_Object_Protected_Data *obj,
                             Evas_Textgrid_Data *o,
                             Evas_Font_Set **fontp,
                             Evas_Font_Description *fdesc,
                             const char *kind)
{
   int ret = -1;
   Evas_Font_Set *font;

   font = evas_font_load(obj->layer->evas->font_path,
                         obj->layer->evas->hinting,
                         fdesc,
                         o->cur.font_source,
                         (int)(((double) o->cur.font_size) *
                         obj->cur->scale),
                         o->cur.bitmap_scalable);
   if (font)
     {
        Eina_Unicode W[2] = { 'O', 0 };
        Evas_Font_Instance *script_fi = NULL;
        Evas_Font_Instance *cur_fi = NULL;
        Evas_Text_Props text_props;
        Evas_Script_Type script;
        int advance, vadvance, ascent;

        script = evas_common_language_script_type_get(W, 1);
        ENFN->font_run_end_get(ENC, font, &script_fi, &cur_fi,
                               script, W, 1);
        memset(&text_props, 0, sizeof(Evas_Text_Props));
        evas_common_text_props_script_set(&text_props, script);
        ENFN->font_text_props_info_create(ENC, script_fi, W, &text_props,
                                          NULL, 0, 1,
                                          EVAS_TEXT_PROPS_MODE_NONE,
                                          fdesc->lang);
        advance = ENFN->font_h_advance_get(ENC, font, &text_props);
        vadvance = ENFN->font_v_advance_get(ENC, font, &text_props);
        ascent = ENFN->font_ascent_get(ENC, font);
        DBG("on font '%s', with alternate weight/slant %s, "
            "width: %d vs %d, height: %d vs %d, ascent: %d vs %d",
             fdesc->name, kind,
             o->cur.char_width, advance,
             o->cur.char_height, vadvance,
             o->ascent, ascent);
        if ((o->cur.char_width != advance) ||
            (o->cur.char_height != vadvance) ||
            (o->ascent != ascent))
          {
             evas_font_free(font);
          }
        else
          {
             *fontp = font;
             ret = 0;
          }
        evas_common_text_props_content_unref(&text_props);
     }
   else
     {
         DBG("cannot load font '%s' with alternate weight/slant %s",
             fdesc->name, kind);
     }
   return ret;
}

static void
_evas_textgrid_font_reload(Eo *eo_obj, Evas_Textgrid_Data *o)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;
   Evas_Font_Description *fdesc;
   Eina_List *was = NULL;

   fdesc = o->cur.font_description_normal;

   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = _evas_pointer_list_in_rect_get(obj->layer->evas, eo_obj, obj,
                                               1, 1);
     }

   if (o->font_normal)
     {
        evas_font_free(o->font_normal);
        o->font_normal = NULL;
     }

   o->font_normal = evas_font_load(obj->layer->evas->font_path,
                                   obj->layer->evas->hinting,
                                   o->cur.font_description_normal,
                                   o->cur.font_source,
                                   (int)(((double) o->cur.font_size) *
                                   obj->cur->scale),
                                   o->cur.bitmap_scalable);
   if (o->font_normal)
     {
        Eina_Unicode W[2] = { 'O', 0 };
        Evas_Font_Instance *script_fi = NULL;
        Evas_Font_Instance *cur_fi = NULL;
        Evas_Text_Props text_props;
        Evas_Script_Type script;
        int advance, vadvance;

        script = evas_common_language_script_type_get(W, 1);
        ENFN->font_run_end_get(ENC, o->font_normal, &script_fi, &cur_fi,
                               script, W, 1);
        memset(&text_props, 0, sizeof(Evas_Text_Props));
        evas_common_text_props_script_set(&text_props, script);
        ENFN->font_text_props_info_create(ENC, script_fi, W, &text_props,
                                          NULL, 0, 1,
                                          EVAS_TEXT_PROPS_MODE_NONE,
                                          fdesc->lang);
        advance = ENFN->font_h_advance_get(ENC, o->font_normal, &text_props);
        vadvance = ENFN->font_v_advance_get(ENC, o->font_normal, &text_props);
        o->cur.char_width = advance;
        o->cur.char_height = vadvance;
        o->ascent = ENFN->font_ascent_get(ENC, o->font_normal);
        evas_common_text_props_content_unref(&text_props);
     }
   else
     {
        EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
          {
             state_write->geometry.w = 0;
             state_write->geometry.h = 0;
          }
        EINA_COW_STATE_WRITE_END(obj, state_write, cur);

        o->ascent = 0;
     }

   DBG("font: '%s' weight: %d, slant: %d",
       fdesc->name, fdesc->weight, fdesc->slant);

   /* Bold */
   if (o->font_bold)
     {
        evas_font_free(o->font_bold);
        o->font_bold = NULL;
     }
   if ((fdesc->weight == EVAS_FONT_WEIGHT_NORMAL) ||
       (fdesc->weight == EVAS_FONT_WEIGHT_BOOK))
     {
        Evas_Font_Description *bold_desc = evas_font_desc_dup(fdesc);

        eina_stringshare_del(bold_desc->style);
        bold_desc->style = NULL;

        bold_desc->weight = EVAS_FONT_WEIGHT_BOLD;
        _alternate_font_weight_slant(obj, o, &o->font_bold, bold_desc,
                                     "bold");
        evas_font_desc_unref(bold_desc);
     }

   /* Italic */
   if (o->font_italic)
     {
        evas_font_free(o->font_italic);
        o->font_italic = NULL;
     }
   if (fdesc->slant == EVAS_FONT_SLANT_NORMAL)
     {
        Evas_Font_Description *italic_desc = evas_font_desc_dup(fdesc);
        int ret;

        eina_stringshare_del(italic_desc->style);
        italic_desc->style = NULL;

        italic_desc->slant = EVAS_FONT_SLANT_ITALIC;
        ret = _alternate_font_weight_slant(obj, o, &o->font_italic,
                                           italic_desc, "italic");
        if (ret != 0)
          {
             italic_desc->slant = EVAS_FONT_SLANT_OBLIQUE;
             _alternate_font_weight_slant(obj, o, &o->font_italic,
                                          italic_desc,
                                          "oblique");
          }
        evas_font_desc_unref(italic_desc);
     }

   /* BoldItalic */
   if (o->font_bolditalic)
     {
        evas_font_free(o->font_bolditalic);
        o->font_bolditalic = NULL;
     }
   if (fdesc->slant == EVAS_FONT_SLANT_NORMAL &&
       ((fdesc->weight == EVAS_FONT_WEIGHT_NORMAL) ||
        (fdesc->weight == EVAS_FONT_WEIGHT_BOOK)))
     {
        Evas_Font_Description *bolditalic_desc = evas_font_desc_dup(fdesc);
        int ret;

        eina_stringshare_del(bolditalic_desc->style);
        bolditalic_desc->style = NULL;

        bolditalic_desc->slant = EVAS_FONT_SLANT_ITALIC;
        bolditalic_desc->weight = EVAS_FONT_WEIGHT_BOLD;
        ret = _alternate_font_weight_slant(obj, o, &o->font_bolditalic,
                                           bolditalic_desc,
                                           "bolditalic");
        if (ret != 0)
          {
             bolditalic_desc->slant = EVAS_FONT_SLANT_OBLIQUE;
             _alternate_font_weight_slant(obj, o, &o->font_bolditalic,
                                          bolditalic_desc,
                                          "boldoblique");
          }
        evas_font_desc_unref(bolditalic_desc);
     }

   o->changed = 1;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   if (!obj->layer->evas->is_frozen && !pass && !freeze && obj->cur->visible)
     _evas_canvas_event_pointer_in_list_mouse_move_feed(obj->layer->evas, was, eo_obj, obj, 1, 1, EINA_TRUE, NULL);
   eina_list_free(was);
   evas_object_inform_call_resize(eo_obj, obj);
   o->changed = 1;
   o->core_change = 1;
   evas_object_textgrid_rows_clear(eo_obj);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_textgrid_efl_text_font_font_set(Eo *eo_obj,
                                            Evas_Textgrid_Data *o,
                                            const char *font_name,
                                            Evas_Font_Size font_size)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Font_Description *fdesc;

   if ((!font_name) || (!*font_name) || (font_size <= 0))
     return;

   evas_object_async_block(obj);
   fdesc = evas_font_desc_new();
   /* Set default language according to locale. */
   eina_stringshare_replace(&(fdesc->lang),
                            evas_font_lang_normalize("auto"));
   evas_font_name_parse(fdesc, font_name);
   if (o->cur.font_description_normal &&
       !evas_font_desc_cmp(fdesc, o->cur.font_description_normal) &&
       (font_size == o->cur.font_size))
     {
        evas_font_desc_unref(fdesc);
        return;
     }

   if (o->cur.font_description_normal)
     evas_font_desc_unref(o->cur.font_description_normal);
   o->cur.font_description_normal = fdesc;

   o->cur.font_size = font_size;
   eina_stringshare_replace(&o->cur.font_name, font_name);
   o->prev.font_name = NULL;

   _evas_textgrid_font_reload(eo_obj, o);
}

EOLIAN static void
_evas_textgrid_efl_text_font_font_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, const char **font_name, Evas_Font_Size *font_size)
{
   if (font_name) *font_name = o->cur.font_name;
   if (font_size) *font_size = o->cur.font_size;
}

EOLIAN static void
_evas_textgrid_cell_size_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int *w, int *h)
{
   if (w) *w = o->cur.char_width;
   if (h) *h = o->cur.char_height;
}

EOLIAN static void
_evas_textgrid_palette_set(Eo *eo_obj, Evas_Textgrid_Data *o, Evas_Textgrid_Palette pal, int idx, int r, int g, int b, int a)
{

   Eina_Array *palette;
   Evas_Object_Textgrid_Color *color, *c;
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int count, i;

   if ((idx < 0) || (idx > 255)) return;

   if (a > 255) a = 255;
   if (a < 0) a = 0;
   if (r > 255) r = 255;
   if (r < 0) r = 0;
   if (g > 255) g = 255;
   if (g < 0) g = 0;
   if (b > 255) b = 255;
   if (b < 0) b = 0;
   if (r > a)
     {
        r = a;
        ERR("Evas only handles pre multiplied colors!");
     }
   if (g > a)
     {
        g = a;
        ERR("Evas only handles pre multiplied colors!");
     }
   if (b > a)
     {
        b = a;
        ERR("Evas only handles pre multiplied colors!");
     }

   evas_object_async_block(obj);
   switch (pal)
     {
     case EVAS_TEXTGRID_PALETTE_STANDARD:
       palette = &(o->cur.palette_standard);
       break;
     case EVAS_TEXTGRID_PALETTE_EXTENDED:
       palette = &(o->cur.palette_extended);
       break;
     default:
       return;
     }

   count = eina_array_count(palette);
   if (idx < count)
     {
        color = eina_array_data_get(palette, idx);
        if (color->a == a &&
            color->r == r &&
            color->g == g &&
            color->b == b)
          return;
     }
   else
     {
        color = malloc(sizeof(Evas_Object_Textgrid_Color));
        if (!color) return;
     }

   color->a = a;
   color->r = r;
   color->g = g;
   color->b = b;

   if (idx < count) eina_array_data_set(palette, idx, color);
   else if (idx == count) eina_array_push(palette, color);
   else
     {
        for (i = count; i < idx; i++)
          {
             c = calloc(1, sizeof(Evas_Object_Textgrid_Color));
             if (!c)
               {
                  ERR("Evas cannot allocate memory");
                  free(color);
                  return;
               }
             eina_array_push(palette, c);
          }
        eina_array_push(palette, color);
     }
   o->changed = 1;
   o->pal_change = 1;
   evas_object_textgrid_rows_clear(eo_obj);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_textgrid_palette_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, Evas_Textgrid_Palette pal, int idx, int *r, int *g, int *b, int *a)
{
   Eina_Array *palette;
   Evas_Object_Textgrid_Color *color;

   if (idx < 0) return;

   switch (pal)
     {
      case EVAS_TEXTGRID_PALETTE_STANDARD:
        palette = &(((Evas_Textgrid_Data *)o)->cur.palette_standard);
        break;
      case EVAS_TEXTGRID_PALETTE_EXTENDED:
        palette = &(((Evas_Textgrid_Data *)o)->cur.palette_extended);
        break;
      default:
        return;
     }

   if (idx >= (int)eina_array_count(palette)) return;
   color = eina_array_data_get(palette, idx);
   if (!color) return;

   if (a) *a = color->a;
   if (r) *r = color->r;
   if (g) *g = color->g;
   if (b) *b = color->b;
}

EOLIAN static void
_evas_textgrid_supported_font_styles_set(Eo *eo_obj, Evas_Textgrid_Data *o, Evas_Textgrid_Font_Style styles)
{
   /* FIXME: to do */
   if (styles)
     {
        Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
        o->changed = 1;
        evas_object_change(eo_obj, obj);
     }
}

EOLIAN static Evas_Textgrid_Font_Style
_evas_textgrid_supported_font_styles_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o EINA_UNUSED)
{
   /* FIXME: to do */
   return EVAS_TEXTGRID_FONT_STYLE_NORMAL;
}

EOLIAN static void
_evas_textgrid_cellrow_set(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int y, const Evas_Textgrid_Cell *row)
{
   if (!row) return;

   if ((y < 0) || (y >= o->cur.h)) return;
}

EOLIAN static Evas_Textgrid_Cell*
_evas_textgrid_cellrow_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int y)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_async_block(obj);
   if ((y < 0) || (y >= o->cur.h)) return NULL;

   return o->cur.cells + (y * o->cur.w);
}

EOLIAN static void
_evas_textgrid_update_add(Eo *eo_obj, Evas_Textgrid_Data *o, int x, int y, int w, int h)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int i, x2;

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur.w, o->cur.h);
   if ((w <= 0) || (h <= 0)) return;

   evas_object_async_block(obj);
   x2 = x + w - 1;
   for (i = 0; i < h; i++)
     {
        Evas_Object_Textgrid_Row *r = &(o->cur.rows[y + i]);

        if (r->ch1 < 0)
          {
             evas_object_textgrid_row_clear(o, r);
             r->ch1 = x;
             r->ch2 = x2;
          }
        else
          {
             if (x < r->ch1) r->ch1 = x;
             if (x2 > r->ch2) r->ch2 = x2;
          }
     }
   o->row_change = 1;
   o->changed = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_textgrid_efl_object_dbg_info_get(Eo *eo_obj, Evas_Textgrid_Data *o EINA_UNUSED, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   Efl_Dbg_Info *group = EFL_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
   Efl_Dbg_Info *node;

   const char *text;
   int size;
   efl_text_font_get(eo_obj, &text, &size);
   EFL_DBG_INFO_APPEND(group, "Font", EINA_VALUE_TYPE_STRING, text);
   EFL_DBG_INFO_APPEND(group, "Text size", EINA_VALUE_TYPE_INT, size);

   text = efl_text_font_source_get(eo_obj);
   EFL_DBG_INFO_APPEND(group, "Font source", EINA_VALUE_TYPE_STRING, text);

     {
        int w, h;
        evas_obj_textgrid_grid_size_get(eo_obj, &w, &h);
        node = EFL_DBG_INFO_LIST_APPEND(group, "Grid size");
        EFL_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, w);
        EFL_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, h);
     }
}

EAPI void
evas_object_textgrid_font_source_set(Eo *obj, const char *font_source)
{
   efl_text_font_source_set((Eo *) obj, font_source);
}

EAPI const char *
evas_object_textgrid_font_source_get(const Eo *obj)
{
   const char *font_source = NULL;
   font_source = efl_text_font_source_get((Eo *) obj);
   return font_source;
}

EAPI void
evas_object_textgrid_font_set(Eo *obj, const char *font_name, Evas_Font_Size font_size)
{
   efl_text_font_set((Eo *) obj, font_name, font_size);
}

EAPI void
evas_object_textgrid_font_get(const Eo *obj, const char **font_name, Evas_Font_Size *font_size)
{
   efl_text_font_get((Eo *) obj, font_name, font_size);
}

EOLIAN static void
_evas_textgrid_efl_text_font_font_bitmap_scalable_set(Eo *eo_obj, Evas_Textgrid_Data *o, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   if (o->cur.bitmap_scalable == bitmap_scalable) return;
   o->prev.bitmap_scalable = o->cur.bitmap_scalable;
   o->cur.bitmap_scalable = bitmap_scalable;
   _evas_textgrid_font_reload(eo_obj, o);
}

EOLIAN static Efl_Text_Font_Bitmap_Scalable
_evas_textgrid_efl_text_font_font_bitmap_scalable_get(const Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o)
{
   return o->cur.bitmap_scalable;
}

#define EVAS_TEXTGRID_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _evas_textgrid_efl_object_dbg_info_get)

#include "canvas/evas_textgrid_eo.c"
