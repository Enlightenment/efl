#ifdef HAVE_CONFIG
# include "config.h"
#endif

#include <Eo.h>
#include <Evas.h>
#include <Elementary.h>
#include <Elm_Code.h>
#include "elm_code_widget2.eo.h"

typedef struct
{
   Elm_Code *code;

   Evas_Font_Size font_size;
   unsigned int cursor_line, cursor_col;
   Eina_Bool editable;

} Elm_Code_Widget2_Data;

EOLIAN static void
_elm_code_widget2_eo_base_constructor(Eo *obj, Elm_Code_Widget2_Data *pd)
{
   eo_do_super(obj, ELM_CODE_WIDGET2_CLASS, eo_constructor());
printf("constr\n");
}

EOLIAN static void
_elm_code_widget2_evas_object_smart_resize(Eo *obj, Elm_Code_Widget2_Data *pd, Evas_Coord w, Evas_Coord h)
{
printf("size %d, %d\n", w, h);
}

EOLIAN static void
_elm_code_widget2_class_constructor(Eo_Class *klass)
{

}

EOLIAN static void
_elm_code_widget2_elm_interface_scrollable_content_pos_set(Eo *obj, Elm_Code_Widget2_Data *pd, Evas_Coord x, Evas_Coord y, Eina_Bool sig)
{

}

EOLIAN static void
_elm_code_widget2_evas_object_smart_add(Eo *obj, Elm_Code_Widget2_Data *pd)
{
   Evas_Object *text;

printf("add\n");
   eo_do_super(obj, ELM_CODE_WIDGET2_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);
//   elm_widget_can_focus_set(obj, EINA_TRUE);

   text = elm_label_add(obj);
   elm_object_text_set(text, "HELLO");
   elm_widget_sub_object_add(obj, text);

   evas_object_size_hint_weight_set(text, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(text, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(text);

   eo_do(obj, elm_obj_widget_theme_apply());

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_code_widget2_font_size_set(Eo *obj, Elm_Code_Widget2_Data *pd, Evas_Font_Size font_size)
{
   printf("set\n");
   pd->font_size = font_size;
}

EOLIAN static Evas_Font_Size
_elm_code_widget2_font_size_get(Eo *obj, Elm_Code_Widget2_Data *pd)
{
   return pd->font_size;
}

#include "elm_code_widget2.eo.c"
