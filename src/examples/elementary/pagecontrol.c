//Compile with:
//edje_cc pagecontrol.edc
//gcc -g -o pagecontrol pagecontrol.c `pkg-config --cflags --libs elementary`


#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Elementary.h>

#define WIN_WIDTH 300
#define WIN_HEIGHT 400


EAPI_MAIN int
elm_main(int argc,
         char **argv)
{
   Eo *win, *layout, *obj, *ind, *rect;
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

   // Pagescroller
   //obj = efl_add(EFL_UI_PAGECONTROL_CLASS, win);
   obj = efl_add(EFL_UI_PAGESCROLLER_CLASS, win);
   efl_content_set(efl_part(layout, "pagecontrol"), obj);

   // Pagescroller Transition Effect
   tran = efl_add(EFL_PAGE_TRANSITION_SCROLL_CLASS, NULL);
   //tran = efl_add(EFL_PAGE_TRANSITION_CUBE_CLASS, NULL);
   //tran = efl_add(EFL_PAGE_TRANSITION_ROTATE_CLASS, NULL);
   efl_ui_pagescroller_transition_set(obj, tran);

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

   // Page Indicator
   ind = efl_add(EFL_UI_PAGEINDICATOR_CLASS, layout);
   efl_content_set(efl_part(layout, "pageindicator"), ind);
   efl_ui_pageindicator_bind(ind, obj);

   elm_run();

   return 0;
}
ELM_MAIN()
