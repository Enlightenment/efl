
void _evas_text_shadow_color_set(Eo *obj, Evas_Text_Data *pd, int r, int g, int b, int a);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_text_shadow_color_set, EFL_FUNC_CALL(r, g, b, a), int r, int g, int b, int a);

void _evas_text_shadow_color_get(const Eo *obj, Evas_Text_Data *pd, int *r, int *g, int *b, int *a);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_text_shadow_color_get, EFL_FUNC_CALL(r, g, b, a), int *r, int *g, int *b, int *a);

void _evas_text_ellipsis_set(Eo *obj, Evas_Text_Data *pd, double ellipsis);


static Eina_Error
__eolian_evas_text_ellipsis_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   evas_obj_text_ellipsis_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_text_ellipsis_set, EFL_FUNC_CALL(ellipsis), double ellipsis);

double _evas_text_ellipsis_get(const Eo *obj, Evas_Text_Data *pd);


static Eina_Value
__eolian_evas_text_ellipsis_get_reflect(Eo *obj)
{
   double val = evas_obj_text_ellipsis_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_ellipsis_get, double, -1.000000 /* +1.000000 */);

void _evas_text_bidi_delimiters_set(Eo *obj, Evas_Text_Data *pd, const char *delim);


static Eina_Error
__eolian_evas_text_bidi_delimiters_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   evas_obj_text_bidi_delimiters_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_text_bidi_delimiters_set, EFL_FUNC_CALL(delim), const char *delim);

const char *_evas_text_bidi_delimiters_get(const Eo *obj, Evas_Text_Data *pd);


static Eina_Value
__eolian_evas_text_bidi_delimiters_get_reflect(Eo *obj)
{
   const char *val = evas_obj_text_bidi_delimiters_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_bidi_delimiters_get, const char *, NULL);

void _evas_text_outline_color_set(Eo *obj, Evas_Text_Data *pd, int r, int g, int b, int a);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_text_outline_color_set, EFL_FUNC_CALL(r, g, b, a), int r, int g, int b, int a);

void _evas_text_outline_color_get(const Eo *obj, Evas_Text_Data *pd, int *r, int *g, int *b, int *a);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_text_outline_color_get, EFL_FUNC_CALL(r, g, b, a), int *r, int *g, int *b, int *a);

void _evas_text_glow2_color_set(Eo *obj, Evas_Text_Data *pd, int r, int g, int b, int a);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_text_glow2_color_set, EFL_FUNC_CALL(r, g, b, a), int r, int g, int b, int a);

void _evas_text_glow2_color_get(const Eo *obj, Evas_Text_Data *pd, int *r, int *g, int *b, int *a);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_text_glow2_color_get, EFL_FUNC_CALL(r, g, b, a), int *r, int *g, int *b, int *a);

void _evas_text_style_set(Eo *obj, Evas_Text_Data *pd, Evas_Text_Style_Type style);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_text_style_set, EFL_FUNC_CALL(style), Evas_Text_Style_Type style);

Evas_Text_Style_Type _evas_text_style_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_style_get, Evas_Text_Style_Type, 0);

void _evas_text_glow_color_set(Eo *obj, Evas_Text_Data *pd, int r, int g, int b, int a);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_text_glow_color_set, EFL_FUNC_CALL(r, g, b, a), int r, int g, int b, int a);

void _evas_text_glow_color_get(const Eo *obj, Evas_Text_Data *pd, int *r, int *g, int *b, int *a);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_text_glow_color_get, EFL_FUNC_CALL(r, g, b, a), int *r, int *g, int *b, int *a);

int _evas_text_max_descent_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_max_descent_get, int, 0);

void _evas_text_style_pad_get(const Eo *obj, Evas_Text_Data *pd, int *l, int *r, int *t, int *b);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_text_style_pad_get, EFL_FUNC_CALL(l, r, t, b), int *l, int *r, int *t, int *b);

Efl_Text_Bidirectional_Type _evas_text_direction_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_direction_get, Efl_Text_Bidirectional_Type, 0);

int _evas_text_ascent_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_ascent_get, int, 0);

int _evas_text_horiz_advance_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_horiz_advance_get, int, 0);

int _evas_text_inset_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_inset_get, int, 0);

int _evas_text_max_ascent_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_max_ascent_get, int, 0);

int _evas_text_vert_advance_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_vert_advance_get, int, 0);

int _evas_text_descent_get(const Eo *obj, Evas_Text_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_text_descent_get, int, 0);

