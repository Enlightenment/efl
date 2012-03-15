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
# include "config.h"
#else
# define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  300
#define HEIGHT 300

#define RECTW 30
#define RECTH 30

#define NRECTS 20

static const char commands[] = \
  "commands are:\n"
  "\ti - prepend rectangle\n"
  "\ta - append rectangle\n"
  "\tc - remove\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static void
_on_destroy(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
}

/* here just to keep our example's window size and background image's
 * size in synchrony */
static void
_on_canvas_resize(Ecore_Evas *ee)
{
   Evas_Object *bg;
   Evas_Object *edje_obj;
   int          w;
   int          h;

   bg = ecore_evas_data_get(ee, "background");
   edje_obj = ecore_evas_data_get(ee, "edje_obj");
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(bg, w, h);
   evas_object_resize(edje_obj, w, h);
}

static void
_on_rect_mouse_down(void *data, Evas *e, Evas_Object *o, void *event_info)
{
   Ecore_Evas            *ee;
   Evas_Event_Mouse_Down *ev;
   Evas_Object           *edje_obj;

   ee = (Ecore_Evas *)data;
   ev = (Evas_Event_Mouse_Down *)event_info;
   edje_obj = ecore_evas_data_get(ee, "edje_obj");

   if (ev->button == 1)
     {
	printf("Removing rect %p under the mouse pointer.\n", o);
	edje_object_part_box_remove(edje_obj, "example/box", o);
	evas_object_del(o);
     }
   else if (ev->button == 3)
     {
	Evas_Object *rect;
	Eina_Bool    r;

	rect = evas_object_rectangle_add(e);
	evas_object_color_set(rect, 0, 0, 255, 255);
	evas_object_resize(rect, RECTW + 10, RECTH);
	evas_object_show(rect);

	printf("Inserting rect %p before the rectangle under the mouse pointer.\n", rect);
	r = edje_object_part_box_insert_before(edje_obj, "example/box", rect, o);
	if (!r)
	  printf("An error ocurred when appending rect %p to the box.\n", rect);

	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _on_rect_mouse_down, NULL);
     }
}

static void
_on_bg_key_down(void *data, Evas *e, Evas_Object *o __UNUSED__, void *event_info)
{
   Ecore_Evas          *ee;
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;
   Evas_Object         *rect;
   Eina_Bool            r;

   ee = (Ecore_Evas *)data;
   ev = (Evas_Event_Key_Down *)event_info;
   edje_obj = ecore_evas_data_get(ee, "edje_obj");

   if (!strcmp(ev->keyname, "h"))
     {
        fprintf(stdout, commands);
        return;
     }
   if (!strcmp(ev->keyname, "i"))
     {
	rect = evas_object_rectangle_add(e);
	evas_object_color_set(rect, 0, 0, 128, 255);
	evas_object_resize(rect, RECTW + 30, RECTH);
	evas_object_show(rect);

	printf("Inserting rect %p before the rectangle under the mouse pointer.\n", rect);
	r = edje_object_part_box_insert_at(edje_obj, "example/box", rect, 0);
	if (!r)
	  printf("An error ocurred when appending rect %p to the box.\n", rect);

	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _on_rect_mouse_down, NULL);
     }
   else if (!strcmp(ev->keyname, "a"))
     {
	rect = evas_object_rectangle_add(e);
	evas_object_color_set(rect, 0, 128, 0, 255);
	evas_object_resize(rect, RECTW, RECTH);
	evas_object_show(rect);

	printf("Inserting rect %p before the rectangle under the mouse pointer.\n", rect);
	r = edje_object_part_box_append(edje_obj, "example/box", rect);
	if (!r)
	  printf("An error ocurred when appending rect %p to the box.\n", rect);

	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _on_rect_mouse_down, NULL);
     }
   else if (!strcmp(ev->keyname, "c"))
     edje_object_part_box_remove_all(edje_obj, "example/box", EINA_TRUE);
   else if (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
   else
     {
        printf("unhandled key: %s\n", ev->keyname);
        fprintf(stdout, commands);
     }
}

int
main(int argc __UNUSED__, char *argv[])
{
   char         edje_file_path[PATH_MAX];
   const char  *edje_file = "box.edj";
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg;
   Evas_Object *edje_obj;
   Eina_Prefix *pfx;
   int          i;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   pfx = eina_prefix_new(argv[0], main,
                         "EDJE_EXAMPLES",
                         "edje/examples",
                         edje_file,
                         PACKAGE_BIN_DIR,
                         PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR,
                         PACKAGE_DATA_DIR);
   if (!pfx)
     goto shutdown_edje;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto free_prefix;

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "Edje Box Example");

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_resize(bg, WIDTH, HEIGHT);
   evas_object_focus_set(bg, EINA_TRUE);
   evas_object_show(bg);
   ecore_evas_data_set(ee, "background", bg);

   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, _on_bg_key_down, ee);

   edje_obj = edje_object_add(evas);

   snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);
   edje_object_file_set(edje_obj, edje_file_path, "example/group");
   evas_object_move(edje_obj, 0, 0);
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   evas_object_show(edje_obj);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);

   for (i = 0; i < NRECTS; i++)
     {
	Evas_Object *rect;
	Eina_Bool r = EINA_FALSE;
	int red = (i * 10) % 256;
	rect = evas_object_rectangle_add(evas);
	evas_object_color_set(rect, red, 0, 0, 255);
	evas_object_resize(rect, RECTW, RECTH);
	r = edje_object_part_box_append(edje_obj, "example/box", rect);
	if (!r)
	  printf("An error ocurred when appending rect #%d to the box.\n", i);
	evas_object_show(rect);

	evas_object_event_callback_add(
	   rect, EVAS_CALLBACK_MOUSE_DOWN, _on_rect_mouse_down, ee);
     }

   fprintf(stdout, commands);

   ecore_evas_show(ee);

   ecore_main_loop_begin();

   eina_prefix_free(pfx);
   ecore_evas_free(ee);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

 free_prefix:
   eina_prefix_free(pfx);
 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}
