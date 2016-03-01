#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>

#include "evas_evas_obj.h"
#include "evas_elw_button.h"
#include "evas_elw_box.h"
#include "evas_elw_boxedbutton.h"
#include "evas_elw_win.h"

Eina_Bool
_btn_clicked_cb(void *data, const Eo_Event *event)
{
   const Eo_Class *klass = eo_class_get(event->obj);
   printf("%s obj-type:'%s' data:'%s'\n", desc->name, eo_class_name_get(klass), (const char *) data);

   return EO_CALLBACK_CONTINUE;
}

int
main(int argc, char *argv[])
{
   Evas_Coord winw, winh;
     {
        winw = 400;
        winh = 400;
     }

   elm_init(argc, argv);
   eo_init();

   Eo *win = eo_add(ELW_WIN_CLASS, NULL);
   exevas_obj_size_set(win, winw, winh);
   exevas_obj_visibility_set(win, EINA_TRUE);

   Eo *bt = eo_add(ELW_BUTTON_CLASS, win);
   exevas_obj_position_set(bt, 25, 25);
   exevas_obj_size_set(bt, 50, 50);
   exevas_obj_color_set(bt, 255, 0, 0, 255);
   elw_button_text_set(bt, "Click");
   exevas_obj_visibility_set(bt, EINA_TRUE);
   eo_event_callback_add(bt, EV_CLICKED, _btn_clicked_cb, "btn");

   int r, g, b, a;
   exevas_obj_color_get(bt, &r, &g, &b, &a);
   printf("RGBa(%d, %d, %d, %d)\n", r, g, b, a);

   Eo *bx = eo_add(ELW_BOXEDBUTTON_CLASS, win);
   exevas_obj_position_set(bx, 100, 100);
   exevas_obj_size_set(bx, 70, 70);
   exevas_obj_color_set(bx, 0, 0, 255, 255);
   elw_button_text_set(bx, "Click2");
   exevas_obj_visibility_set(bx, EINA_TRUE);
   eo_event_callback_add(bx, EV_CLICKED, _btn_clicked_cb, "bxedbtn");

   elm_run();

   eo_unref(bx);
   eo_unref(bt);
   eo_unref(win);
   eo_shutdown();
   elm_shutdown();
   return 0;
}

