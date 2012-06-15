/*

$  : implemented
$* : and is declared in Evas.h and commented

$*  EAPI Evas_Object *evas_object_textgrid_add(Evas *e);

$*  EAPI void evas_object_textgrid_size_set(Evas_Object *o, int w, int h);

$*  EAPI void evas_object_textgrid_size_get(Evas_Object *o, int *w, int *h);

$*  EAPI void evas_object_textgrid_palette_set(Evas_Object *o, Evas_Palette pal, int n, int r, int g, int b, int a);

$*  EAPI void evas_object_textgrid_palette_get(Evas_Object *o, Evas_Palette pal, int n, int *r, int *g, int *b, int *a);

$*  EAPI void evas_object_textgrid_cellrow_set(Evas_Object *o, int y, Evas_Text_Cell *row);

$*  EAPI Evas_Text_Cell *evas_object_textgrid_cellrow_get(Evas_Object *o, int y);

  EAPI void evas_object_textgrid_update_add(Evas_Object *o, int x, int y, int w, int h);

$*  EAPI void evas_object_textgrid_font_source_set(Evas_Object *o, const char *font);

$*  EAPI const char *evas_object_textgrid_font_source_get(Evas_Object *o);

$*  EAPI void evas_object_textgrid_font_set(Evas_Object *o, const char *font, Evas_Font_Size size);

$*  EAPI void evas_object_textgrid_font_get(Evas_Object *o, const char **font, Evas_Font_Size *size);

$*  EAPI void evas_object_textgrid_cell_size_get(Evas_Object *o, int *w, int *h);

$*  EAPI Evas_Font_Style evas_object_textgrid_supported_font_styles_get(Evas_Object *o);

$*  EAPI void evas_object_textgrid_supported_font_styles_set(Evas_Object *o, Evas_Font_Style fstyles);

 */

#include "evas_common.h" /* Includes evas_bidi_utils stuff. */
#include "evas_private.h"

/* save typing */
#define TG_ENFN obj->layer->evas->engine.func
#define TG_ENDT obj->layer->evas->engine.data.output

/* private magic number for text objects */
static const char o_type[] = "textgrid";

/* private struct for line object internal data */
typedef struct _Evas_Object_Textgrid       Evas_Object_Textgrid;
typedef struct _Evas_Object_Textgrid_Cell  Evas_Object_Textgrid_Cell;
typedef struct _Evas_Object_Textgrid_Color Evas_Object_Textgrid_Color;

struct _Evas_Object_Textgrid
{
   DATA32                         magic;

   struct {
      int                         nbr_lines;
      int                         nbr_columns;
      int                         char_width;
      int                         char_height;
      Evas_Textgrid_Cell  *cells;

      const char                 *font_source;
      const char                 *font_name;
      Evas_Font_Size              font_size;
      Evas_Font_Description      *font_description;

      Eina_Array                 *palette_standard;
      Eina_Array                 *palette_extended;
   } cur, prev;

   float                          max_ascent;

   Evas_Font_Set                 *font;

   unsigned int                   changed_size : 1;
   unsigned int                   changed      : 1;
};

struct _Evas_Object_Textgrid_Color
{
   unsigned char a;
   unsigned char r;
   unsigned char g;
   unsigned char b;
};

/* private methods for textgrid objects */
static void evas_object_textgrid_init(Evas_Object *obj);
static void *evas_object_textgrid_new(void);
static void evas_object_textgrid_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
static void evas_object_textgrid_free(Evas_Object *obj);
static void evas_object_textgrid_render_pre(Evas_Object *obj);
static void evas_object_textgrid_render_post(Evas_Object *obj);

static unsigned int evas_object_textgrid_id_get(Evas_Object *obj);
static unsigned int evas_object_textgrid_visual_id_get(Evas_Object *obj);
static void *evas_object_textgrid_engine_data_get(Evas_Object *obj);

static int evas_object_textgrid_is_opaque(Evas_Object *obj);
static int evas_object_textgrid_was_opaque(Evas_Object *obj);

static void evas_object_textgrid_scale_update(Evas_Object *obj);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   evas_object_textgrid_free,
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

