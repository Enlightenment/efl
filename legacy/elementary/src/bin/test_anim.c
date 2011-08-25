#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static const char *names[] =
{
     "bub1", "sh1",
     "bub2", "sh2",
     "bub3", "sh3",
};

static void
_del(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Ecore_Animator *ani = evas_object_data_get(win, "animator");

   ecore_animator_del(ani);
}

static Eina_Bool
anim(void *data)
{
   Evas_Object *win = data;
   Evas_Object *bub, *sh;
   Evas_Coord x, y, w, h, vw, vh;
   double t, xx, yy, zz, r, fac;
   double lx, ly;
   unsigned int i;

   evas_output_viewport_get(evas_object_evas_get(win), 0, 0, &vw, &vh);
   r = 48;
   t = ecore_loop_time_get();
   fac = 2.0 / (double)((sizeof(names) / sizeof(char *) / 2));
   evas_pointer_canvas_xy_get(evas_object_evas_get(win), &x, &y);
   lx = x;
   ly = y;

   for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
     {
        bub = evas_object_data_get(win, names[i * 2]);
        sh = evas_object_data_get(win, names[(i * 2) + 1]);
        zz = (((2 + sin(t * 6 + (M_PI * (i * fac)))) / 3) * 64) * 2;
        xx = (cos(t * 4 + (M_PI * (i * fac))) * r) * 2;
        yy = (sin(t * 6 + (M_PI * (i * fac))) * r) * 2;

        w = zz;
        h = zz;
        x = (vw / 2) + xx - (w / 2);
        y = (vh / 2) + yy - (h / 2);

        evas_object_move(bub, x, y);
        evas_object_resize(bub, w, h);

        x = x - ((lx - (x + (w / 2))) / 4);
        y = y - ((ly - (y + (h / 2))) / 4);

        evas_object_move(sh, x, y);
        evas_object_resize(sh, w, h);
     }
   return ECORE_CALLBACK_RENEW;
}

void
test_anim(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bub, *sh;
   Ecore_Animator *ani;
   char buf[PATH_MAX];
   unsigned int i;

   win = elm_win_add(NULL, "animation", ELM_WIN_BASIC);
   elm_win_title_set(win, "Animation");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   snprintf(buf, sizeof(buf), "%s/images/bubble_sh.png", elm_app_data_dir_get());
   for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
     {
        sh = evas_object_image_filled_add(evas_object_evas_get(win));
        evas_object_image_file_set(sh, buf, NULL);
        evas_object_resize(sh, 64, 64);
        evas_object_show(sh);
        evas_object_data_set(win, names[(i * 2) + 1], sh);
     }

   snprintf(buf, sizeof(buf), "%s/images/bubble.png", elm_app_data_dir_get());
    for (i = 0; i < (sizeof(names) / sizeof(char *) / 2); i++)
     {
        bub = evas_object_image_filled_add(evas_object_evas_get(win));
        evas_object_image_file_set(bub, buf, NULL);
        evas_object_resize(bub, 64, 64);
        evas_object_show(bub);
        evas_object_data_set(win, names[(i * 2)], bub);
     }

   evas_object_resize(win, 480, 800);
   evas_object_show(win);

   ani = ecore_animator_add(anim, win);
   evas_object_data_set(win, "animator", ani);

   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _del, win);
}
#endif
