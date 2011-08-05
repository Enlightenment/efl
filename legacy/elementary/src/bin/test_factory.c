#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

// 32 ^ 5 = 33mil
#define BLOK 32

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
   Evas_Object *bx, *bt;
   int i;
   
   printf("ADD for factory 4 %p [%i]\n", obj, (BLOK * (int)evas_object_data_get(obj, "num")));
   bx = elm_box_add(win);
   elm_box_homogeneous_set(bx, EINA_TRUE);

   for (i = 0; i < BLOK; i++)
     {
        char buf[32];
        
        snprintf(buf, sizeof(buf), "%i", 
                 (i + (BLOK * (int)evas_object_data_get(obj, "num"))));
        
        bt = elm_button_add(win);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_object_text_set(bt, buf);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
     }
   
   elm_factory_content_set(obj, bx);
   evas_object_show(bx);
}

static void
fac_realize3(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bx, *fc;
   int i;
   
   printf("ADD for factory 3 %p [%i]\n", obj, (BLOK * (int)evas_object_data_get(obj, "num")));
   bx = elm_box_add(win);
   elm_box_homogeneous_set(bx, EINA_TRUE);

   for (i = 0; i < BLOK; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_data_set(fc, "num", (void *)(i + (BLOK * (int)evas_object_data_get(obj, "num"))));
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize_end, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_box_pack_end(bx, fc);
        evas_object_show(fc);
     }

   elm_factory_content_set(obj, bx);
   evas_object_show(bx);
}

static void
fac_realize2(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bx, *fc;
   int i;
   
   printf("ADD for factory 2 %p [%i]\n", obj, (BLOK * (int)evas_object_data_get(obj, "num")));
   bx = elm_box_add(win);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   
   for (i = 0; i < BLOK; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_data_set(fc, "num", (void *)(i + (BLOK * (int)evas_object_data_get(obj, "num"))));
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize3, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_box_pack_end(bx, fc);
        evas_object_show(fc);
     }

   elm_factory_content_set(obj, bx);
   evas_object_show(bx);
}

static void
fac_realize1(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bx, *fc;
   int i;
   
   printf("ADD for factory 1 %p [%i]\n", obj, (BLOK * (int)evas_object_data_get(obj, "num")));
   bx = elm_box_add(win);
   elm_box_homogeneous_set(bx, EINA_TRUE);

   for (i = 0; i < BLOK; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_data_set(fc, "num", (void *)(i + (BLOK * (int)evas_object_data_get(obj, "num"))));
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize2, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_box_pack_end(bx, fc);
        evas_object_show(fc);
     }

   elm_factory_content_set(obj, bx);
   evas_object_show(bx);
}

void
test_factory(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *sc, *fc;
   int i;

   win = elm_win_add(NULL, "factory", ELM_WIN_BASIC);
   elm_win_title_set(win, "Factory");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_homogeneous_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   for (i = 0; i < BLOK; i++)
     {
        fc = elm_factory_add(win);
        // initial height per factory of 1000
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_data_set(fc, "num", (void *)i);
        evas_object_size_hint_min_set(fc, 0, 1000);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize1, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_box_pack_end(bx, fc);
        evas_object_show(fc);
     }

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   elm_scroller_content_set(sc, bx);
   evas_object_show(bx);

   evas_object_show(sc);
   
   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
