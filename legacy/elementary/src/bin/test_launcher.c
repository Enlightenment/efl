#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
test_launcher(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *sc, *tb, *pad;
   int i, j, k, n, m;
   char buf[PATH_MAX];
   
   win = elm_win_add(NULL, "launcher", ELM_WIN_BASIC);
   elm_win_title_set(win, "Launcher");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_04.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(bg, 0.0, 0.0);
   evas_object_size_hint_align_set(bg, 0.5, 0.5);
   n = 0; m = 0;
   for (k = 0 ; k < 5; k++)
     {
        pad = evas_object_rectangle_add(evas_object_evas_get(win));
        evas_object_size_hint_min_set(pad, 470, 4);
        evas_object_size_hint_weight_set(pad, 0.0, 0.0);
        evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_table_pack(tb, pad, (k * 7) + 1, 0, 5, 1);
        
        pad = evas_object_rectangle_add(evas_object_evas_get(win));
        evas_object_size_hint_min_set(pad, 470, 4);
        evas_object_size_hint_weight_set(pad, 0.0, 0.0);
        evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_table_pack(tb, pad, (k * 7) + 1, 14, 5, 1);
        
        pad = evas_object_rectangle_add(evas_object_evas_get(win));
        evas_object_size_hint_min_set(pad, 4, 4);
        evas_object_size_hint_weight_set(pad, 0.0, 0.0);
        evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_table_pack(tb, pad, k * 7, 1, 1, 12);
        
        pad = evas_object_rectangle_add(evas_object_evas_get(win));
        evas_object_size_hint_min_set(pad, 4, 4);
        evas_object_size_hint_weight_set(pad, 0.0, 0.0);
        evas_object_size_hint_align_set(pad, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_table_pack(tb, pad, (k * 7) + 6, 1, 1, 12);
        
        for (j = 0; j < 6; j++)
          {
             for (i = 0; i < 5; i++)
               {
                  Evas_Object *ic, *lb;
                  const char *names[] =
                    {
                         "Hello",    "World",    "Things",  "Should", 
                         "Work",     "Great",    "These",    "Days", 
                         "Lots",     "Of",       "Labels",   "Make",
                         "Everyone", "Happy",    "And",      "Calm"
                    };
                  
                  ic = elm_icon_add(win);
                  snprintf(buf, sizeof(buf), "%s/images/icon_%02i.png", PACKAGE_DATA_DIR, n);
                  elm_icon_file_set(ic, buf, NULL);
                  elm_icon_scale_set(ic, 0, 0);
                  evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(ic, 0.5, 0.5);
                  elm_table_pack(tb, ic, (k * 7) + 1 + i, 1 + (j * 2), 1, 1);
                  evas_object_show(ic);
                  
                  lb = elm_label_add(win);
                  elm_object_style_set(lb, "marker");
                  elm_label_label_set(lb, names[m]);
                  evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set(lb, 0.5, 0.5);
                  elm_table_pack(tb, lb, (k * 7) + 1 + i, 1 + (j * 2) + 1, 1, 1);
                  evas_object_show(lb);
                  
                  n++; if (n > 23) n = 0;
                  m++; if (m > 15) m = 0;
               }
          }
     }
   
   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, 1, 0);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);
   
   elm_scroller_content_set(sc, tb);
   evas_object_show(tb);
   
   elm_scroller_page_relative_set(sc, 1.0, 1.0);
   // elm_scroller_page_size_set(sc, 200, 200);
   evas_object_show(sc);

   evas_object_resize(win, 480, 800);
   evas_object_show(win);
}
#endif
