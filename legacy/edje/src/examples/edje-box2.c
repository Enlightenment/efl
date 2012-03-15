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
# include "config.h"
#else
# define __UNUSED__
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Edje.h>

static const char commands[] = \
  "commands are:\n"
  "\tShift - remove box\n"
  "\tCtrl - insert box\n"
  "\tEsc - exit\n"
  "\th - print help\n";

static void
custom_layout(Evas_Object *o, Evas_Object_Box_Data *p, void *data __UNUSED__)
{
   Evas_Object_Box_Option *opt;
   Eina_List *l;
   int x, y, w, h;
   int xx, yy, ww, hh;
   int count;

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
on_keydown(void *data, Evas *evas, Evas_Object *o __UNUSED__, void *einfo)
{
   Evas_Event_Key_Down *ev;
   Evas_Object         *edje_obj;
   const Evas_Modifier *mods;

   ev = (Evas_Event_Key_Down *)einfo;
   edje_obj = (Evas_Object *)data;

   mods = evas_key_modifier_get(evas);
   if (!strcmp(ev->keyname, "h"))
     {
        fprintf(stdout, commands);
        return;
     }
   if (evas_key_modifier_is_set(mods, "Shift"))
     {
	int pos;
	Evas_Object *obj = NULL;
	pos = atoi(ev->keyname);
	obj = edje_object_part_box_remove_at(edje_obj, "example/box", pos);
	if (obj)
	  evas_object_del(obj);
	return;
     }
   if (evas_key_modifier_is_set(mods, "Control"))
     {
	Evas_Object *obj;
	int pos;
	pos = atoi(ev->keyname);
	obj = new_greenie_block(evas);
	if (!edje_object_part_box_insert_at(edje_obj, "example/box", obj, pos))
	  edje_object_part_box_append(edje_obj, "example/box", obj);
	return;
     }
   if (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
}

static Evas_Object *
box_new(Ecore_Evas *ee, const char *edje_file_path, const char *name, int x, int y, int w, int h)
{
   Evas_Object *edje_obj;

   edje_obj = edje_object_add(ecore_evas_get(ee));
   evas_object_move(edje_obj, x, y);
   evas_object_resize(edje_obj, w, h);
   if (!edje_object_file_set(edje_obj, edje_file_path, "example/group2"))
     {
	printf("error: could not load file object.\n");
     }
   evas_object_show(edje_obj);
   evas_object_name_set(edje_obj, name);
   ecore_evas_data_set(ee, "edje_obj", edje_obj);

   return edje_obj;
}

static void
on_resize(Ecore_Evas *ee)
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
on_destroy(Ecore_Evas *ee __UNUSED__)
{
   ecore_main_loop_quit();
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
   Evas_Object *last;
   Evas_Object *o;
   Eina_Prefix *pfx;
   int          w;
   int          h;
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
   ee = ecore_evas_new(NULL, 0, 0, 640, 480, NULL);
   if (!ee)
     goto free_prefix;

   ecore_evas_callback_resize_set(ee, on_resize);
   ecore_evas_callback_destroy_set(ee, on_destroy);
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_resize(bg, w, h);
   evas_object_show(bg);
   evas_object_focus_set(bg, 1);
   ecore_evas_data_set(ee, "background", bg);

   edje_box_layout_register("custom_layout", custom_layout, NULL, NULL, NULL, NULL);

   snprintf(edje_file_path, sizeof(edje_file_path),
            "%s/examples/%s", eina_prefix_data_get(pfx), edje_file);
   edje_obj = box_new(ee, edje_file_path, "box", 0, 0, w, h);
   evas_object_event_callback_add(bg, EVAS_CALLBACK_KEY_DOWN, on_keydown, edje_obj);

   for (i = 1; i <= 5; i++)
     {
	o = last = evas_object_rectangle_add(evas);
        evas_object_size_hint_min_set(o, 50, 50);
        evas_object_resize(o, 50, 50);
	evas_object_color_set(o, 255, 0, 0, 128);
	evas_object_show(o);

	if (!edje_object_part_box_append(edje_obj, "example/box", o))
	  {
	     fprintf(stderr, "error appending child object!\n");
	     return 1;
	  }
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
