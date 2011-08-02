/**
 * Simple Edje example illustrating box functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc box.edc && gcc -o edje-box edje-box.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/box.edj";

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
_rect_mouse_down(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   struct _App *app = data;
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button == 1)
     {
	printf("Removing rect %p under the mouse pointer.\n", o);
	edje_object_part_box_remove(app->edje, "example/box", o);
	evas_object_del(o);
     }
   else if (ev->button == 3)
     {
	Evas_Object *rect;
	Eina_Bool r;

	rect = evas_object_rectangle_add(e);
	evas_object_color_set(rect, 0, 0, 255, 255);
	evas_object_resize(rect, RECTW + 10, RECTH);
	evas_object_show(rect);

	printf("Inserting rect %p before the rectangle under the mouse pointer.\n", rect);
	r = edje_object_part_box_insert_before(app->edje, "example/box", rect, o);
	if (!r)
	  printf("An error ocurred when appending rect %p to the box.\n", rect);

	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _rect_mouse_down, app);
     }
}

static void
_bg_key_down(void *data, Evas *e, Evas_Object *o __UNUSED__, void *event_info)
{
   struct _App *app = data;
   Evas_Event_Key_Down *ev = event_info;
   Evas_Object *rect;
   Eina_Bool r;


   if (!strcmp(ev->keyname, "i"))
     {
	rect = evas_object_rectangle_add(e);
	evas_object_color_set(rect, 0, 0, 128, 255);
	evas_object_resize(rect, RECTW + 30, RECTH);
	evas_object_show(rect);

	printf("Inserting rect %p before the rectangle under the mouse pointer.\n", rect);
	r = edje_object_part_box_insert_at(app->edje, "example/box", rect, 0);
	if (!r)
	  printf("An error ocurred when appending rect %p to the box.\n", rect);

	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _rect_mouse_down, app);
     }
   else if (!strcmp(ev->keyname, "a"))
     {
	rect = evas_object_rectangle_add(e);
	evas_object_color_set(rect, 0, 128, 0, 255);
	evas_object_resize(rect, RECTW, RECTH);
	evas_object_show(rect);

	printf("Inserting rect %p before the rectangle under the mouse pointer.\n", rect);
	r = edje_object_part_box_append(app->edje, "example/box", rect);
	if (!r)
	  printf("An error ocurred when appending rect %p to the box.\n", rect);

	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _rect_mouse_down, app);
     }
   else if (!strcmp(ev->keyname, "c"))
     edje_object_part_box_remove_all(app->edje, "example/box", EINA_TRUE);
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

   for (i = 0; i < NRECTS; i++)
     {
	Evas_Object *rect;
	Eina_Bool r = EINA_FALSE;
	int red = (i * 10) % 256;
	rect = evas_object_rectangle_add(evas);
	evas_object_color_set(rect, red, 0, 0, 255);
	evas_object_resize(rect, RECTW, RECTH);
	r = edje_object_part_box_append(app.edje, "example/box", rect);
	if (!r)
	  printf("An error ocurred when appending rect #%d to the box.\n", i);
	evas_object_show(rect);

	evas_object_event_callback_add(
	   rect, EVAS_CALLBACK_MOUSE_DOWN, _rect_mouse_down, &app);
     }

   ecore_main_loop_begin();

   ecore_evas_free(app.ee);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;
}
