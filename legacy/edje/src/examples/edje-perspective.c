/**
 * Simple Edje example illustrating drag functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc drag.edc && gcc -o drag-box drag-box.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  480
#define HEIGHT 320

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/perspective.edj";

struct _App {
    Ecore_Evas *ee;
    Evas_Object *edje;
    Evas_Object *bg;
    Edje_Perspective *ps;
    Eina_Bool animating;
    int x, y; // relative position of part in the screen
    int focal;
};

static void
_on_destroy(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

/* here just to keep our example's window size and background image's
 * size in synchrony */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;
   struct _App *app = ecore_evas_data_get(ee, "app");

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(app->bg, w, h);
   evas_object_resize(app->edje, w, h);
}

static void
_part_move(struct _App *app, int dx, int dy)
{
   char emission[64];

   if (app->animating)
     return;

   app->x += dx;
   app->y += dy;
   if (app->x > 1)
     app->x = 1;
   if (app->x < 0)
     app->x = 0;
   if (app->y > 1)
     app->y = 1;
   if (app->y < 0)
     app->y = 0;

   snprintf(emission, sizeof(emission), "move,%d,%d", app->x, app->y);
   edje_object_signal_emit(app->edje, emission, "");
   app->animating = EINA_TRUE;
}


static void
_bg_key_down(void *data, Evas *e, Evas_Object *o __UNUSED__, void *event_info)
{
   struct _App *app = data;
   Evas_Event_Key_Down *ev = event_info;


   // just moving the part and text
   if (!strcmp(ev->keyname, "Down"))
     {
	_part_move(app, 0, 1);
     }
   else if (!strcmp(ev->keyname, "Up"))
     {
	_part_move(app, 0, -1);
     }
   else if (!strcmp(ev->keyname, "Left"))
     {
	_part_move(app, -1, 0);
     }
   else if (!strcmp(ev->keyname, "Right"))
     {
	_part_move(app, 1, 0);
     }
   else if (!strcmp(ev->keyname, "Prior"))
     {
	_part_move(app, -1, -1);
     }
   else if (!strcmp(ev->keyname, "Next"))
     {
	_part_move(app, 1, 1);
     }
   // adjusting the perspective focal point distance
   else if (!strcmp(ev->keyname, "KP_Add"))
     {
	app->focal += 5;
	edje_perspective_set(app->ps, 240, 160, 0, app->focal);
	edje_object_calc_force(app->edje);
     }
   else if (!strcmp(ev->keyname, "KP_Subtract"))
     {
	app->focal -= 5;
	if (app->focal < 5)
	  app->focal = 5;

	edje_perspective_set(app->ps, 240, 160, 0, app->focal);
	edje_object_calc_force(app->edje);
     }
   // exiting
   else if (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
   else
     printf("unhandled key: %s\n", ev->keyname);
}

static void
_animation_end_cb(void *data, Evas_Object *o __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   struct _App *app = data;

   app->animating = EINA_FALSE;
}

int
main(void)
{
   Evas *evas;
   struct _App app;
   int i;

   ecore_evas_init();
   edje_init();

   edje_frametime_set(((double)1) / 60);

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   app.animating = EINA_FALSE;
   app.x = 0;
   app.y = 0;
   app.focal = 50;
   app.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   ecore_evas_callback_destroy_set(app.ee, _on_destroy);
   ecore_evas_callback_resize_set(app.ee, _canvas_resize_cb);
   ecore_evas_title_set(app.ee, "Edje Box Example");
   ecore_evas_show(app.ee);

   ecore_evas_data_set(app.ee, "app", &app);

   evas = ecore_evas_get(app.ee);

   app.bg = evas_object_rectangle_add(evas);
   evas_object_color_set(app.bg, 255, 255, 255, 255);
   evas_object_resize(app.bg, WIDTH, HEIGHT);
   evas_object_focus_set(app.bg, EINA_TRUE);
   evas_object_show(app.bg);

   evas_object_event_callback_add(app.bg, EVAS_CALLBACK_KEY_DOWN, _bg_key_down, &app);

   app.edje = edje_object_add(evas);

   edje_object_file_set(app.edje, edje_file_path, "example/group");
   evas_object_move(app.edje, 0, 0);
   evas_object_resize(app.edje, WIDTH, HEIGHT);
   evas_object_show(app.edje);

   edje_object_signal_callback_add(app.edje, "animation,end", "", _animation_end_cb, &app);

   app.ps = edje_perspective_new(evas);
   edje_perspective_set(app.ps, 240, 160, 0, app.focal);
   edje_perspective_global_set(app.ps, EINA_TRUE);

   ecore_main_loop_begin();

   ecore_evas_free(app.ee);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;
}
