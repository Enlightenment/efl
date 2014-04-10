#include "evas_common_private.h" /* Includes evas_bidi_utils stuff. */
#include "evas_private.h"

#include "Eo.h"

#define MY_CLASS EVAS_OBJ_TEXTGRID_CLASS

#define MY_CLASS_NAME "Evas_Textgrid"

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

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
typedef struct _Evas_Textgrid_Hash_Master  Evas_Textgrid_Hash_Master;
typedef struct _Evas_Textgrid_Hash_Glyphs  Evas_Textgrid_Hash_Glyphs;

struct _Evas_Textgrid_Hash_Master
{
   int next[16];
};

struct _Evas_Textgrid_Hash_Glyphs
{
   Evas_Text_Props props[256];
};

struct _Evas_Textgrid_Data
{
   DATA32                         magic;

   struct {
      int                         w, h;
      int                         char_width;
      int                         char_height;
      Evas_Object_Textgrid_Row   *rows;
      Evas_Textgrid_Cell         *cells;

      const char                 *font_source;
      const char                 *font_name;
      Evas_Font_Size              font_size;
      Evas_Font_Description      *font_description;

      Eina_Array                  palette_standard;
      Eina_Array                  palette_extended;
   } cur, prev;

   int                            ascent;

   Evas_Font_Set                 *font;

   Evas_Textgrid_Hash_Master     *master;
   Evas_Textgrid_Hash_Glyphs     *glyphs;
   unsigned char                 *master_used;
   unsigned char                 *glyphs_used;
   unsigned int                   master_length;
   unsigned int                   glyphs_length;

   unsigned int                   last_mask;
   Evas_Textgrid_Hash_Glyphs     *last_glyphs;

   Eina_Array                     glyphs_cleanup;

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
   unsigned char r, g, b, a;
   int x;
   unsigned int text_props;
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
					void *output, void *context, void *surface,
					int x, int y, Eina_Bool do_async);
static void evas_object_textgrid_render_pre(Evas_Object *eo_obj,
					    Evas_Object_Protected_Data *obj,
					    void *type_private_data);
static void evas_object_textgrid_render_post(Evas_Object *eo_obj,
					     Evas_Object_Protected_Data *obj,
					     void *type_private_data);

static unsigned int evas_object_textgrid_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_textgrid_visual_id_get(Evas_Object *eo_obj);
static void *evas_object_textgrid_engine_data_get(Evas_Object *eo_obj);

static int evas_object_textgrid_is_opaque(Evas_Object *eo_obj,
					  Evas_Object_Protected_Data *obj,
					  void *type_private_data);
static int evas_object_textgrid_was_opaque(Evas_Object *eo_obj,
					   Evas_Object_Protected_Data *obj,
					   void *type_private_data);

static void evas_object_textgrid_scale_update(Evas_Object *eo_obj,
					      Evas_Object_Protected_Data *pd,
					      void *type_private_data);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
   evas_object_textgrid_render,
   evas_object_textgrid_render_pre,
   evas_object_textgrid_render_post,
   evas_object_textgrid_id_get,
   evas_object_textgrid_visual_id_get,
   evas_object_textgrid_engine_data_get,
   /* these are optional. NULL = nothing */
   NULL,
   NULL,
   NULL,
   NULL,
   evas_object_textgrid_is_opaque,
   evas_object_textgrid_was_opaque,
   NULL,
   NULL,
   NULL,
   evas_object_textgrid_scale_update,
   NULL,
   NULL,
   NULL
};

