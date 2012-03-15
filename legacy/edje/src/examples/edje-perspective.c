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
# include "config.h"
#else
# define __UNUSED__
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  480
#define HEIGHT 320

static const char commands[] = \
  "commands are:\n"
  "\tDown - move part down\n"
  "\tUp -  move part up\n"
  "\tLeft - move part left\n"
  "\tRight - move part right\n"
  "\tPrior - move part up-left\n"
  "\tNext - move part down-right\n"
  "\tInsert - increase focal\n"
  "\tSuppr - decrease focal\n"
  "\tEsc - exit\n"
  "\th - print help\n";

struct _App {
    Evas_Object *edje_obj;
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
_on_canvas_resize(Ecore_Evas *ee)
{
   int w, h;
   struct _App *app = ecore_evas_data_get(ee, "app");

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(app->bg, w, h);
   evas_object_resize(app->edje_obj, w, h);
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
   edje_object_signal_emit(app->edje_obj, emission, "");
   app->animating = EINA_TRUE;
}


static void
_on_bg_key_down(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   struct _App *app = data;
   Evas_Event_Key_Down *ev = event_info;

   if (!strcmp(ev->keyname, "h"))
     {
        fprintf(stdout, commands);
        return;
     }
   // just moving the part and text
   else if (!strcmp(ev->keyname, "Down"))
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
	edje_object_calc_force(app->edje_obj);
     }
   else if (!strcmp(ev->keyname, "KP_Subtract"))
     {
	app->focal -= 5;
	if (app->focal < 5)
	  app->focal = 5;

	edje_perspective_set(app->ps, 240, 160, 0, app->focal);
	edje_object_calc_force(app->edje_obj);
     }
   // exiting
   else if (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
   else
     {
        printf("unhandled key: %s\n", ev->keyname);
        fprintf(stdout, commands);
     }
}

static void
_animation_end_cb(void *data, Evas_Object *o __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   struct _App *app = data;

   app->animating = EINA_FALSE;
}

int
main(int argc __UNUSED__, char *argv[])
{
   char         edje_file_path[PATH_MAX];
   const char  *edje_file = "perspective.edj";
   struct _App  app;
   Ecore_Evas  *ee;
   Evas        *evas;
   Eina_Prefix *pfx;

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

   edje_frametime_set(1.0 / 60.0);

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   app.animating = EINA_FALSE;
   app.x = 0;
   app.y = 0;
   app.focal = 50;

   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee)
     goto free_prefix;

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_callback_resize_set(ee, _on_canvas_resize);
   ecore_evas_title_set(ee, "Edje Box Example");

   ecore_evas_data_set(ee, "app", &app);

   evas = ecore_evas_get(ee);

   app.bg = evas_object_rectangle_add(evas);
   evas_object_color_set(app.bg, 255, 255, 255, 255);
   evas_object_resize(app.bg, WIDTH, HEIGHT);
   evas_object_focus_set(app.bg, EINA_TRUE);
   evas_object_show(app.bg);

   evas_object_event_callback_add(app.bg, EVAS_CALLBACK_KEY_DOWN, _on_bg_key_down, &app);

   app.edje_obj = edje_object_add(evas);

   snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);
   edje_object_file_set(app.edje_obj, edje_file_path, "example/group");
   evas_object_move(app.edje_obj, 0, 0);
   evas_object_resize(app.edje_obj, WIDTH, HEIGHT);
   evas_object_show(app.edje_obj);

   edje_object_signal_callback_add(app.edje_obj, "animation,end", "", _animation_end_cb, &app);

   app.ps = edje_perspective_new(evas);
   edje_perspective_set(app.ps, 240, 160, 0, app.focal);
   edje_perspective_global_set(app.ps, EINA_TRUE);

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
