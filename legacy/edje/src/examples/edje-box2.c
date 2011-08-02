/**
 * Simple Edje example illustrating a custom box layout.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc box.edc && gcc -o edje-box2 edje-box2.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#define __UNUSED__
#endif

#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/box.edj";

struct _App {
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *bg;
   Evas_Object *box;
};

static struct _App app;

static void
custom_layout(Evas_Object *o, Evas_Object_Box_Data *p, void *data)
{
   int x, y, w, h;
   int xx, yy, ww, hh;
   int count;
   Eina_List *l;
   Evas_Object_Box_Option *opt;

   evas_object_geometry_get(o, &x, &y, &w, &h);
   count = eina_list_count(p->children);
   ww = w / (count?:1);
   hh = h / (count?:1);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;

   xx = x;
   yy = y;
   EINA_LIST_FOREACH(p->children, l, opt)
     {
	evas_object_move(opt->obj, xx, yy);
	xx += ww;
	yy += hh;
     }
}

static Evas_Object *
new_greenie_block(Evas *e)
{
   Evas_Object *o;

   o = evas_object_rectangle_add(e);
   evas_object_resize(o, 10, 10);
   evas_object_color_set(o, 0, 255, 0, 255);
   evas_object_show(o);

   return o;
}

static void
on_keydown(void *data, Evas *evas, Evas_Object *o, void *einfo)
{
   struct _App *app = data;
   Evas_Event_Key_Down *ev = einfo;
   const Evas_Modifier *mods;

   mods = evas_key_modifier_get(evas);
   if (evas_key_modifier_is_set(mods, "Shift"))
     {
	int pos;
	Evas_Object *obj = NULL;
	pos = atoi(ev->keyname);
	obj = edje_object_part_box_remove_at(app->box, "example/box", pos);
	if (obj)
	  evas_object_del(obj);
	return;
     }
   if (evas_key_modifier_is_set(mods, "Control"))
     {
	Evas_Object *o;
	int pos;
	pos = atoi(ev->keyname);
	o = new_greenie_block(app->evas);
	if (!edje_object_part_box_insert_at(app->box, "example/box", o, pos))
	  edje_object_part_box_append(app->box, "example/box", o);
	return;
     }
   if (strcmp(ev->keyname, "Escape") == 0)
     ecore_main_loop_quit();
}

static Evas_Object *
box_new(Evas *evas, const char *name, int x, int y, int w, int h)
{
   Evas_Object *o;

   o = edje_object_add(evas);
   evas_object_move(o, x, y);
   evas_object_resize(o, w, h);
   if (!edje_object_file_set(o, edje_file_path, "example/group2"))
     {
	printf("error: could not load file object.\n");
     }
   evas_object_show(o);

   evas_object_name_set(o, name);

   return o;
}

static void
on_resize(Ecore_Evas *ee)
{
   int w, h;

   evas_output_viewport_get(app.evas, NULL, NULL, &w, &h);
   evas_object_resize(app.bg, w, h);
   evas_object_resize(app.box, w, h);
}

static void
on_destroy(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

int
main(int argc, char *argv[])
{
   Ecore_Evas *ee;
   int w, h, i;
   Evas_Object *last;
   Evas_Object *o;

   evas_init();
   ecore_init();
   ecore_evas_init();
   edje_init();

   ee = ecore_evas_new(NULL, 0, 0, 640, 480, NULL);
   ecore_evas_show(ee);

   app.ee = ee;
   app.evas = ecore_evas_get(ee);

   ecore_evas_callback_resize_set(ee, on_resize);
   ecore_evas_callback_destroy_set(ee, on_destroy);

   evas_output_viewport_get(app.evas, NULL, NULL, &w, &h);

   app.bg = evas_object_rectangle_add(app.evas);
   evas_object_resize(app.bg, w, h);
   evas_object_show(app.bg);
   evas_object_focus_set(app.bg, 1);
   evas_object_event_callback_add(
      app.bg, EVAS_CALLBACK_KEY_DOWN, on_keydown, &app);

   edje_box_layout_register("custom_layout", custom_layout, NULL, NULL, NULL, NULL);

   app.box = box_new(app.evas, "box", 0, 0, w, h);

   for (i = 1; i <= 5; i++)
     {
	o = last = evas_object_rectangle_add(app.evas);
        evas_object_size_hint_min_set(o, 50, 50);
        evas_object_resize(o, 50, 50);
	evas_object_color_set(o, 255, 0, 0, 128);
	evas_object_show(o);

	if (!edje_object_part_box_append(app.box, "example/box", o))
	  {
	     fprintf(stderr, "error appending child object!\n");
	     return 1;
	  }
     }

   ecore_main_loop_begin();

   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   evas_shutdown();


   return 0;
}