/* almost generic private array data type */
static int
evas_object_textgrid_textprop_get(Evas_Object *eo_obj, Evas_Textgrid_Data *o, Eina_Unicode codepoint,
                                  unsigned int glyphs_index, unsigned char *used)
{
   Evas_Textgrid_Hash_Glyphs *glyph;
   unsigned char idx = codepoint & 0xFF;

   glyph = &(o->glyphs[glyphs_index]);

   if (!glyph->props[idx].info)
     {
        Evas_Font_Instance *script_fi = NULL;
        Evas_Font_Instance *cur_fi = NULL;
        Evas_Script_Type script;

        script = evas_common_language_script_type_get(&codepoint, 1);
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        ENFN->font_run_end_get(ENDT, o->font, &script_fi, &cur_fi, script, &codepoint, 1);
        memset(&(glyph->props[idx]), 0, sizeof(Evas_Text_Props));
        evas_common_text_props_script_set(&(glyph->props[idx]), script);
        ENFN->font_text_props_info_create(ENDT, script_fi, &codepoint,
                                          &(glyph->props[idx]), NULL, 0, 1,
                                          EVAS_TEXT_PROPS_MODE_NONE);
        (*used)++;
     }
   else
     {
        evas_common_text_props_content_ref(&(glyph->props[idx]));
     }
   
   return glyphs_index << 8 | (unsigned int) idx;
}

static int
evas_object_textgrid_textprop_ref(Evas_Object *eo_obj, Evas_Textgrid_Data *o, Eina_Unicode codepoint)
{
   unsigned int mask = 0xF0000000;
   unsigned int shift = 28;
   unsigned int offset = 0;
   unsigned int glyphs_index;

   if (o->last_glyphs)
     {
        if ((o->last_mask) && ((o->last_mask & codepoint) == o->last_mask))
          goto end;
     }

   if (!o->master)
     {
        o->master = calloc(6, sizeof (Evas_Textgrid_Hash_Master));
        o->master_used = calloc(6, sizeof (unsigned char));
        o->glyphs = calloc(1, sizeof (Evas_Textgrid_Hash_Glyphs));
        o->glyphs_used = calloc(1, sizeof (unsigned char));
        if (!o->master_used)
          {
             free(o->master);
             o->master = NULL;
             free(o->master_used);
             o->master_used = NULL;
             free(o->glyphs);
             o->glyphs = NULL;
             free(o->glyphs_used);
             o->glyphs_used = NULL;
             return 0xFFFFFFFF;
          }

        while (shift > 8)
          {
             o->master[offset].next[(mask & codepoint) >> shift] = offset + 1;
             o->master_used[offset] = 1;
             offset++;
             shift -= 4;
             mask >>= 4;
          }

        o->glyphs_length = 1;
        o->master_length = 6;
        o->master[5].next[(codepoint & 0xF00) >> 8] = 0xFF000000;
        o->last_glyphs = o->glyphs;
        o->last_mask = codepoint & 0xFFFFFF00;

        goto end;
     }

   while ((shift > 8)
          && (o->master[offset].next[(codepoint & mask) >> shift] != 0))
     {
        offset = o->master[offset].next[(codepoint & mask) >> shift];
        mask >>= 4;
        shift -= 4;
     }

   if (shift > 8)
     {
        Evas_Textgrid_Hash_Master *tmp;
        unsigned char *tmp_used;
        int master_count;
        int count;
        int end;

        count = (shift - 8) / 4;
        master_count = o->master_length + count;

        /* FIXME: find empty entry */
        tmp = realloc(o->master, master_count * sizeof (Evas_Textgrid_Hash_Master));
        if (!tmp) return 0xFFFFFFFF;
        o->master = tmp;
        tmp_used = realloc(o->master_used, master_count);
        if (!tmp_used) return 0xFFFFFFFF;
        o->master_used = tmp_used;

        memset(o->master + o->master_length, 0, count * sizeof (Evas_Textgrid_Hash_Master));
        memset(o->master_used + o->master_length, 1, count);
        end = o->master_length;
        o->master_length = master_count;

        while (shift > 8)
          {
             o->master[offset].next[(mask & codepoint) >> shift] = end;
             o->master_used[offset] = 1;
             end++;
             offset = end;
             shift -= 4;
             mask >>= 4;
          }
        offset--;
     }
   if ((o->master[offset].next[(codepoint & mask) >> shift] == 0)
       || ((o->master[offset].next[(codepoint & mask) >> shift] & 0xFFFFFF) >= o->glyphs_length))
     {
        Evas_Textgrid_Hash_Glyphs *tmp;
        unsigned char *tmp_used;
        int count;

        /* FIXME: find empty entry */
        if (o->master[offset].next[(codepoint & mask) >> shift] == 0)
          count = o->glyphs_length + 1;
        else
          count = (o->master[offset].next[(codepoint & mask) >> shift] & 0xFFFFFF) + 1;
        tmp = realloc(o->glyphs, count * sizeof (Evas_Textgrid_Hash_Glyphs));
        if (!tmp) return 0xFFFFFFFF;
        o->glyphs = tmp;
        tmp_used = realloc(o->glyphs_used, count * sizeof (unsigned char));
        if (!tmp_used) return 0xFFFFFFFF;
        o->glyphs_used = tmp_used;

        // FIXME: What should we write when allocating more than one new entry?
        o->master[offset].next[(codepoint & mask) >> shift] = o->glyphs_length + 0xFF000000;

        memset(o->glyphs + o->glyphs_length, 0, (count - o->glyphs_length) * sizeof (Evas_Textgrid_Hash_Glyphs));
        memset(o->glyphs_used, 0, (count - o->glyphs_length) * sizeof(o->glyphs_used[0]));
        o->glyphs_length = count;
     }

   o->last_glyphs = o->glyphs + (o->master[offset].next[(codepoint & mask) >> shift] & 0xFFFFFF);
   o->last_mask = codepoint & 0xFFFFFF00;

 end:
   glyphs_index = o->last_glyphs - o->glyphs;
   return evas_object_textgrid_textprop_get(eo_obj, o, codepoint, glyphs_index,
                                            &(o->glyphs_used[glyphs_index]));
}

