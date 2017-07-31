#include "elm_test_focus_common.h"

//   printf("[%s:%d] %p %p --- %p --- %p ----- JWH : %d\n", __func__, __LINE__,
//          btn[0], btn[1], obj, elm_object_focused_object_get(obj),
//          elm_object_focus_get(btn[0]));
//   fflush(stdout);

static void __focused_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object **btn = (Evas_Object **)data;
   int i;

   elm_object_focus_set(btn[0], EINA_TRUE);
   ck_assert_int_eq(elm_object_focus_get(btn[0]), 1);

   elm_object_focus_next(obj, ELM_FOCUS_RIGHT);
   ck_assert(elm_object_focused_object_get(obj) == btn[1]);

   elm_object_focus_next(obj, ELM_FOCUS_DOWN);
   ck_assert(elm_object_focused_object_get(obj) == btn[2]);

   elm_object_focus_next(obj, ELM_FOCUS_DOWN);
   ck_assert(elm_object_focused_object_get(obj) == btn[4]);

   elm_object_focus_next(obj, ELM_FOCUS_RIGHT);
   ck_assert(elm_object_focused_object_get(obj) == btn[3]);

   elm_object_focus_next(obj, ELM_FOCUS_UP);
   ck_assert(elm_object_focused_object_get(obj) == btn[1]);

   elm_object_focus_next(obj, ELM_FOCUS_LEFT);
   ck_assert(elm_object_focused_object_get(obj) == btn[1]);

   elm_exit();
}

START_TEST(focus_next_test1)
{
   Evas_Object *win, *btn[2];
   int i;

   elm_init(1, NULL);

   // creating window
   win = efl_add(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Focus Test1"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   evas_object_smart_callback_add(win, "focused", __focused_cb, btn);
   efl_gfx_size_set(win, 500, 500);
   efl_gfx_visible_set(win, 1);

   // creating buttons
   //////////////////////////////////////////
   //                                      //
   //       btn1               btn2        //
   //                                      //
   //                                      //
   //        btn3                          //
   //                                      //
   //                              btn4    //
   //                                      //
   //           btn5                       //
   //////////////////////////////////////////
     {
        char *btn_str[5] = {"Button1", "Button2", "Button3", "Button4", "Button5"};
        int btn_loc[5][2] = {{50, 50}, {300, 50}, {100, 250}, {350, 350}, {150, 450}};
        int btn_w = 100, btn_h = 50;

        for (i = 0; i < 5; i++)
          {
             btn[i] = efl_add(EFL_UI_BUTTON_CLASS, win,
                              efl_text_set(efl_added, btn_str[i]),
                              efl_gfx_geometry_set(efl_added,
                                                   btn_loc[i][0], btn_loc[i][1],
                                                   btn_w, btn_h),
                              efl_gfx_visible_set(efl_added, 1));
         }
     }

   elm_run();
   elm_shutdown();
}
END_TEST
void elm_test_focus_legacy(TCase *tc)
{
    tcase_add_test(tc, focus_next_test1);
}