int _evas_text_last_up_to_pos(const Eo *obj, Evas_Text_Data *pd, int x, int y);

EOAPI EFL_FUNC_BODYV_CONST(evas_obj_text_last_up_to_pos, int, -1 /* +1 */, EFL_FUNC_CALL(x, y), int x, int y);

int _evas_text_char_coords_get(const Eo *obj, Evas_Text_Data *pd, int x, int y, int *cx, int *cy, int *cw, int *ch);

EOAPI EFL_FUNC_BODYV_CONST(evas_obj_text_char_coords_get, int, 0, EFL_FUNC_CALL(x, y, cx, cy, cw, ch), int x, int y, int *cx, int *cy, int *cw, int *ch);

Eina_Bool _evas_text_char_pos_get(const Eo *obj, Evas_Text_Data *pd, int pos, int *cx, int *cy, int *cw, int *ch);

EOAPI EFL_FUNC_BODYV_CONST(evas_obj_text_char_pos_get, Eina_Bool, 0, EFL_FUNC_CALL(pos, cx, cy, cw, ch), int pos, int *cx, int *cy, int *cw, int *ch);

Efl_Object *_evas_text_efl_object_constructor(Eo *obj, Evas_Text_Data *pd);


void _evas_text_efl_object_destructor(Eo *obj, Evas_Text_Data *pd);


void _evas_text_efl_gfx_entity_size_set(Eo *obj, Evas_Text_Data *pd, Eina_Size2D size);


void _evas_text_efl_gfx_entity_scale_set(Eo *obj, Evas_Text_Data *pd, double scale);


void _evas_text_efl_text_text_set(Eo *obj, Evas_Text_Data *pd, const char *text);


const char *_evas_text_efl_text_text_get(const Eo *obj, Evas_Text_Data *pd);


void _evas_text_efl_text_font_font_set(Eo *obj, Evas_Text_Data *pd, const char *font, Efl_Font_Size size);


void _evas_text_efl_text_font_font_get(const Eo *obj, Evas_Text_Data *pd, const char **font, Efl_Font_Size *size);


void _evas_text_efl_text_font_font_source_set(Eo *obj, Evas_Text_Data *pd, const char *font_source);


const char *_evas_text_efl_text_font_font_source_get(const Eo *obj, Evas_Text_Data *pd);


void _evas_text_efl_text_font_font_bitmap_scalable_set(Eo *obj, Evas_Text_Data *pd, Efl_Text_Font_Bitmap_Scalable scalable);


Efl_Text_Font_Bitmap_Scalable _evas_text_efl_text_font_font_bitmap_scalable_get(const Eo *obj, Evas_Text_Data *pd);


void _evas_text_efl_gfx_filter_filter_program_set(Eo *obj, Evas_Text_Data *pd, const char *code, const char *name);


void _evas_text_efl_canvas_filter_internal_filter_dirty(Eo *obj, Evas_Text_Data *pd);


Eina_Bool _evas_text_efl_canvas_filter_internal_filter_input_alpha(Eo *obj, Evas_Text_Data *pd);


Eina_Bool _evas_text_efl_canvas_filter_internal_filter_input_render(Eo *obj, Evas_Text_Data *pd, void *filter, void *engine, void *output, void *drawctx, void *data, int l, int r, int t, int b, int x, int y, Eina_Bool do_async);


void _evas_text_efl_canvas_filter_internal_filter_state_prepare(Eo *obj, Evas_Text_Data *pd, Efl_Canvas_Filter_State *state, void *data);


void _evas_text_efl_canvas_object_paragraph_direction_set(Eo *obj, Evas_Text_Data *pd, Efl_Text_Bidirectional_Type dir);


Efl_Text_Bidirectional_Type _evas_text_efl_canvas_object_paragraph_direction_get(const Eo *obj, Evas_Text_Data *pd);