static void
evas_object_textgrid_textprop_unref(Evas_Textgrid_Data *o, unsigned int props_index)
{
   Evas_Text_Props *props;

   props = &(o->glyphs[props_index >> 8].props[props_index & 0xFF]);

   if (props->info)
     {
        if (props->info->refcount == 1)
          eina_array_push(&o->glyphs_cleanup,
                          (void *)((uintptr_t)props_index));
        else
          {
             evas_common_text_props_content_nofree_unref(props);
          }
     }
}

static Evas_Text_Props *
evas_object_textgrid_textprop_int_to(Evas_Textgrid_Data *o, int props)
{
   return &(o->glyphs[props >> 8].props[props & 0xFF]);
}

/* all nice and private */
static void
evas_object_textgrid_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = eo_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;

   Evas_Textgrid_Data *o = obj->private_data;
   o->magic = MAGIC_OBJ_TEXTGRID;
   o->prev = o->cur;
   eina_array_step_set(&o->cur.palette_standard, sizeof (Eina_Array), 16);
   eina_array_step_set(&o->cur.palette_extended, sizeof (Eina_Array), 16);
   eina_array_step_set(&o->glyphs_cleanup, sizeof (Eina_Array), 16);
}

static void
evas_object_textgrid_row_clear(Evas_Textgrid_Data *o, Evas_Object_Textgrid_Row *r)
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
          if (r->texts[i].text_props != 0xFFFFFFFF)
            {
               evas_object_textgrid_textprop_unref(o, r->texts[i].text_props);
               r->texts[i].text_props = 0xFFFFFFFF;
            }
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

   Evas_Textgrid_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o->cur.rows) return;
   for (i = 0; i < o->cur.h; i++)
     {
        evas_object_textgrid_row_clear(o, &(o->cur.rows[i]));
        o->cur.rows[i].ch1 = 0;
        o->cur.rows[i].ch2 = o->cur.w - 1;
     }
}

