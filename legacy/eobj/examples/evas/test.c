#include <Elementary.h>

#include "evas_obj.h"
#include "elw_button.h"
#include "elw_box.h"
#include "elw_boxedbutton.h"
#include "elw_win.h"

Eina_Bool
_btn_clicked_cb(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
{
   (void) obj;
   (void) event_info;
   const Eobj_Class *klass = eobj_class_get(obj);
   printf("%s obj-type:'%s' data:'%s'\n", desc->name, eobj_class_name_get(klass), (const char *) data);

   return EINA_TRUE;
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
   eobj_init();

   Eobj *win = eobj_add(ELW_WIN_CLASS, NULL);
   eobj_do(win, EVAS_OBJ_SIZE_SET(winw, winh), EVAS_OBJ_VISIBILITY_SET(EINA_TRUE));

   Eobj *bt = eobj_add(ELW_BUTTON_CLASS, win);
   eobj_do(bt, EVAS_OBJ_POSITION_SET(25, 25),
         EVAS_OBJ_SIZE_SET(50, 50),
         EVAS_OBJ_COLOR_SET(255, 0, 0, 255),
         ELW_BUTTON_TEXT_SET("Click"),
         EVAS_OBJ_VISIBILITY_SET(EINA_TRUE));
   eobj_event_callback_add(bt, SIG_CLICKED, _btn_clicked_cb, "btn");

   int r, g, b, a;
   eobj_do(bt, EVAS_OBJ_COLOR_GET(&r, &g, &b, &a));
   printf("RGBa(%d, %d, %d, %d)\n", r, g, b, a);

   Eobj *bx = eobj_add(ELW_BOXEDBUTTON_CLASS, win);
   eobj_do(bx, EVAS_OBJ_POSITION_SET(100, 100),
         EVAS_OBJ_SIZE_SET(70, 70),
         EVAS_OBJ_COLOR_SET(0, 0, 255, 255),
         ELW_BUTTON_TEXT_SET("Click2"),
         EVAS_OBJ_VISIBILITY_SET(EINA_TRUE));
   eobj_event_callback_add(bx, SIG_CLICKED, _btn_clicked_cb, "bxedbtn");

   elm_run();

   eobj_unref(bx);
   eobj_unref(bt);
   eobj_unref(win);
   eobj_shutdown();
   elm_shutdown();
   return 0;
}

