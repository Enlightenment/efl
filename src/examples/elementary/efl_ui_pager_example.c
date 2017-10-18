//Compile with:
//edje_cc pagecontrol.edc
//gcc -g -o efl_ui_pager_example efl_ui_pager_example.c `pkg-config --cflags --libs elementary`


#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>

#define WIN_WIDTH 300
#define WIN_HEIGHT 400



static void
_clicked(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   efl_ui_pagescroller_current_page_set(obj, 2, EINA_TRUE, EINA_TRUE);
}

EAPI_MAIN int
elm_main(int argc,
         char **argv)
{
   Eo *win, *layout, *obj, *ind, *rect, *btn;
   Efl_Page_Transition *tran;

   // Window
   win = efl_add(efl_ui_win_class_get(), NULL, "test", ELM_WIN_BASIC);
   efl_gfx_size_set(win, WIN_WIDTH, WIN_HEIGHT);
   efl_gfx_visible_set(win, EINA_TRUE);
  
   // Layout
   layout = efl_add(ELM_LAYOUT_CLASS, win);
   efl_gfx_size_hint_weight_set(layout, 1, 1);
   efl_file_set(layout, "pagecontrol.edj", "pagecontrol_example");
   efl_content_set(win, layout);

   // Pager
   obj = efl_add(EFL_UI_PAGER_CLASS, win);
   efl_content_set(efl_part(layout, "pagecontrol"), obj);

   tran = efl_add(EFL_PAGE_TRANSITION_SCROLL_CLASS, NULL);
   efl_ui_pager_transition_set(obj, tran);

   efl_ui_pager_page_size_set(obj, 200, 300);
   efl_ui_pager_padding_set(obj, 20);

#if 0
   efl_ui_pager_page_size_set(obj,
                              EFL_UI_PAGER_SIZE_FILL,
                              EFL_UI_PAGER_SIZE_FILL); // follow object size
   efl_ui_pager_page_align_set(obj,
                               EFL_UI_PAGER_ALIGN_CENTER,
                               EFL_UI_PAGER_ALIGN_CENTER);
#endif

   // Pages
   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                  efl_provider_find(obj, EVAS_CANVAS_CLASS));
   efl_gfx_visible_set(rect, EINA_TRUE);
   efl_gfx_color_set(rect, 255, 0, 0, 255);
   efl_pack_end(obj, rect);

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                  efl_provider_find(obj, EVAS_CANVAS_CLASS));
   efl_gfx_visible_set(rect, EINA_TRUE);
   efl_gfx_color_set(rect, 0, 255, 0, 255);
   efl_pack_end(obj, rect);

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                  efl_provider_find(obj, EVAS_CANVAS_CLASS));
   efl_gfx_visible_set(rect, EINA_TRUE);
   efl_gfx_color_set(rect, 0, 0, 255, 255);
   efl_pack_end(obj, rect);

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                  efl_provider_find(obj, EVAS_CANVAS_CLASS));
   efl_gfx_visible_set(rect, EINA_TRUE);
   efl_gfx_color_set(rect, 255, 255, 0, 255);
   efl_pack_end(obj, rect);

   rect = efl_add(EFL_CANVAS_RECTANGLE_CLASS,
                  efl_provider_find(obj, EVAS_CANVAS_CLASS));
   efl_gfx_visible_set(rect, EINA_TRUE);
   efl_gfx_color_set(rect, 0, 255, 255, 255);
   efl_pack_end(obj, rect);

   elm_run();

   return 0;
}
ELM_MAIN()