static void
evas_object_textgrid_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Object_Textgrid_Color *c;
   Evas_Textgrid_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   /* free obj */
   evas_object_textgrid_rows_clear(eo_obj);
   if (o->cur.rows) free(o->cur.rows);
   if (o->cur.font_name) eina_stringshare_del(o->cur.font_name);
   if (o->cur.font_source) eina_stringshare_del(o->cur.font_source);
   if (o->cur.font_description) evas_font_desc_unref(o->cur.font_description);
   if (o->font) evas_font_free(obj->layer->evas->evas, o->font);
   if (o->cur.cells) free(o->cur.cells);
   while ((c = eina_array_pop(&o->cur.palette_standard)))
     free(c);
   eina_array_flush(&o->cur.palette_standard);
   while ((c = eina_array_pop(&o->cur.palette_extended)))
     free(c);
   eina_array_flush(&o->cur.palette_extended);

   while (eina_array_count(&o->glyphs_cleanup) > 0)
     {
        Evas_Text_Props *prop;
        unsigned int props_index;

        props_index = (unsigned int) (intptr_t) eina_array_pop(&o->glyphs_cleanup);
        prop = &(o->glyphs[props_index >> 8].props[props_index & 0xFF]);

        evas_common_text_props_content_nofree_unref(prop);
        if (!prop->info)
          {
             o->glyphs_used[props_index >> 8]--;

             if (!o->glyphs_used[props_index >> 8])
               {
                  /* FIXME: cleanup the master tree */
               }
          }
     }
   eina_array_flush(&o->glyphs_cleanup);

   free(o->master);
   free(o->glyphs);
   free(o->master_used);
   free(o->glyphs_used);

   o->magic = 0;
}

