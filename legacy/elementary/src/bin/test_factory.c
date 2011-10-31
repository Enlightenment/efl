#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

// 16 ^ 4 = 65k
#define BLOK 16
// homogenous layout
//#define HOMOG 1
// aligned to top of box
#define ZEROALIGN 1
#define DEFSZ 64

static void
fac_unrealize(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   // setting factory content to null deletes it
   printf("--------DELETE for factory %p [f: %p]\n", elm_object_content_get(obj), obj);
   elm_object_content_set(obj, NULL);
}

static void
fac_realize_end(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bx, *bt;
   int i;

   bx = elm_box_add(win);
   printf("   ADD lv 3 = %p [%i]\n", bx, (BLOK * (int)evas_object_data_get(obj, "num")));
#ifdef HOMOG
   elm_box_homogeneous_set(bx, EINA_TRUE);
#endif
#ifdef ZEROALIGN
   elm_box_align_set(bx, 0.0, 0.0);
#endif

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

   elm_object_content_set(obj, bx);
   evas_object_show(bx);
}

static void
fac_realize2(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bx, *fc;
   int i;

   bx = elm_box_add(win);
   printf("  ADD lv 2 = %p [%i]\n", bx, (BLOK * (int)evas_object_data_get(obj, "num")));
#ifdef HOMOG
   elm_box_homogeneous_set(bx, EINA_TRUE);
#endif
#ifdef ZEROALIGN
   elm_box_align_set(bx, 0.0, 0.0);
#endif

   for (i = 0; i < BLOK; i++)
     {
        fc = elm_factory_add(win);
        elm_factory_maxmin_mode_set(fc, EINA_TRUE);
        // initial height per factory of DEFSZ
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_data_set(fc, "num", (void *)(i + (BLOK * (int)evas_object_data_get(obj, "num"))));
        evas_object_size_hint_min_set(fc, 0, DEFSZ);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize_end, win);
        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_box_pack_end(bx, fc);
        evas_object_show(fc);
     }

   elm_object_content_set(obj, bx);
   evas_object_show(bx);
}

static void
fac_realize1(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bx, *fc;
   int i;

   bx = elm_box_add(win);
   printf(" ADD lv 1 = %p [%i]\n", bx, (BLOK * (int)evas_object_data_get(obj, "num")));
#ifdef HOMOG
   elm_box_homogeneous_set(bx, EINA_TRUE);
#endif
#ifdef ZEROALIGN
   elm_box_align_set(bx, 0.0, 0.0);
#endif

   for (i = 0; i < BLOK; i++)
     {
        fc = elm_factory_add(win);
        elm_factory_maxmin_mode_set(fc, EINA_TRUE);
        // initial height per factory of DEFSZ
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_data_set(fc, "num", (void *)(i + (BLOK * (int)evas_object_data_get(obj, "num"))));
        evas_object_size_hint_min_set(fc, 0, DEFSZ);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize2, win);
//        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_box_pack_end(bx, fc);
        evas_object_show(fc);
     }

   elm_object_content_set(obj, bx);
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
#ifdef HOMOG
   elm_box_homogeneous_set(bx, EINA_TRUE);
#endif
#ifdef ZEROALIGN
   elm_box_align_set(bx, 0.0, 0.0);
#endif
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, 0.0);

   for (i = 0; i < BLOK; i++)
     {
        fc = elm_factory_add(win);
        elm_factory_maxmin_mode_set(fc, EINA_TRUE);
        // initial height per factory of DEFSZ
        // scrollbar will be wrong until enough
        // children have been realized and the
        // real size is known
        evas_object_data_set(fc, "num", (void *)i);
        evas_object_size_hint_min_set(fc, 0, DEFSZ);
        evas_object_size_hint_weight_set(fc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(fc, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_smart_callback_add(fc, "realize", fac_realize1, win);
//        evas_object_smart_callback_add(fc, "unrealize", fac_unrealize, win);
        elm_box_pack_end(bx, fc);
        evas_object_show(fc);
     }

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);

   elm_object_content_set(sc, bx);
   evas_object_show(bx);

   evas_object_show(sc);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