/* all nice and private */
static void
evas_object_textgrid_init(Evas_Object *obj)
{
   /* alloc textgrid ob, setup methods and default values */
   obj->object_data = evas_object_textgrid_new();
   /* set up default settings for this kind of object */
   obj->cur.color.r = 255;
   obj->cur.color.g = 255;
   obj->cur.color.b = 255;
   obj->cur.color.a = 255;
   obj->cur.geometry.x = 0;
   obj->cur.geometry.y = 0;
   obj->cur.geometry.w = 0;
   obj->cur.geometry.h = 0;
   obj->cur.layer = 0;
   /* set up object-specific settings */
   obj->prev = obj->cur;
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->type = o_type;
}

static void *
evas_object_textgrid_new(void)
{
   Evas_Object_Textgrid *o;

   /* FIXME: using evas mempool like text ? */

   /* alloc obj private data */
   o = calloc(1, sizeof(Evas_Object_Textgrid));
   o->magic = MAGIC_OBJ_TEXTGRID;
   o->prev = o->cur;
   o->cur.palette_standard = eina_array_new(16);
   o->cur.palette_extended = eina_array_new(16);
   return o;
}

static void
evas_object_textgrid_free(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;
   unsigned int i;

   /* frees private object data. very simple here */
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

   /* free obj */
   if (o->cur.font_name) eina_stringshare_del(o->cur.font_name);
   if (o->cur.font_source) eina_stringshare_del(o->cur.font_source);
   if (o->cur.font_description) evas_font_desc_unref(o->cur.font_description);
   if (o->font) evas_font_free(obj->layer->evas, o->font);
   if (o->cur.cells) free(o->cur.cells);
   for (i = 0; i < eina_array_count(o->cur.palette_standard); i++)
     free(eina_array_data_get(o->cur.palette_standard, i));
   eina_array_free(o->cur.palette_standard);
   for (i = 0; i < eina_array_count(o->cur.palette_extended); i++)
     free(eina_array_data_get(o->cur.palette_extended, i));
   eina_array_free(o->cur.palette_extended);
   o->magic = 0;
   /* FIXME: using evas mempool like text ? */
   free(o);
}