EOLIAN static void
_evas_textgrid_eo_base_destructor(Eo *eo_obj, Evas_Textgrid_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_textgrid_free(eo_obj, obj);
   eo_data_unref(eo_obj, obj->private_data);
   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void
evas_object_textgrid_row_rect_append(Evas_Object_Textgrid_Row *row, int x, int w, int r, int g, int b, int a)
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

static void
evas_object_textgrid_row_text_append(Evas_Object_Textgrid_Row *row, Evas_Object *eo_obj, Evas_Textgrid_Data *o, int x, Eina_Unicode codepoint, int r, int g, int b, int a)
{
   unsigned int text_props_index;

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

   text_props_index = evas_object_textgrid_textprop_ref(eo_obj, o, codepoint);

   row->texts[row->texts_num - 1].text_props = text_props_index;
   row->texts[row->texts_num - 1].x = x;
   row->texts[row->texts_num - 1].r = r;
   row->texts[row->texts_num - 1].g = g;
   row->texts[row->texts_num - 1].b = b;
   row->texts[row->texts_num - 1].a = a;
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
evas_object_textgrid_render(Evas_Object *eo_obj,
			    Evas_Object_Protected_Data *obj,
			    void *type_private_data,
			    void *output, void *context, void *surface, int x, int y, Eina_Bool do_async)
{
   Evas_Textgrid_Cell *cells;
   Evas_Object_Textgrid_Color *c;
   Eina_Array *palette;
   int xx, yy, xp, yp, w, h, ww, hh;
   int rr = 0, rg = 0, rb = 0, ra = 0, rx = 0, rw = 0, run;

   /* render object to surface with context, and offset by x,y */
   Evas_Textgrid_Data *o = type_private_data;
   ENFN->context_multiplier_unset(output, context);
   ENFN->context_render_op_set(output, context, obj->cur->render_op);

   if (!(o->font) || (!o->cur.cells)) return;

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
                       evas_object_textgrid_row_text_append(row, eo_obj, o, xp,
                                                            cells->codepoint,
                                                            c->r, c->g, c->b, c->a);
                       // XXX: underlines and strikethroughs dont get
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
             ENFN->context_color_set(output, context,
                                     row->rects[xx].r, row->rects[xx].g,
                                     row->rects[xx].b, row->rects[xx].a);
             ENFN->rectangle_draw(output, context, surface,
                                  xp + row->rects[xx].x, yp,
                                  row->rects[xx].w, h,
                                  do_async);
          }

        if (row->texts_num)
          {
             if ((do_async) && (ENFN->multi_font_draw))
               {
                  Eina_Bool async_unref;
		  Evas_Font_Array_Data *fad;

                  texts = malloc(sizeof(*texts));
                  texts->array = eina_inarray_new(sizeof(Evas_Font_Array_Data), 1); /* FIXME: Wasting 1 int here */
                  texts->refcount = 1;

		  fad = eina_inarray_grow(texts->array, row->texts_num);
		  if (!fad)
		    {
                       ERR("Failed to allocate Evas_Font_Array_Data.");
                       free(texts);
                       return;
		    }

                  for (xx = 0; xx < row->texts_num; xx++)
                    {
                       Evas_Text_Props     *props;

                       props =
                         evas_object_textgrid_textprop_int_to
                         (o, row->texts[xx].text_props);

                       evas_common_font_draw_prepare(props);

                       evas_common_font_glyphs_ref(props->glyphs);
                       evas_unref_queue_glyph_put(obj->layer->evas,
                                                  props->glyphs);

                       fad->color.r = row->texts[xx].r;
                       fad->color.g = row->texts[xx].g;
                       fad->color.b = row->texts[xx].b;
                       fad->color.a = row->texts[xx].a;
                       fad->x = row->texts[xx].x;
                       fad->glyphs = props->glyphs;

		       fad++;
                    }

                  async_unref =
                    ENFN->multi_font_draw(output, context, surface,
                                          o->font, xp, yp + o->ascent,
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
             else
               {
                  for (xx = 0; xx < row->texts_num; xx++)
                    {
                       Evas_Text_Props *props;
                       unsigned int     r, g, b, a;
                       int              tx = xp + row->texts[xx].x;
                       int              ty = yp + o->ascent;

                       props =
                         evas_object_textgrid_textprop_int_to
                         (o, row->texts[xx].text_props);

                       r = row->texts[xx].r;
                       g = row->texts[xx].g;
                       b = row->texts[xx].b;
                       a = row->texts[xx].a;

                       ENFN->context_color_set(output, context,
                                               r, g, b, a);
                       evas_font_draw_async_check(obj, output, context, surface,
                                                  o->font, tx, ty, ww, hh,
                                                  ww, hh, props, do_async);
                    }
               }
          }

        for (xx = 0; xx < row->lines_num; xx++)
          {
             ENFN->context_color_set(output, context,
                                     row->lines[xx].r, row->lines[xx].g,
                                     row->lines[xx].b, row->lines[xx].a);
             ENFN->rectangle_draw(output, context, surface,
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

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add thr appropriate redraw rectangles */

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
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj, obj);
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
   /* if we restacked (layer or just within a layer) and dont clip anyone */
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
   if (obj->cur->scale != obj->prev->scale)
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
evas_object_textgrid_render_post(Evas_Object *eo_obj,
				 Evas_Object_Protected_Data *obj EINA_UNUSED,
				 void *type_private_data)
{
   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   Evas_Textgrid_Data *o = type_private_data;
   /* remove those pesky changes */
   evas_object_clip_changes_clean(eo_obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(eo_obj);
   o->prev = o->cur;

   while (eina_array_count(&o->glyphs_cleanup) > 0)
     {
        Evas_Text_Props *prop;
        unsigned int props_index;

        props_index = (unsigned int) (intptr_t) eina_array_pop(&o->glyphs_cleanup);
        prop = &(o->glyphs[props_index >> 8].props[props_index & 0xFF]);
        
        evas_common_text_props_content_nofree_unref(prop);
        if (!prop->info)
          {
             o->glyphs_used[props_index >> 8]--;

             if (!o->glyphs_used[props_index >> 8])
               {
                  /* FIXME: cleanup the master tree */
               }
          }
     }
}

static unsigned int
evas_object_textgrid_id_get(Evas_Object *eo_obj)
{
   Evas_Textgrid_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_TEXTGRID;
}

static unsigned int
evas_object_textgrid_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Textgrid_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *
evas_object_textgrid_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Textgrid_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->font;
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

static void
evas_object_textgrid_scale_update(Evas_Object *eo_obj,
				  Evas_Object_Protected_Data *pd EINA_UNUSED,
				  void *type_private_data)
{
   int font_size;
   const char *font_name;

   Evas_Textgrid_Data *o = type_private_data;
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
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = eo_add(EVAS_OBJ_TEXTGRID_CLASS, e);
   eo_unref(eo_obj);
   return eo_obj;
}

EOLIAN static void
_evas_textgrid_eo_base_constructor(Eo *eo_obj, Evas_Textgrid_Data *class_data EINA_UNUSED)
{
   Eo *eo_parent = NULL;

   eo_do_super(eo_obj, MY_CLASS, eo_constructor());

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_textgrid_init(eo_obj);

   eo_do(eo_obj, eo_parent = eo_parent_get());
   evas_object_inject(eo_obj, obj, evas_object_evas_get(eo_parent));
}

EOLIAN static void
_evas_textgrid_size_set(Eo *eo_obj, Evas_Textgrid_Data *o, int w, int h)
{
   int i;

   if ((h <= 0) || (w <= 0)) return;

   if ((o->cur.w == w) && (o->cur.h == h)) return;

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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_textgrid_size_get(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int *w, int *h)
{
   if (w) *w = o->cur.w;
   if (h) *h = o->cur.h;
}

EOLIAN static void
_evas_textgrid_font_source_set(Eo *eo_obj, Evas_Textgrid_Data *o, const char *font_source)
{

   if ((!font_source) || (!*font_source))
     return;

   if ((o->cur.font_source) && (font_source) &&
       (!strcmp(o->cur.font_source, font_source))) return;

   eina_stringshare_replace(&o->cur.font_source, font_source);
   o->changed = 1;
   o->core_change = 1;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static const char*
_evas_textgrid_font_source_get(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o)
{
   return o->cur.font_source;
}

EOLIAN static void
_evas_textgrid_font_set(Eo *eo_obj, Evas_Textgrid_Data *o, const char *font_name, Evas_Font_Size font_size)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   Eina_Bool is, was = EINA_FALSE;
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;
   Evas_Font_Description *font_description;
   
   if ((!font_name) || (!*font_name) || (font_size <= 0))
     return;

   font_description = evas_font_desc_new();
   evas_font_name_parse(font_description, font_name);
   if (o->cur.font_description &&
       !evas_font_desc_cmp(font_description, o->cur.font_description) &&
       (font_size == o->cur.font_size))
     {
        evas_font_desc_unref(font_description);
        return;
     }

   if (o->cur.font_description) evas_font_desc_unref(o->cur.font_description);
   o->cur.font_description = font_description;

   o->cur.font_size = font_size;
   eina_stringshare_replace(&o->cur.font_name, font_name);
   o->prev.font_name = NULL;

   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = evas_object_is_in_output_rect(eo_obj, obj,
                                              obj->layer->evas->pointer.x,
                                              obj->layer->evas->pointer.y,
                                              1, 1);
     }
   
   /* DO IT */
   if (o->font)
     {
        evas_font_free(obj->layer->evas->evas, o->font);
        o->font = NULL;
     }
   
   o->font = evas_font_load(obj->layer->evas->evas, o->cur.font_description,
                            o->cur.font_source,
                            (int)(((double) o->cur.font_size) * 
                                  obj->cur->scale));
   if (o->font)
     {
        Eina_Unicode W[2] = { 'O', 0 };
        Evas_Font_Instance *script_fi = NULL;
        Evas_Font_Instance *cur_fi = NULL;
        Evas_Text_Props text_props;
        Evas_Script_Type script;
        int advance, vadvance;
        
        script = evas_common_language_script_type_get(W, 1);
        ENFN->font_run_end_get(ENDT, o->font, &script_fi, &cur_fi,
                               script, W, 1);
        memset(&text_props, 0, sizeof(Evas_Text_Props));
        evas_common_text_props_script_set(&text_props, script);
        ENFN->font_text_props_info_create(ENDT, script_fi, W, &text_props,
                                          NULL, 0, 1,
                                          EVAS_TEXT_PROPS_MODE_NONE);
        advance = ENFN->font_h_advance_get(ENDT, o->font, &text_props);
        vadvance = ENFN->font_v_advance_get(ENDT, o->font, &text_props);
        o->cur.char_width = advance;
        o->cur.char_height = vadvance;
        o->ascent = ENFN->font_ascent_get(ENDT, o->font);;
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

   o->changed = 1;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   if (!(obj->layer->evas->is_frozen))
     {
        if ((!pass) && (!freeze))
          {
             is = evas_object_is_in_output_rect(eo_obj, obj,
                                                obj->layer->evas->pointer.x,
                                                obj->layer->evas->pointer.y,
                                                1, 1);
             if ((is ^ was) && obj->cur->visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
   evas_object_inform_call_resize(eo_obj);
   o->changed = 1;
   o->core_change = 1;
   evas_object_textgrid_rows_clear(eo_obj);
   evas_object_change(eo_obj, obj);

   /* Force destroy of all cached Evas_Text_Props */
   while (eina_array_count(&o->glyphs_cleanup) > 0)
     {
        Evas_Text_Props *prop;
        unsigned int props_index;

        props_index = (unsigned int) (intptr_t) eina_array_pop(&o->glyphs_cleanup);
        prop = &(o->glyphs[props_index >> 8].props[props_index & 0xFF]);
        
        evas_common_text_props_content_nofree_unref(prop);
        if (!prop->info)
          {
             o->glyphs_used[props_index >> 8]--;

             if (!o->glyphs_used[props_index >> 8])
               {
                  /* FIXME: cleanup the master tree */
               }
          }
     }
}

EOLIAN static void
_evas_textgrid_font_get(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, const char **font_name, Evas_Font_Size *font_size)
{
   if (font_name) *font_name = o->cur.font_name;
   if (font_size) *font_size = o->cur.font_size;
}

EOLIAN static void
_evas_textgrid_cell_size_get(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int *w, int *h)
{
   if (w) *w = o->cur.char_width;
   if (h) *h = o->cur.char_height;
}

EOLIAN static void
_evas_textgrid_palette_set(Eo *eo_obj, Evas_Textgrid_Data *o, Evas_Textgrid_Palette pal, int idx, int r, int g, int b, int a)
{

   Eina_Array *palette;
   Evas_Object_Textgrid_Color *color, *c;
   int count, i;

   if ((idx < 0) || (idx > 255)) return;

   if (a > 255) a = 255; if (a < 0) a = 0;
   if (r > 255) r = 255; if (r < 0) r = 0;
   if (g > 255) g = 255; if (g < 0) g = 0;
   if (b > 255) b = 255; if (b < 0) b = 0;
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
                  ERR("Evas can not allocate memory");
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_textgrid_palette_get(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, Evas_Textgrid_Palette pal, int idx, int *r, int *g, int *b, int *a)
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
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
        o->changed = 1;
        evas_object_change(eo_obj, obj);
     }
}

EOLIAN static Evas_Textgrid_Font_Style
_evas_textgrid_supported_font_styles_get(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o EINA_UNUSED)
{
   /* FIXME: to do */
   return 0;
}

EOLIAN static void
_evas_textgrid_cellrow_set(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int y, const Evas_Textgrid_Cell *row)
{
   if (!row) return;

   if ((y < 0) || (y >= o->cur.h)) return;
}

EOLIAN static Evas_Textgrid_Cell*
_evas_textgrid_cellrow_get(Eo *eo_obj EINA_UNUSED, Evas_Textgrid_Data *o, int y)
{
   Evas_Textgrid_Cell *ret;
   if ((y < 0) || (y >= o->cur.h)) ret = NULL;

   ret = o->cur.cells + (y * o->cur.w);

   return ret;
}

EOLIAN static void
_evas_textgrid_update_add(Eo *eo_obj, Evas_Textgrid_Data *o, int x, int y, int w, int h)
{
   int i, x2;
   
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, o->cur.w, o->cur.h);
   if ((w <= 0) || (h <= 0)) return;
   
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_textgrid_eo_base_dbg_info_get(Eo *eo_obj, Evas_Textgrid_Data *o EINA_UNUSED, Eo_Dbg_Info *root)
{
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
   Eo_Dbg_Info *node;

   const char *text;
   int size;
   eo_do(eo_obj, evas_obj_textgrid_font_get(&text, &size));
   EO_DBG_INFO_APPEND(group, "Font", EINA_VALUE_TYPE_STRING, text);
   EO_DBG_INFO_APPEND(group, "Text size", EINA_VALUE_TYPE_INT, size);

   eo_do(eo_obj, text = evas_obj_textgrid_font_source_get());
   EO_DBG_INFO_APPEND(group, "Font source", EINA_VALUE_TYPE_STRING, text);

     {
        int w, h;
        eo_do(eo_obj, evas_obj_textgrid_size_get(&w, &h));
        node = EO_DBG_INFO_LIST_APPEND(group, "Grid size");
        EO_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, w);
        EO_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, h);
     }
}

#include "canvas/evas_textgrid.eo.c"
