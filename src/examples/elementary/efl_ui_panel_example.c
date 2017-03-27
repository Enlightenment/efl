//Compile with:
//gcc -g efl_ui_panel_example.c -o efl_ui_panel_example `pkg-config --cflags --libs elementary`

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>


static void
_win_del(void *data EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED,
         void *event_info EINA_UNUSED)
{
   elm_exit();
}

static void
_clicked_cb(void *data,
            Evas_Object *obj EINA_UNUSED,
            void *event_info)
{
   efl_ui_panel_toggle(data);
}

EAPI_MAIN int
elm_main(int argc,
         char **argv)
{
   Eo *win, *obj, *rect, *btn;

   win = efl_add(EFL_UI_WIN_STANDARD_CLASS, NULL, "test", "test");
   evas_object_smart_callback_add(win, "delete,request", _win_del, NULL);
   efl_gfx_size_set(win, 360, 640);
   efl_gfx_visible_set(win, EINA_TRUE);

   obj = efl_add(EFL_UI_PANEL_CLASS, win);
   efl_gfx_visible_set(obj, EINA_TRUE);
   efl_content_set(win, obj);
   efl_gfx_size_hint_weight_set(obj, EFL_GFX_SIZE_HINT_EXPAND,
                                EFL_GFX_SIZE_HINT_EXPAND);

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(rect, 0, 0, 255, 255);
   efl_gfx_visible_set(rect, EINA_TRUE);
   efl_content_set(obj, rect);
   efl_gfx_size_hint_align_set(rect, EFL_GFX_SIZE_HINT_FILL,
                               EFL_GFX_SIZE_HINT_FILL);

   btn = efl_add(ELM_BUTTON_CLASS, win);
   efl_gfx_visible_set(btn, EINA_TRUE);
   efl_gfx_size_set(btn, 100, 50);
   elm_object_text_set(btn, "toggle");
   evas_object_smart_callback_add(btn, "clicked", _clicked_cb, obj);

   elm_run();

   return 0;
}
ELM_MAIN()