static Eina_Bool
_evas_text_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EVAS_TEXT_EXTRA_OPS
#define EVAS_TEXT_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(evas_obj_text_shadow_color_set, _evas_text_shadow_color_set),
      EFL_OBJECT_OP_FUNC(evas_obj_text_shadow_color_get, _evas_text_shadow_color_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_ellipsis_set, _evas_text_ellipsis_set),
      EFL_OBJECT_OP_FUNC(evas_obj_text_ellipsis_get, _evas_text_ellipsis_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_bidi_delimiters_set, _evas_text_bidi_delimiters_set),
      EFL_OBJECT_OP_FUNC(evas_obj_text_bidi_delimiters_get, _evas_text_bidi_delimiters_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_outline_color_set, _evas_text_outline_color_set),
      EFL_OBJECT_OP_FUNC(evas_obj_text_outline_color_get, _evas_text_outline_color_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_glow2_color_set, _evas_text_glow2_color_set),
      EFL_OBJECT_OP_FUNC(evas_obj_text_glow2_color_get, _evas_text_glow2_color_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_style_set, _evas_text_style_set),
      EFL_OBJECT_OP_FUNC(evas_obj_text_style_get, _evas_text_style_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_glow_color_set, _evas_text_glow_color_set),
      EFL_OBJECT_OP_FUNC(evas_obj_text_glow_color_get, _evas_text_glow_color_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_max_descent_get, _evas_text_max_descent_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_style_pad_get, _evas_text_style_pad_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_direction_get, _evas_text_direction_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_ascent_get, _evas_text_ascent_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_horiz_advance_get, _evas_text_horiz_advance_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_inset_get, _evas_text_inset_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_max_ascent_get, _evas_text_max_ascent_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_vert_advance_get, _evas_text_vert_advance_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_descent_get, _evas_text_descent_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_last_up_to_pos, _evas_text_last_up_to_pos),
      EFL_OBJECT_OP_FUNC(evas_obj_text_char_coords_get, _evas_text_char_coords_get),
      EFL_OBJECT_OP_FUNC(evas_obj_text_char_pos_get, _evas_text_char_pos_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _evas_text_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _evas_text_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _evas_text_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_scale_set, _evas_text_efl_gfx_entity_scale_set),
      EFL_OBJECT_OP_FUNC(efl_text_set, _evas_text_efl_text_text_set),
      EFL_OBJECT_OP_FUNC(efl_text_get, _evas_text_efl_text_text_get),
      EFL_OBJECT_OP_FUNC(efl_text_font_set, _evas_text_efl_text_font_font_set),
      EFL_OBJECT_OP_FUNC(efl_text_font_get, _evas_text_efl_text_font_font_get),
      EFL_OBJECT_OP_FUNC(efl_text_font_source_set, _evas_text_efl_text_font_font_source_set),
      EFL_OBJECT_OP_FUNC(efl_text_font_source_get, _evas_text_efl_text_font_font_source_get),
      EFL_OBJECT_OP_FUNC(efl_text_font_bitmap_scalable_set, _evas_text_efl_text_font_font_bitmap_scalable_set),
      EFL_OBJECT_OP_FUNC(efl_text_font_bitmap_scalable_get, _evas_text_efl_text_font_font_bitmap_scalable_get),
      EFL_OBJECT_OP_FUNC(efl_gfx_filter_program_set, _evas_text_efl_gfx_filter_filter_program_set),
      EFL_OBJECT_OP_FUNC(evas_filter_dirty, _evas_text_efl_canvas_filter_internal_filter_dirty),
      EFL_OBJECT_OP_FUNC(evas_filter_input_alpha, _evas_text_efl_canvas_filter_internal_filter_input_alpha),
      EFL_OBJECT_OP_FUNC(evas_filter_input_render, _evas_text_efl_canvas_filter_internal_filter_input_render),
      EFL_OBJECT_OP_FUNC(evas_filter_state_prepare, _evas_text_efl_canvas_filter_internal_filter_state_prepare),
      EFL_OBJECT_OP_FUNC(efl_canvas_object_paragraph_direction_set, _evas_text_efl_canvas_object_paragraph_direction_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_object_paragraph_direction_get, _evas_text_efl_canvas_object_paragraph_direction_get),
      EVAS_TEXT_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"ellipsis", __eolian_evas_text_ellipsis_set_reflect, __eolian_evas_text_ellipsis_get_reflect},
      {"bidi_delimiters", __eolian_evas_text_bidi_delimiters_set_reflect, __eolian_evas_text_bidi_delimiters_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _evas_text_class_desc = {
   EO_VERSION,
   "Evas.Text",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Evas_Text_Data),
   _evas_text_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(evas_text_class_get, &_evas_text_class_desc, EFL_CANVAS_OBJECT_CLASS, EFL_TEXT_INTERFACE, EFL_TEXT_FONT_INTERFACE, EFL_CANVAS_FILTER_INTERNAL_MIXIN, NULL);

#include "evas_text_eo.legacy.c"
