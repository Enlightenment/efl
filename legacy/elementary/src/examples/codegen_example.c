/**
 * Simple Elementary example illustrating how to use elementary_codegen.
 *
 * elementary_codegen is a tool that generates code to acessing the
 * parts and programs with the keyword "api" of a specified
 * group. This tool make easier working with edje avoiding common
 * misspelling errors when acessing the parts and/or programs.
 *
 * To use the elementary_codegen:
 * elementary_codegen --prefix <myapp_myobj> <input.edj> <a_group> <source.c> <header.h>
 *
 * In the case of this example:
 * elementary_codegen --prefix=codegen_example codegen_example.edj \
 * elm/example/mylayout/default codegen_example_generated.c codegen_example_generated.h
 *
 * @verbatim
 * edje_cc codegen_example.edc && elementary_codegen --prefix=codegen_example \
 * codegen_example.edj elm/example/mylayout/default codegen_example_generated.c \
 * codegen_example_generated.h
 * gcc -c codegen_example_generated.c `pkg-config --libs --cflags ecore-evas edje elementary`
 * gcc -o codegen_example codegen_example_generated.o \
 * codegen_example.c `pkg-config --libs --cflags ecore-evas edje elementary`
 * @endverbatim
 */

#include "codegen_example_generated.h"

static Eina_Bool _btn_large = EINA_FALSE;

static void
_swallow_btn_cb(void *data, Evas_Object *btn, void *event_info)
{
   Evas_Object *layout = data;

   if (_btn_large == EINA_FALSE)
     {
        _btn_large = EINA_TRUE;
        codegen_example_swallow_grow_emit(layout);
        elm_object_text_set(btn, "Reduce me!");
        if (!codegen_example_table_clear(layout, EINA_TRUE))
          fprintf(stderr, "Could not remove the items from the table!\n");
     }
   else
     {
        _btn_large = EINA_FALSE;
        codegen_example_swallow_shrink_emit(layout);
        elm_object_text_set(btn, "Enlarge me!");
     }
}

static void
_size_changed_cb(void *data, Evas_Object *layout, const char *emission, const char *source)
{
   Evas_Object *edje;
   Evas_Coord w, h;

   elm_layout_sizing_eval(layout);
   edje = elm_layout_edje_get(layout);
   edje_object_size_min_calc(edje, &w, &h);
   printf("Minimum size for this theme: %dx%d\n", w, h);
}

static Evas_Object *
_button_create(Evas_Object *parent, const char *label)
{
   Evas_Object *btn;
   btn = elm_button_add(parent);
   if (!btn) return NULL;

   elm_object_text_set(btn, label);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);

   return btn;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *btn, *layout, *tbl_items[6];
   const char *labels[] = {"One", "Two", "Three", "Four", "Five", "Six"};
   int i;

   elm_app_info_set(elm_main, "elementary", "examples/codegen_example.edj");

   win = elm_win_util_standard_add("codegen", "Elementary CodeGen");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   // Adding layout
   layout = codegen_example_layout_add(win, NULL, NULL);
   if (!layout)
     {
        printf("Could not create the layout\n");
        return -1;
     }

   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   evas_object_show(layout);

   codegen_example_size_changed_callback_add(layout, _size_changed_cb, layout);

   // Setting title
   const char *title = codegen_example_title_get(layout);
   if (title)
     {
        elm_win_title_set(win, title);
        codegen_example_title_set(layout, title);
     }

   btn = _button_create(win, "Enlarge me!");
   codegen_example_custom_set(layout, btn);
   evas_object_smart_callback_add(btn, "clicked", _swallow_btn_cb, layout);

   for (i = 0; i < 6; i++)
     {
        tbl_items[i] = _button_create(win, labels[i]);
        if (i < 3)
          {
             if (!codegen_example_table_pack(layout, tbl_items[i], i, i, 1,1))
               fprintf(stderr, "Could not add the button to the table!\n");
          }
        else
          {
             if (!codegen_example_box_append(layout, tbl_items[i]))
               fprintf(stderr, "Could not add the button to the box!\n");
          }

        evas_object_show(tbl_items[i]);
     }

   evas_object_resize(win, 500, 600);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
