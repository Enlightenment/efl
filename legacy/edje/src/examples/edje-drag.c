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

#define WIDTH  300
#define HEIGHT 300

#define RECTW 30
#define RECTH 30

#define NRECTS 20

static const char *PARTNAME = "example/knob";

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/drag.edj";

struct _App {
    Ecore_Evas *ee;
    Evas_Object *edje;
    Evas_Object *bg;
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
_bg_key_down(void *data, Evas *e, Evas_Object *o __UNUSED__, void *event_info)
{
   struct _App *app = data;
   Evas_Event_Key_Down *ev = event_info;
   Evas_Object *rect;
   Eina_Bool r;


   if (!strcmp(ev->keyname, "Down"))
     {
	edje_object_part_drag_step(app->edje, PARTNAME, 0, 1.0);
     }
   else if (!strcmp(ev->keyname, "Up"))
     {
	edje_object_part_drag_step(app->edje, PARTNAME, 0, -1.0);
     }
   else if (!strcmp(ev->keyname, "m"))
     {
	edje_object_part_drag_value_set(app->edje, PARTNAME, 0.0, 0.5);
     }
   else if (!strcmp(ev->keyname, "Prior"))
     {
	edje_object_part_drag_page(app->edje, PARTNAME, 0.0, -1.0);
     }
   else if (!strcmp(ev->keyname, "Next"))
     {
	edje_object_part_drag_page(app->edje, PARTNAME, 0.0, 1.0);
     }
   else if (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
   else
     printf("unhandled key: %s\n", ev->keyname);
}

static void
_knob_moved_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   double val;
   edje_object_part_drag_value_get(o, PARTNAME, NULL, &val);
   printf("value changed to: %0.3f\n", val);
}

int
main(void)
{
   Evas *evas;
   struct _App app;
   int i;

   ecore_evas_init();
   edje_init();

   /* this will give you a window with an Evas canvas under the first
    * engine available */
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

   edje_object_part_drag_size_set(app.edje, PARTNAME, 1.0, 0.4);

   if (!edje_object_part_drag_step_set(app.edje, PARTNAME, 0.0, 0.1))
     printf("error when setting drag step size.\n");

   if (!edje_object_part_drag_page_set(app.edje, PARTNAME, 0.0, 0.3))
     printf("error when setting drag page step size.\n");

   edje_object_signal_callback_add(app.edje, "drag", PARTNAME, _knob_moved_cb, &app);

   ecore_main_loop_begin();

   ecore_evas_free(app.ee);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;
}
