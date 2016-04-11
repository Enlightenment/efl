#include "codegen_example_generated.h"

Evas_Object *
codegen_example_layout_add(Evas_Object *o, Elm_Theme *th, const char *edje_file)
{
   Evas_Object *l;

   if (edje_file)
     elm_theme_extension_add(th, edje_file);
   else
     elm_theme_extension_add(th, "./codegen_example.edj");

   l = elm_layout_add(o);
   if (!l) return NULL;

   if (!elm_layout_theme_set(l, "example", "mylayout", "default"))
     {
        evas_object_del(l);
        return NULL;
     }

   return l;
}

void
codegen_example_title_set(Evas_Object *o, const char *value)
{
   elm_layout_text_set(o, "example/title", value);
}

const char *
codegen_example_title_get(const Evas_Object *o)
{
   return elm_layout_text_get(o, "example/title");
}

void
codegen_example_custom_set(Evas_Object *o, Evas_Object *value)
{
   elm_layout_content_set(o, "example/custom", value);
}

Evas_Object *
codegen_example_custom_unset(Evas_Object *o)
{
   return elm_layout_content_unset(o, "example/custom");
}

Evas_Object *
codegen_example_custom_get(const Evas_Object *o)
{
   return elm_layout_content_get(o, "example/custom");
}

Eina_Bool
codegen_example_box_append(Evas_Object *o, Evas_Object *child)
{
   return elm_layout_box_append(o, "example/box", child);
}

Eina_Bool
codegen_example_box_prepend(Evas_Object *o, Evas_Object *child)
{
   return elm_layout_box_prepend(o, "example/box", child);
}

Eina_Bool
codegen_example_box_insert_before(Evas_Object *o, Evas_Object *child, const Evas_Object *reference)
{
   return elm_layout_box_insert_before(o, "example/box", child, reference);
}

Eina_Bool
codegen_example_box_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos)
{
   return elm_layout_box_insert_at(o, "example/box", child, pos);
}

Evas_Object *
codegen_example_box_remove(Evas_Object *o, Evas_Object *child)
{
   return elm_layout_box_remove(o, "example/box", child);
}

Eina_Bool
codegen_example_box_remove_all(Evas_Object *o, Eina_Bool clear)
{
   return elm_layout_box_remove_all(o, "example/box", clear);
}

Eina_Bool
codegen_example_table_pack(Evas_Object *o, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   return elm_layout_table_pack(o, "example/table", child, col, row, colspan, rowspan);
}

Evas_Object *
codegen_example_table_unpack(Evas_Object *o, Evas_Object *child)
{
   return elm_layout_table_unpack(o, "example/table", child);
}

Eina_Bool
codegen_example_table_clear(Evas_Object *o, Eina_Bool clear)
{
   return elm_layout_table_clear(o, "example/table", clear);
}

void
codegen_example_swallow_grow_emit(Evas_Object *o)
{
   elm_layout_signal_emit(o, "button,enlarge", "");
}

void
codegen_example_swallow_shrink_emit(Evas_Object *o)
{
   elm_layout_signal_emit(o, "button,reduce", "");
}

void
codegen_example_size_changed_callback_add(Evas_Object *o, Edje_Signal_Cb func, void *data)
{
   elm_layout_signal_callback_add(o, "size,changed", "", func, data);
}

void
codegen_example_size_changed_callback_del(Evas_Object *o, Edje_Signal_Cb func)
{
   elm_layout_signal_callback_del(o, "size,changed", "", func);
}

