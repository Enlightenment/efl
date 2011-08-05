#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
fac_unrealize(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   // setting factory content to null deletes it
   printf("DELETE for factory %p\n", obj);
   elm_factory_content_set(obj, NULL);
}

static void
fac_realize_end(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *tb, *bt;
   int i;
   
   printf("ADD for factory 4 %p\n", obj);
   tb = elm_table_add(win);

   for (i = 0; i < 40; i++)
     {
        char buf[32];
        
        snprintf(buf, sizeof(buf), "%i", i);
        
        bt = elm_button_add(win);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_object_text_set(bt, buf);
        elm_table_pack(tb, bt, 0, i, 1, 1);
        evas_object_show(bt);
     }
   
   elm_factory_content_set(obj, tb);
   evas_object_show(tb);
}

static void
fac_realize3(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *tb, *fc;
   int i;
   
   printf("ADD for factory 3 %p\n", obj);
   tb = elm_table_add(win);

   for (i = 0; i < 40; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize_end, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_table_pack(tb, fc, 0, i, 1, 1);
        evas_object_show(fc);
     }

   elm_factory_content_set(obj, tb);
   evas_object_show(tb);
}

static void
fac_realize2(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *tb, *fc;
   int i;
   
   printf("ADD for factory 2 %p\n", obj);
   tb = elm_table_add(win);

   for (i = 0; i < 40; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize3, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_table_pack(tb, fc, 0, i, 1, 1);
        evas_object_show(fc);
     }

   elm_factory_content_set(obj, tb);
   evas_object_show(tb);
}

static void
fac_realize1(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *tb, *fc;
   int i;
   
   printf("ADD for factory 1 %p\n", obj);
   tb = elm_table_add(win);

   for (i = 0; i < 40; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize2, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_table_pack(tb, fc, 0, i, 1, 1);
        evas_object_show(fc);
     }

   elm_factory_content_set(obj, tb);
   evas_object_show(tb);
}

void
test_factory(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *tb, *sc, *fc;
   int i;

   win = elm_win_add(NULL, "factory", ELM_WIN_BASIC);
   elm_win_title_set(win, "Factory");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   for (i = 0; i < 40; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize1, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_table_pack(tb, fc, 0, i, 1, 1);
        evas_object_show(fc);
     }

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   elm_scroller_content_set(sc, tb);
   evas_object_show(tb);

   evas_object_show(sc);
   
   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