static void
evas_object_textgrid_render(Evas_Object *obj, void *output, void *context, void *surface, int x, int y)
{
   Evas_Object_Textgrid *o;

   /* render object to surface with context, and offxet by x,y */
   o = (Evas_Object_Textgrid *)(obj->object_data);
   TG_ENFN->context_multiplier_unset(output, context);
   TG_ENFN->context_render_op_set(output, context, obj->cur.render_op);

   if (o->font && o->cur.cells)
     {
        Evas_Textgrid_Cell  *cells;
        int i;
        int j;

        /* Doing char by char because each char may have a different attribute */
        for (i = 0, cells = o->cur.cells; i < o->cur.nbr_lines; i++)
          for (j = 0; j < o->cur.nbr_columns; j++, cells++)
            {
               Evas_Object_Textgrid_Color *c;
               Eina_Array *palette;
               unsigned char pal;

               Evas_Font_Instance *script_fi = NULL;
               Evas_Font_Instance *cur_fi = NULL;
               Evas_Text_Props text_props;
               Evas_Script_Type script;
               int run_len;

//               if (cells->codepoint)
//                 printf("cell %dx%d: [%i] %c\n", j, i, cells->codepoint, (char)cells->codepoint);

               /* background */
               if (cells->bg_extended)
                 palette = o->cur.palette_extended;
               else
                 palette = o->cur.palette_standard;

               c = eina_array_data_get(palette, cells->bg);
               if (!c)
                 continue;

               if (c->a > 0)
                 {
                    TG_ENFN->context_color_set(output, context, c->r, c->g, c->b, c->a);
                    TG_ENFN->rectangle_draw(output,
                                            context,
                                            surface,
                                            obj->cur.geometry.x + x + j * o->cur.char_width,
                                            obj->cur.geometry.y + y + i * o->cur.char_height,
                                            o->cur.char_width,
                                            o->cur.char_height);
                 }

               if (!cells->codepoint) continue;
               /* foreground */
               if (cells->fg_extended)
                 palette = o->cur.palette_extended;
               else
                 palette = o->cur.palette_standard;

               c = eina_array_data_get(palette, cells->fg);
               if (!c)
                 continue;
               
               if (c->a == 0) continue;
               /* FIXME: manage attributes */
               script = evas_common_language_script_type_get((const Eina_Unicode *)&cells->codepoint, 1);
               run_len = TG_ENFN->font_run_end_get(TG_ENDT,
                                                   o->font,
                                                   &script_fi, &cur_fi,
                                                   script, &cells->codepoint, 1);
               memset(&text_props, 0, sizeof(Evas_Text_Props));
               evas_common_text_props_script_set(&text_props,
                                                 script);
               TG_ENFN->font_text_props_info_create(TG_ENDT,
                                                    script_fi,
                                                    &cells->codepoint,
                                                    &text_props,
                                                    NULL,
                                                    0, 1,
                                                    EVAS_TEXT_PROPS_MODE_NONE);

               TG_ENFN->context_color_set(output, context, c->r, c->g, c->b, c->a);
               TG_ENFN->font_draw(output,
                                  context,
                                  surface,
                                  o->font,
                                  obj->cur.geometry.x + x + j * o->cur.char_width,
                                  obj->cur.geometry.y + y + i * o->cur.char_height + (int)o->max_ascent,
                                  obj->cur.geometry.w,
                                  obj->cur.geometry.h,
                                  obj->cur.geometry.w,
                                  obj->cur.geometry.h,
                                  &text_props);

               /* FIXME: for these 2 flags, must be over the char and  not the cell ? */
               if (cells->underline)
                 {
                    TG_ENFN->context_color_set(output, context, c->r, c->g, c->b, c->a);
                    TG_ENFN->rectangle_draw(output,
                                            context,
                                            surface,
                                            obj->cur.geometry.x + x + j * o->cur.char_width,
                                            obj->cur.geometry.y + y + i * o->cur.char_height + (int)o->max_ascent + 1,
                                            o->cur.char_width,
                                            1);
                 }
               if (cells->strikethrough)
                 {
                    TG_ENFN->context_color_set(output, context, c->r, c->g, c->b, c->a);
                    TG_ENFN->rectangle_draw(output,
                                            context,
                                            surface,
                                            obj->cur.geometry.x + x + j * o->cur.char_width,
                                            obj->cur.geometry.y + y + i * o->cur.char_height + ((3 * (int)o->max_ascent) >> 2),
                                            o->cur.char_width,
                                            1);
                 }
            }
     }
}

static void
evas_object_textgrid_render_pre(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;
   int is_v, was_v;

   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = 1;
   /* pre-render phase. this does anything an object needs to do just before */
   /* rendering. this could mean loading the image data, retrieving it from */
   /* elsewhere, decoding video etc. */
   /* then when this is done the object needs to figure if it changed and */
   /* if so what and where and add thr appropriate redraw rectangles */
   o = (Evas_Object_Textgrid *)(obj->object_data);
/*    if (o && o->font && o->cur.cells) */
/*      { */
/*         unsigned int i; */
/*         unsigned int j; */
/*         unsigned int idx; */

/*         for (i = 0, idx = 0; i < o->cur.nbr_lines; i++) */
/*           for (j = 0; j < o->cur.nbr_columns; j++, idx++) */
/*             { */
/*               if (!o->cur.cells[idx].color.fg.changed) */
/*                 { */
/*                   o->cur.cells[idx].color.fg.r = o->cur.color.fg.r; */
/*                   o->cur.cells[idx].color.fg.g = o->cur.color.fg.g; */
/*                   o->cur.cells[idx].color.fg.b = o->cur.color.fg.b; */
/*                   o->cur.cells[idx].color.fg.a = o->cur.color.fg.a; */
/*                 } */
/*               if (!o->cur.cells[idx].color.bg.changed) */
/*                 { */
/*                   o->cur.cells[idx].color.bg.r = o->cur.color.bg.r; */
/*                   o->cur.cells[idx].color.bg.g = o->cur.color.bg.g; */
/*                   o->cur.cells[idx].color.bg.b = o->cur.color.bg.b; */
/*                   o->cur.cells[idx].color.bg.a = o->cur.color.bg.a; */
/*                 } */
/*             } */
/*      } */
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur.clipper)
     {
	if (obj->cur.cache.clip.dirty)
	  evas_object_clip_recalc(obj->cur.clipper);
	obj->cur.clipper->func->render_pre(obj->cur.clipper);
     }
   /* now figure what changed and add draw rects */
   /* if it just became visible or invisible */
   is_v = evas_object_is_visible(obj);
   was_v = evas_object_was_visible(obj);
   if (is_v != was_v)
     {
	evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes, obj, is_v, was_v);
	goto done;
     }
   if (obj->changed_map)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            obj);
        goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, obj);
   /* if we restacked (layer or just within a layer) and dont clip anyone */
   if (obj->restack)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }
   /* if it changed color */
   if ((obj->cur.color.r != obj->prev.color.r) ||
       (obj->cur.color.g != obj->prev.color.g) ||
       (obj->cur.color.b != obj->prev.color.b) ||
       (obj->cur.color.a != obj->prev.color.a))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }
   /* if it changed geometry - and obviously not visibility or color */
   /* calculate differences since we have a constant color fill */
   /* we really only need to update the differences */
   if ((obj->cur.geometry.x != obj->prev.geometry.x) ||
       (obj->cur.geometry.y != obj->prev.geometry.y) ||
       (obj->cur.geometry.w != obj->prev.geometry.w) ||
       (obj->cur.geometry.h != obj->prev.geometry.h))
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }
   if (obj->cur.render_op != obj->prev.render_op)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }
   if (obj->cur.scale != obj->prev.scale)
     {
	evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
	goto done;
     }

   if (o->changed)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
        goto done;
     }
