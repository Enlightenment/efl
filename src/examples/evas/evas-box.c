/**
 * Simple Evas example illustrating a custom Evas box object
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o evas-box evas-box.c `pkg-config --libs --cflags evas ecore ecore-evas eina`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH  (640)
#define HEIGHT (480)

static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";

static const char commands[] = \
  "commands are:\n"
  "\ta - change the box's alignment values\n"
  "\tp - change the box's padding values\n"
  "\t1 - change the box's layout to horizontal\n"
  "\t2 - change the box's layout to vertical\n"
  "\t3 - change the box's layout to horizontal homogeneous\n"
  "\t4 - change the box's layout to vertical homogeneous\n"
  "\t5 - change the box's layout to horizontal maximum size homogeneous\n"
  "\t6 - change the box's layout to vertical maximum size homogeneous\n"
  "\t7 - change the box's layout to horizontal flow\n"
  "\t8 - change the box's layout to vertical flow\n"
  "\t9 - change the box's layout to stack\n"
  "\t0 - change the box's layout to a custom-made one\n"
  "\tCtrl + NUMBER - insert a new child object at that position in the box\n"
  "\tShift + NUMBER - remove the child object at that position in the box\n"
  "\th - print help\n";

struct exemple_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *bg, *box, *border;
};

static struct exemple_data d;

static void /* custom 'diagonal' layout */
_custom_layout(Evas_Object          *o,
               Evas_Object_Box_Data *p,
               void                 *data EINA_UNUSED)
{
   int x, y, w, h;
   int xx, yy, ww, hh;
   int count;
   Eina_List *l;
   Evas_Object_Box_Option *opt;

   evas_object_geometry_get(o, &x, &y, &w, &h);
   count = eina_list_count(p->children);
   ww = w / (count ? : 1);
   hh = h / (count ? : 1);
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

static Evas_Object * /* new rectangle to be put in the box */
_new_rectangle_add(Evas *e)
{
   Evas_Object *o;

   o = evas_object_rectangle_add(e);
   evas_object_resize(o, 10, 10);
   evas_object_color_set(o, 0, 255, 0, 255);
   evas_object_show(o);

   return o;
}

/* use the following commands to interact with this example - 'h' is
 * the key for help */
static void
_on_keydown(void        *data EINA_UNUSED,
            Evas        *evas EINA_UNUSED,
            Evas_Object *o EINA_UNUSED,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;
   const Evas_Modifier *mods = evas_key_modifier_get(evas);

   if (strcmp(ev->key, "h") == 0) /* print help */
     {
        fprintf(stdout, commands);
        return;
     }

   if (evas_key_modifier_is_set(mods, "Shift"))
     {
        int pos;
        Eina_Bool ret;
        Evas_Object *obj;
        Eina_List *children;

        pos = atoi(ev->key);
        children = evas_object_box_children_get(d.box);

        obj = eina_list_nth(children, pos);
        if (!obj) goto list_free;

        ret = evas_object_box_remove_at(d.box, pos);
        if (ret) evas_object_del(obj);

list_free:
        eina_list_free(children);
        return;
     }

   if (evas_key_modifier_is_set(mods, "Control"))
     {
        Evas_Object *o;
        int pos;
        pos = atoi(ev->key);
        o = _new_rectangle_add(d.evas);
        if (!evas_object_box_insert_at(d.box, o, pos))
          evas_object_box_append(d.box, o);
        return;
     }

   if (strcmp(ev->key, "a") == 0)
     {
        double h, v;

        evas_object_box_align_get(d.box, &h, &v);

        if (h == 0.5)
          h = v = 1.0;
        else if (h == 1.0)
          h = v = -1.0;
        else if (h == -1.0)
          h = v = 0.0;
        else if (h == 0.0)
          h = v = 0.5;

        evas_object_box_align_set(d.box, h, v);

        fprintf(stdout, "Applying new alignment values (%.1f, %.1f)"
                        " on the box\n", h, v);
        return;
     }

   if (strcmp(ev->key, "p") == 0)
     {
        int h, v;

        evas_object_box_padding_get(d.box, &h, &v);

        if (h == 0)
          h = v = 50;
        else
          h = v = 0;

        evas_object_box_padding_set(d.box, h, v);

        fprintf(stdout, "Applying new padding values (%d, %d)"
                        " on the box\n", h, v);
        return;
     }

   if (strcmp(ev->key, "1") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_horizontal, NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n", "horizontal");
        return;
     }

   if (strcmp(ev->key, "2") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_vertical, NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n", "vertical");
        return;
     }

   if (strcmp(ev->key, "3") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_homogeneous_horizontal, NULL,
          NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n",
                "horizontal homogeneous");
        return;
     }

   if (strcmp(ev->key, "4") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_homogeneous_vertical, NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n",
                "vertical homogeneous");
        return;
     }

   if (strcmp(ev->key, "5") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_homogeneous_max_size_horizontal,
          NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n",
                "horizontal maximum size homogeneous");
        return;
     }

   if (strcmp(ev->key, "6") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_homogeneous_max_size_vertical,
          NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n",
                "vertical maximum size homogeneous");
        return;
     }

   if (strcmp(ev->key, "7") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_flow_horizontal, NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n", "horizontal flow");
        return;
     }

   if (strcmp(ev->key, "8") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_flow_vertical, NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n", "vertical flow");
        return;
     }

   if (strcmp(ev->key, "9") == 0)
     {
        evas_object_box_layout_set(
          d.box, evas_object_box_layout_stack, NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n", "stack");
        return;
     }

   if (strcmp(ev->key, "0") == 0)
     {
        evas_object_box_layout_set(d.box, _custom_layout, NULL, NULL);

        fprintf(stdout, "Applying '%s' layout on the box\n", "CUSTOM");
        return;
     }
}

static void
_on_delete(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void /* adjust canvas' contents on resizes */
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   evas_object_resize(d.bg, w, h);

   evas_object_move(d.box, (w / 4), (h / 4));
   evas_object_resize(d.box, (w / 2), (h / 2));

   evas_object_move(d.border, (w / 4) - 2, (h / 4) - 2);
   evas_object_resize(d.border, (w / 2) + 4, (h / 2) + 4);
}

int
main(void)
{
   Evas_Object *last, *o;
   int i;

   srand(time(NULL));

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto panic;

   ecore_evas_callback_delete_request_set(d.ee, _on_delete);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.box = evas_object_box_add(d.evas);
   evas_object_show(d.box);

   for (i = 1; i <= 5; i++)
     {
        o = last = evas_object_rectangle_add(d.evas);
        evas_object_size_hint_min_set(o, 50, 50);
        evas_object_color_set(
          o, rand() % 256, rand() % 256, rand() % 256, 255);
        evas_object_show(o);

        if (!evas_object_box_append(d.box, o))
          {
             fprintf(stderr, "Error appending child object on the box!\n");
             goto error;
          }
     }

   /* this is a border around the box, container of the rectangles we
    * are going to experiment with. this way you can see how the
    * container relates to the children */
   d.border = evas_object_image_filled_add(d.evas);
   evas_object_image_file_set(d.border, border_img_path, NULL);
   evas_object_image_border_set(d.border, 2, 2, 2, 2);
   evas_object_image_border_center_fill_set(d.border, EVAS_BORDER_FILL_NONE);
   evas_object_show(d.border);

   fprintf(stdout, commands);

   _canvas_resize_cb(d.ee);
   ecore_main_loop_begin();
   ecore_evas_shutdown();
   return 0;

error:
   ecore_evas_shutdown();
   return -1;

panic:
   fprintf(stderr, "You got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   return -2;
}

