//Compile with:
//gcc -g -Wall -o ecore_animator_example ecore_animator_example.c `pkg-config --cflags --libs evas ecore-evas ecore`

#include <Ecore.h>
#include <Ecore_Evas.h>

static Eina_Bool _advance_frame(void *data, double pos);
static Eina_Bool _advance_frame2(void *data, double pos);
static Eina_Bool _advance_frame3(void *data);
static Eina_Bool _start_second_anim(void *data);
static Eina_Bool _freeze_third_anim(void *data);
static Eina_Bool _thaw_third_anim(void *data);

int
main(void)
{
   Evas_Object *rect, *bg, *rect2;
   Ecore_Evas *ee;
   Evas *evas;
   Ecore_Animator *anim;

   ecore_evas_init();

   ee = ecore_evas_new(NULL, 0, 0, 300, 400, NULL);
   ecore_evas_show(ee);
   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_resize(bg, 300, 400);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas);
   evas_object_color_set(rect, 0, 0, 255, 255);
   evas_object_resize(rect, 50, 50);
   evas_object_show(rect);

   rect2 = evas_object_rectangle_add(evas);
   evas_object_color_set(rect2, 0, 55, 0, 255);
   evas_object_resize(rect2, 50, 50);
   evas_object_show(rect2);

   ecore_animator_frametime_set(1. / 50);
   ecore_animator_timeline_add(5, _advance_frame, rect);

   anim = ecore_animator_add(_advance_frame3, rect2);

   ecore_timer_add(10, _start_second_anim, rect);
   ecore_timer_add(5, _freeze_third_anim, anim);
   ecore_timer_add(10, _thaw_third_anim, anim);
   ecore_main_loop_begin();

   evas_object_del(rect);
   ecore_evas_free(ee);
   ecore_animator_del(anim);
   ecore_evas_shutdown();

   return 0;
}

static Eina_Bool
_advance_frame(void *data, double pos)
{
   double frame = pos;
   frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_SPRING, 1.2, 15);

   evas_object_resize(data, 50 * (1 + frame), 50 * (1 + frame));
   evas_object_move(data, 100 * frame, 100 * frame);
   evas_object_color_set(data, 255 * frame, 0, 255 * (1 - frame), 255);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_start_second_anim(void *data)
{
   ecore_animator_frametime_set(1. / 10);
   ecore_animator_timeline_add(20, _advance_frame2, data);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_advance_frame2(void *data, double pos)
{
   double frame = pos;
   frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_BOUNCE, 1.2, 50);

   evas_object_resize(data, 100 - (50 * frame), 100 - (50 * frame));
   evas_object_move(data, 100 * (1 - frame), 100 * (1 - frame));
   evas_object_color_set(data, 255 * (1 - frame), 0, 255 * frame, 255);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_advance_frame3(void *data)
{
   static int x = 0;

   if (x >= 250)
     x = 0;
   evas_object_move(data, ++x, 350);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_freeze_third_anim(void *data)
{
   ecore_animator_freeze(data);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_thaw_third_anim(void *data)
{
   ecore_animator_thaw(data);
   return ECORE_CALLBACK_CANCEL;
}