/*   
        Evas_Textgrid_Cell *cell_cur;
        Evas_Textgrid_Cell *cell_prev;
        unsigned int i;
        unsigned int j;

        if ((o->cur.nbr_lines != o->prev.nbr_lines) ||
            (o->cur.nbr_columns != o->prev.nbr_columns) ||
	    (o->cur.font_size != o->prev.font_size) ||
	    ((o->cur.font_name) && (o->prev.font_name) &&
             (strcmp(o->cur.font_name, o->prev.font_name))) ||
	    ((o->cur.font_name) && (!o->prev.font_name)) ||
	    ((!o->cur.font_name) && (o->prev.font_name)))
	  {
	     evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
						 obj);
	     goto done;
	  }
        // if it changed palettes
        if (eina_array_count(o->cur.palette_standard) != eina_array_count(o->prev.palette_standard))
          {
             evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
             goto done;
          }
        for (i = 0; i < eina_array_count(o->cur.palette_standard); i++)
          {
             Evas_Object_Textgrid_Color *c_cur;
             Evas_Object_Textgrid_Color *c_prev;

             c_cur = eina_array_data_get(o->cur.palette_standard, i);
             c_prev = eina_array_data_get(o->prev.palette_standard, i);
             if ((c_cur->a != c_prev->a) ||
                 (c_cur->r != c_prev->r) ||
                 (c_cur->g != c_prev->g) ||
                 (c_cur->b != c_prev->b))
               {
                  evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
                  goto done;
               }
          }
        if (eina_array_count(o->cur.palette_extended) != eina_array_count(o->prev.palette_extended))
          {
             evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
             goto done;
          }
        for (i = 0; i < eina_array_count(o->cur.palette_extended); i++)
          {
             Evas_Object_Textgrid_Color *c_cur;
             Evas_Object_Textgrid_Color *c_prev;

             c_cur = eina_array_data_get(o->cur.palette_extended, i);
             c_prev = eina_array_data_get(o->prev.palette_extended, i);
             if ((c_cur->a != c_prev->a) ||
                 (c_cur->r != c_prev->r) ||
                 (c_cur->g != c_prev->g) ||
                 (c_cur->b != c_prev->b))
               {
                  evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
                  goto done;
               }
          }

        // if it changed a cell
        for (i = 0, cell_cur = o->cur.cells, cell_prev = o->prev.cells; i < o->cur.nbr_lines; i++)
          for (j = 0; j < o->cur.nbr_columns; j++, cell_cur++, cell_prev++)
            {
               if ((cell_cur->codepoint != cell_prev->codepoint) ||
                   (cell_cur->fg != cell_prev->fg) ||
                   (cell_cur->bg != cell_prev->bg) ||
                   (cell_cur->bold != cell_prev->bold) ||
                   (cell_cur->italic != cell_prev->italic) ||
                   (cell_cur->underline != cell_prev->underline) ||
                   (cell_cur->strikethrough != cell_prev->strikethrough) ||
                   (cell_cur->fg_extended != cell_prev->fg_extended) ||
                   (cell_cur->bg_extended != cell_prev->bg_extended))
                 {
                    evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes, obj);
                    goto done;
                 }
            }
     }
 */
   done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes, obj, is_v, was_v);
}

