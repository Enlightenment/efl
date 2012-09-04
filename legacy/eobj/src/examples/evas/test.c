#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>

#include "evas_obj.h"
#include "elw_button.h"
#include "elw_box.h"
#include "elw_boxedbutton.h"
#include "elw_win.h"

Eina_Bool
_btn_clicked_cb(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   (void) obj;
   (void) event_info;
   const Eo_Class *klass = eo_class_get(obj);
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
   eo_do(win, exevas_obj_size_set(winw, winh), exevas_obj_visibility_set(EINA_TRUE));

   Eo *bt = eo_add(ELW_BUTTON_CLASS, win);
   eo_do(bt, exevas_obj_position_set(25, 25),
         exevas_obj_size_set(50, 50),
         exevas_obj_color_set(255, 0, 0, 255),
         elw_button_text_set("Click"),
         exevas_obj_visibility_set(EINA_TRUE));
   eo_do(bt, eo_event_callback_add(EV_CLICKED, _btn_clicked_cb, "btn"));

   int r, g, b, a;
   eo_do(bt, exevas_obj_color_get(&r, &g, &b, &a));
   printf("RGBa(%d, %d, %d, %d)\n", r, g, b, a);

   Eo *bx = eo_add(ELW_BOXEDBUTTON_CLASS, win);
   eo_do(bx, exevas_obj_position_set(100, 100),
         exevas_obj_size_set(70, 70),
         exevas_obj_color_set(0, 0, 255, 255),
         elw_button_text_set("Click2"),
         exevas_obj_visibility_set(EINA_TRUE));
   eo_do(bx, eo_event_callback_add(EV_CLICKED, _btn_clicked_cb, "bxedbtn"));

   elm_run();

   eo_unref(bx);
   eo_unref(bt);
   eo_unref(win);
   eo_shutdown();
   elm_shutdown();
   return 0;
}