static void
evas_object_textgrid_render_post(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;

   /* this moves the current data to the previous state parts of the object */
   /* in whatever way is safest for the object. also if we don't need object */
   /* data anymore we can free it if the object deems this is a good idea */
   o = (Evas_Object_Textgrid *)(obj->object_data);
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   obj->prev = obj->cur;
   o->prev = o->cur;
   o->changed = 0;
}

static unsigned int
evas_object_textgrid_id_get(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;

   o = (Evas_Object_Textgrid *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_TEXTGRID;
}

static unsigned int
evas_object_textgrid_visual_id_get(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;

   o = (Evas_Object_Textgrid *)(obj->object_data);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *
evas_object_textgrid_engine_data_get(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;

   o = (Evas_Object_Textgrid *)(obj->object_data);
   if (!o) return NULL;
   return o->font;
}

static int
evas_object_textgrid_is_opaque(Evas_Object *obj __UNUSED__)
{
   /* this returns 1 if the internal object data implies that the object is
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

static int
evas_object_textgrid_was_opaque(Evas_Object *obj __UNUSED__)
{
   /* this returns 1 if the internal object data implies that the object was
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

static void
evas_object_textgrid_scale_update(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;
   int font_size;
   const char *font_name;

   o = (Evas_Object_Textgrid *)(obj->object_data);
   font_name = eina_stringshare_add(o->cur.font_name);
   font_size = o->cur.font_size;
   if (o->cur.font_name) eina_stringshare_del(o->cur.font_name);
   o->cur.font_name = NULL;
   o->prev.font_name = NULL;
   o->cur.font_size = 0;
   o->prev.font_size = 0;
   evas_object_textgrid_font_set(obj, font_name, font_size);
}

/*********************  LOCAL *********************/

/*********************  API *********************/

EAPI Evas_Object *
evas_object_textgrid_add(Evas *e)
{
   Evas_Object *obj;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   obj = evas_object_new(e);
   evas_object_textgrid_init(obj);
   evas_object_inject(obj, e);
   return obj;
}

EAPI void
evas_object_textgrid_size_set(Evas_Object *obj, int nbr_lines, int nbr_columns)
{
   Evas_Object_Textgrid *o;
   Evas_Textgrid_Cell *cells;

   if ((nbr_lines <= 0) || (nbr_columns <= 0))
     return;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

   cells = (Evas_Textgrid_Cell *)calloc(nbr_lines * nbr_columns, sizeof(Evas_Textgrid_Cell));
   if (!cells) return;

   /* FIXME : free o->cur.cells ? */
   o->cur.cells = cells;
   o->cur.nbr_lines = nbr_lines;
   o->cur.nbr_columns = nbr_columns;
   o->changed_size = 1;
   o->changed = 1;
}

EAPI void
evas_object_textgrid_size_get(Evas_Object *obj, int *nbr_lines, int *nbr_columns)
{
   Evas_Object_Textgrid *o;

   if (nbr_lines) *nbr_lines = 0;
   if (nbr_columns) *nbr_columns = 0;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

   if (nbr_lines) *nbr_lines = o->cur.nbr_lines;
   if (nbr_columns) *nbr_columns = o->cur.nbr_columns;
}

EAPI void
evas_object_textgrid_font_source_set(Evas_Object *obj, const char *font_source)
{
   Evas_Object_Textgrid *o;

   if ((!font_source) || (!*font_source))
     return;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

   if ((o->cur.font_source) &&
       (font_source) &&
       (!strcmp(o->cur.font_source, font_source)))
     return;

   eina_stringshare_replace(&o->cur.font_source, font_source);
   o->changed = 1;
   evas_object_change(obj);
}

EAPI const char *
evas_object_textgrid_font_source_get(const Evas_Object *obj)
{
   Evas_Object_Textgrid *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return NULL;
   MAGIC_CHECK_END();

   return o->cur.font_source;
}

EAPI void
evas_object_textgrid_font_set(Evas_Object *obj, const char *font_name, Evas_Font_Size font_size)
{
   Evas_Object_Textgrid *o;
   int is, was = 0, pass = 0, freeze = 0;
   Evas_Font_Description *font_description;

   if ((!font_name) || (!*font_name) ||(font_size <= 0))
     return;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

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

   if (obj->layer->evas->events_frozen <= 0)
     {
        pass = evas_event_passes_through(obj);
        freeze = evas_event_freezes_through(obj);
        if ((!pass) && (!freeze))
          was = evas_object_is_in_output_rect(obj,
                                              obj->layer->evas->pointer.x,
                                              obj->layer->evas->pointer.y, 1, 1);
     }

   /* DO IT */
   if (o->font)
     {
        evas_font_free(obj->layer->evas, o->font);
        o->font = NULL;
     }

   o->font = evas_font_load(obj->layer->evas, o->cur.font_description, o->cur.font_source,
                            (int)(((double) o->cur.font_size) * obj->cur.scale));
   if (o->font)
     {
        Eina_Unicode W[2] = { 'W', 0 };
        Evas_Font_Instance *script_fi = NULL;
        Evas_Font_Instance *cur_fi = NULL;
        Evas_Text_Props text_props;
        Evas_Script_Type script;
        int run_len, inset, adv;

        script = evas_common_language_script_type_get((const Eina_Unicode *)W, 1);
        run_len = TG_ENFN->font_run_end_get(TG_ENDT,
                                            o->font,
                                            &script_fi, &cur_fi,
                                            script, W, 1);
        memset(&text_props, 0, sizeof(Evas_Text_Props));
        evas_common_text_props_script_set(&text_props,
                                          script);

        TG_ENFN->font_text_props_info_create(TG_ENDT,
                                             script_fi,
                                             W,
                                             &text_props,
                                             NULL,
                                             0, 1,
                                             EVAS_TEXT_PROPS_MODE_NONE);

        TG_ENFN->font_string_size_get(TG_ENDT,
                                      o->font,
                                      &text_props,
                                      &o->cur.char_width,
                                      &o->cur.char_height);
        adv = TG_ENFN->font_h_advance_get(TG_ENDT, o->font, &text_props);
        inset = TG_ENFN->font_inset_get(TG_ENDT, o->font, &text_props);
        if ((inset + adv) > o->cur.char_width)
          o->cur.char_width = inset + adv;
        o->max_ascent = TG_ENFN->font_max_ascent_get(TG_ENDT, o->font);
     }
   else
     {
        obj->cur.geometry.w = 0;
        obj->cur.geometry.h = 0;
        o->max_ascent = 0;
     }

   o->changed = 1;
   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   evas_object_coords_recalc(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
        if ((!pass) && (!freeze))
          {
             is = evas_object_is_in_output_rect(obj,
                                                obj->layer->evas->pointer.x,
                                                obj->layer->evas->pointer.y,
                                                1, 1);
             if ((is ^ was) && obj->cur.visible)
               evas_event_feed_mouse_move(obj->layer->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
   evas_object_inform_call_resize(obj);
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_textgrid_font_get(const Evas_Object *obj, const char **font_name, Evas_Font_Size *font_size)
{
   Evas_Object_Textgrid *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (font_name) *font_name = "";
   if (font_size) *font_size = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   if (font_name) *font_name = "";
   if (font_size) *font_size = 0;
   return;
   MAGIC_CHECK_END();

   if (font_name) *font_name = o->cur.font_name;
   if (font_size) *font_size = o->cur.font_size;
}

EAPI void
evas_object_textgrid_cell_size_get(Evas_Object *obj, int *width, int *height)
{
   Evas_Object_Textgrid *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (width) *width = 0;
   if (height) *height = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   if (width) *width = 0;
   if (height) *height = 0;
   return;
   MAGIC_CHECK_END();

   if (width) *width = o->cur.char_width;
   if (height) *height = o->cur.char_height;
}

EAPI void
evas_object_textgrid_palette_set(Evas_Object *obj, Evas_Textgrid_Palette pal, int idx, int r, int g, int b, int a)
{
   Evas_Object_Textgrid *o;
   Eina_Array *palette;
   Evas_Object_Textgrid_Color *color;
   unsigned int count;

   if ((idx < 0) || (idx > 255))
     return;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

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
       palette = o->cur.palette_standard;
       break;
     case EVAS_TEXTGRID_PALETTE_EXTENDED:
       palette = o->cur.palette_extended;
       break;
     default:
       return;
     }

   color = (Evas_Object_Textgrid_Color *)malloc(sizeof(Evas_Object_Textgrid_Color));
   if (!color)
     return;

   color->a = a;
   color->r = r;
   color->g = g;
   color->b = b;

   count = eina_array_count(palette);
   if (idx < count)
     eina_array_data_set(palette, idx, color);
   else if (idx == count)
     eina_array_push(palette, color);
   else
     {
        Evas_Object_Textgrid_Color *c;
        unsigned int i;
        for (i = count; i < idx; i++)
          {
             c = (Evas_Object_Textgrid_Color *)malloc(sizeof(Evas_Object_Textgrid_Color));
             if (!c)
               {
                  ERR("Evas can not allocate memory");
                  return;
               }

             color->a = 0;
             color->r = 0;
             color->g = 0;
             color->b = 0;
             eina_array_push(palette, c);
          }
        eina_array_push(palette, color);
     }
   o->changed = 1;
   evas_object_change(obj);
}

EAPI void
evas_object_textgrid_palette_get(Evas_Object *obj, Evas_Textgrid_Palette pal, int idx, int *r, int *g, int *b, int *a)
{
   Evas_Object_Textgrid *o;
   Eina_Array *palette;
   Evas_Object_Textgrid_Color *color;

   if (idx < 0)
     return;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   if (a) *a = 0;
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   if (a) *a = 0;
   if (r) *r = 0;
   if (g) *g = 0;
   if (b) *b = 0;
   return;
   MAGIC_CHECK_END();

   switch (pal)
     {
     case EVAS_TEXTGRID_PALETTE_STANDARD:
       palette = o->cur.palette_standard;
       break;
     case EVAS_TEXTGRID_PALETTE_EXTENDED:
       palette = o->cur.palette_extended;
       break;
     default:
       return;
     }

   if (idx >= eina_array_count(palette))
     return;

   color = eina_array_data_get(palette, idx);
   if (!color)
     return;

   if (a) *a = color->a;
   if (r) *r = color->r;
   if (g) *g = color->g;
   if (b) *b = color->b;
}


EAPI void
evas_object_textgrid_supported_font_styles_set(Evas_Object *obj, Evas_Textgrid_Font_Style styles)
{
   Evas_Object_Textgrid *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

   /* FIXME: to do */
   o->changed = 1;
   evas_object_change(obj);
}

EAPI Evas_Textgrid_Font_Style
evas_object_textgrid_supported_font_styles_get(Evas_Object *obj)
{
   Evas_Object_Textgrid *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return EVAS_TEXTGRID_FONT_STYLE_NORMAL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return EVAS_TEXTGRID_FONT_STYLE_NORMAL;
   MAGIC_CHECK_END();

   /* FIXME: to do */
}

EAPI void
evas_object_textgrid_cellrow_set(Evas_Object *obj, int y, const Evas_Textgrid_Cell *row)
{
   Evas_Object_Textgrid *o;

   if (!row)
     return;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return;
   MAGIC_CHECK_END();

   if ((y < 0) || (y >= o->cur.nbr_lines))
     return;

//   memcpy(o->cur.cells + y * o->cur.nbr_columns, row, o->cur.nbr_columns * sizeof(Evas_Textgrid_Cell));
   o->changed = 1;
   evas_object_change(obj);
}

EAPI Evas_Textgrid_Cell *
evas_object_textgrid_cellrow_get(Evas_Object *obj, int y)
{
   Evas_Object_Textgrid *o;

   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   o = (Evas_Object_Textgrid *)(obj->object_data);
   MAGIC_CHECK(o, Evas_Object_Textgrid, MAGIC_OBJ_TEXTGRID);
   return NULL;
   MAGIC_CHECK_END();

   if ((y < 0) || (y >= o->cur.nbr_lines))
     return NULL;

   return o->cur.cells + y * o->cur.nbr_columns;
}
