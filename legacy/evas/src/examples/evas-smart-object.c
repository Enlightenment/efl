/**
 * Simple Evas example illustrating a custom Evas smart object
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o evas-smart-object evas-smart-object.c `pkg-config --libs --cflags evas ecore ecore-evas`
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
#include <stdio.h>
#include <errno.h>

#define WIDTH  (320)
#define HEIGHT (240)

static const char *commands = \
  "commands are:\n"
  "\tl - insert child rectangle on the left\n"
  "\tr - insert child rectangle on the right\n"
  "\tright arrow - move smart object to the right\n"
  "\tleft arrow - move smart object to the left\n"
  "\tup arrow - move smart object up\n"
  "\tdown arrow - move smart object down\n"
  "\td - decrease smart object's size\n"
  "\ti - increase smart object's size\n"
  "\tc - change smart object's clipper color\n"
  "\th - print help\n";

#define WHITE {255, 255, 255, 255}
#define RED   {255, 0, 0, 255}
#define GREEN {0, 255, 0, 255}
#define BLUE  {0, 0, 255, 255}

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *smt, *bg, *clipper;
};

struct color_tuple
{
   int r, g, b, a;
} clipper_colors[4] = {WHITE, RED, GREEN, BLUE};
int cur_color = 0;

static const char *
_index_to_color(int i)
{
   switch (i)
     {
      case 0:
        return "WHITE (default)";

      case 1:
        return "RED";

      case 2:
        return "GREEN";

      case 3:
        return "BLUE";

      default:
        return "other";
     }
}

static struct test_data d = {0};
static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";

#define _evas_smart_example_type "Evas_Smart_Example"
#define SIG_CHILD_ADDED          "child,added"
#define SIG_CHILD_REMOVED        "child,removed"

static const Evas_Smart_Cb_Description _signals[] =
{
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

typedef struct _Evas_Smart_Example_Data Evas_Smart_Example_Data;
/*
 * This structure augments clipped smart object's instance data,
 * providing extra members required by our example smart object's
 * implementation.
 */
struct _Evas_Smart_Example_Data
{
   Evas_Object_Smart_Clipped_Data base;
   Evas_Object                   *children[2], *border;
};

#define EVAS_SMART_EXAMPLE_DATA_GET(o, ptr) \
  Evas_Smart_Example_Data * ptr = evas_object_smart_data_get(o)

#define EVAS_SMART_EXAMPLE_DATA_GET_OR_RETURN(o, ptr)        \
  EVAS_SMART_EXAMPLE_DATA_GET(o, ptr);                       \
  if (!ptr)                                                  \
    {                                                        \
       fprintf(stderr, "No widget data for object %p (%s)!", \
               o, evas_object_type_get(o));                  \
       fflush(stderr);                                       \
       abort();                                              \
       return;                                               \
    }

#define EVAS_SMART_EXAMPLE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  EVAS_SMART_EXAMPLE_DATA_GET(o, ptr);                         \
  if (!ptr)                                                    \
    {                                                          \
       fprintf(stderr, "No widget data for object %p (%s)!",   \
               o, evas_object_type_get(o));                    \
       fflush(stderr);                                         \
       abort();                                                \
       return val;                                             \
    }

EVAS_SMART_SUBCLASS_NEW(_evas_smart_example_type, _evas_smart_example,
                        Evas_Smart_Class, Evas_Smart_Class,
                        evas_object_smart_clipped_class_get, _signals);

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

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
}

static void
_on_child_del(void        *data,
              Evas        *evas __UNUSED__,
              Evas_Object *o,
              void        *einfo __UNUSED__)
{
   Evas_Object *example_smart = data;
   int index;

   EVAS_SMART_EXAMPLE_DATA_GET(example_smart, priv);

   index = (int)evas_object_data_get(o, "index");
   index--;

   priv->children[index] = NULL;

   evas_object_smart_member_del(o);
   evas_object_smart_changed(example_smart);
}

static void
_evas_smart_example_child_callbacks_unregister(Evas_Object *obj)
{
   evas_object_data_set(obj, "index", NULL);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_FREE, _on_child_del);
}

static void
_evas_smart_example_child_callbacks_register(Evas_Object *o,
                                             Evas_Object *child,
                                             int          index)
{
   evas_object_event_callback_add(child, EVAS_CALLBACK_FREE, _on_child_del, o);
   evas_object_data_set(child, "index", (void *)(++index));
}

/* create and setup a new example smart object's internals */
static void
_evas_smart_example_smart_add(Evas_Object *o)
{
   EVAS_SMART_DATA_ALLOC(o, Evas_Smart_Example_Data);

   /* this is a border around the smart object's area, delimiting it */
   priv->border = evas_object_image_filled_add(evas_object_evas_get(o));
   evas_object_image_file_set(priv->border, border_img_path, NULL);
   evas_object_image_border_set(priv->border, 3, 3, 3, 3);
   evas_object_image_border_center_fill_set(
     priv->border, EVAS_BORDER_FILL_NONE);
   evas_object_smart_member_add(priv->border, o);

   _evas_smart_example_parent_sc->add(o);
}

static void
_evas_smart_example_smart_del(Evas_Object *o)
{
   EVAS_SMART_EXAMPLE_DATA_GET(o, priv);

   if (priv->children[0])
     {
        _evas_smart_example_child_callbacks_unregister(priv->children[0]);
        priv->children[0] = NULL;
     }

   if (priv->children[1])
     {
        _evas_smart_example_child_callbacks_unregister(priv->children[1]);
        priv->children[1] = NULL;
     }

   _evas_smart_example_parent_sc->del(o);
}

static void
_evas_smart_example_smart_show(Evas_Object *o)
{
   EVAS_SMART_EXAMPLE_DATA_GET(o, priv);

   if (priv->children[0]) evas_object_show(priv->children[0]);
   if (priv->children[1]) evas_object_show(priv->children[1]);
   evas_object_show(priv->border);

   _evas_smart_example_parent_sc->show(o);
}

static void
_evas_smart_example_smart_hide(Evas_Object *o)
{
   EVAS_SMART_EXAMPLE_DATA_GET(o, priv);

   if (priv->children[0]) evas_object_hide(priv->children[0]);
   if (priv->children[1]) evas_object_hide(priv->children[1]);
   evas_object_hide(priv->border);

   _evas_smart_example_parent_sc->hide(o);
}

static void
_evas_smart_example_smart_resize(Evas_Object *o,
                                 Evas_Coord   w,
                                 Evas_Coord   h)
{
   Evas_Coord ow, oh;
   evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;

   /* this will trigger recalculation */
   evas_object_smart_changed(o);
}

/* act on child objects' properties, before rendering */
static void
_evas_smart_example_smart_calculate(Evas_Object *o)
{
   Evas_Coord x, y, w, h;

   EVAS_SMART_EXAMPLE_DATA_GET_OR_RETURN(o, priv);
   evas_object_geometry_get(o, &x, &y, &w, &h);

   evas_object_resize(priv->border, w, h);
   evas_object_move(priv->border, x, y);

   if (priv->children[0])
     {
        evas_object_move(priv->children[0], x + 3, y + 3);
        evas_object_resize(priv->children[0], (w / 2) - 3, (h / 2) - 3);
     }

   if (priv->children[1])
     {
        evas_object_move(priv->children[1], x + (w / 2), y + (h / 2));
        evas_object_resize(priv->children[1], (w / 2) - 3, (h / 2) - 3);
     }
}

/* setting our smart interface */
static void
_evas_smart_example_smart_set_user(Evas_Smart_Class *sc)
{
   /* specializing these two */
    sc->add = _evas_smart_example_smart_add;
    sc->del = _evas_smart_example_smart_del;
    sc->show = _evas_smart_example_smart_show;
    sc->hide = _evas_smart_example_smart_hide;

    /* clipped smart object has no hook on resizes or calculations */
    sc->resize = _evas_smart_example_smart_resize;
    sc->calculate = _evas_smart_example_smart_calculate;
}

/* BEGINS example smart object's own interface */

/* add a new example smart object to a canvas */
Evas_Object *
evas_smart_example_add(Evas *evas)
{
   return evas_object_smart_add(evas, _evas_smart_example_smart_class_new());
}

/* remove a child element, return its pointer (or NULL on errors) */
Evas_Object *
evas_smart_example_remove(Evas_Object *o,
                          Evas_Object *child)
{
   int index;

   EVAS_SMART_EXAMPLE_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

   if (priv->children[0] != child && priv->children[1] != child)
     {
        fprintf(stderr, "You are trying to remove something not belonging to"
                        " the example smart object!\n");
        return NULL;
     }

   index = (int)evas_object_data_get(child, "index");
   index--;

   priv->children[index] = NULL;

   evas_object_smart_callback_call(o, SIG_CHILD_REMOVED, child);
   _evas_smart_example_child_callbacks_unregister(child);
   evas_object_smart_member_del(child);
   evas_object_smart_changed(o);

   return child;
}

/* set to return any previous object set to the left position of the
 * smart object or NULL, if any (or on errors) */
Evas_Object *
evas_smart_example_set_left(Evas_Object *o,
                            Evas_Object *child)
{
   Evas_Object *ret = NULL;

   EVAS_SMART_EXAMPLE_DATA_GET_OR_RETURN_VAL(o, priv, NULL);
   if (!child)
     return NULL;

   if (priv->children[1] == child)
     {
        fprintf(stderr, "You mustn't place a child on both slots of"
                        " the example smart object!\n");
        return NULL;
     }

   if (priv->children[0])
     {
        if (priv->children[0] != child)
          ret = evas_smart_example_remove(o, priv->children[0]);
        else return child;
     }

   priv->children[0] = child;
   evas_object_smart_callback_call(o, SIG_CHILD_ADDED, child);

   _evas_smart_example_child_callbacks_register(o, child, 0);
   evas_object_smart_member_add(child, o);
   evas_object_smart_changed(o);

   return ret;
}

/* set to return any previous object set to the right position of the
 * smart object or NULL, if any (or on errors) */
Evas_Object *
evas_smart_example_set_right(Evas_Object *o,
                             Evas_Object *child)
{
   Evas_Object *ret = NULL;

   EVAS_SMART_EXAMPLE_DATA_GET_OR_RETURN_VAL(o, priv, NULL);
   if (!child)
     return NULL;

   if (priv->children[0] == child)
     {
        fprintf(stderr, "You mustn't place a child on both slots of"
                        " the example smart object!\n");
        return NULL;
     }

   if (priv->children[1])
     {
        if (priv->children[1] != child)
          ret = evas_smart_example_remove(o, priv->children[1]);
        else return child;
     }

   priv->children[1] = child;
   evas_object_smart_callback_call(o, SIG_CHILD_ADDED, child);

   _evas_smart_example_child_callbacks_register(o, child, 1);
   evas_object_smart_member_add(child, o);
   evas_object_smart_changed(o);

   return ret;
}

/* END OF example smart object's own interface */

static void
_on_keydown(void        *data __UNUSED__,
            Evas        *evas __UNUSED__,
            Evas_Object *o __UNUSED__,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->keyname, "h") == 0) /* print help */
     {
        fprintf(stdout, commands);
        return;
     }

   if (strcmp(ev->keyname, "l") == 0) /* insert random colored
                                       * rectangle on the left */
     {
        Evas_Object *rect = evas_object_rectangle_add(d.evas), *prev;
        evas_object_color_set(
          rect, rand() % 255, rand() % 255, rand() % 255, 255);
        evas_object_show(rect);

        prev = evas_smart_example_set_left(d.smt, rect);
        fprintf(stdout, "Setting smart object's left spot with a new"
                        " rectangle.\n");
        if (prev)
          {
             int r, g, b;

             evas_object_color_get(prev, &r, &g, &b, NULL);
             fprintf(stdout, "Deleting previous left child,"
                             " which had colors (%d, %d, %d)\n", r, g, b);
             evas_object_del(prev);
          }

        return;
     }

   if (strcmp(ev->keyname, "r") == 0) /* insert random colored
                                       * rectangle on the right */
     {
        Evas_Object *rect = evas_object_rectangle_add(d.evas), *prev;
        evas_object_color_set(
          rect, rand() % 255, rand() % 255, rand() % 255, 255);
        evas_object_show(rect);

        prev = evas_smart_example_set_right(d.smt, rect);
        fprintf(stdout, "Setting smart object's right spot with a new"
                        " rectangle.\n");
        if (prev)
          {
             int r, g, b;

             evas_object_color_get(prev, &r, &g, &b, NULL);
             fprintf(stdout, "Deleting previous right child,"
                             " which had colors (%d, %d, %d)\n", r, g, b);
             evas_object_del(prev);
          }

        return;
     }

   /* move smart object along the canvas */
   if (strcmp(ev->keyname, "Right") == 0 || strcmp(ev->keyname, "Left") == 0 ||
       strcmp(ev->keyname, "Up") == 0 || strcmp(ev->keyname, "Down") == 0)
     {
        Evas_Coord x, y;

        evas_object_geometry_get(d.smt, &x, &y, NULL, NULL);

        switch (ev->keyname[0])
          {
           case 'R':
             x += 20;
             break;

           case 'L':
             x -= 20;
             break;

           case 'U':
             y -= 20;
             break;

           case 'D':
             y += 20;
             break;
          }

        evas_object_move(d.smt, x, y);

        return;
     }

   /* increase smart object's size */
   if (strcmp(ev->keyname, "i") == 0)
     {
        Evas_Coord w, h;

        evas_object_geometry_get(d.smt, NULL, NULL, &w, &h);

        w *= 1.1;
        h *= 1.1;

        evas_object_resize(d.smt, w, h);

        return;
     }

   /* decrease smart object's size */
   if (strcmp(ev->keyname, "d") == 0)
     {
        Evas_Coord w, h;

        evas_object_geometry_get(d.smt, NULL, NULL, &w, &h);

        w *= 0.9;
        h *= 0.9;

        evas_object_resize(d.smt, w, h);

        return;
     }

   /* change smart object's clipper color */
   if (strcmp(ev->keyname, "c") == 0)
     {
        cur_color = (cur_color + 1) % 4;

        evas_object_color_set(
          d.clipper, clipper_colors[cur_color].r, clipper_colors[cur_color].g,
          clipper_colors[cur_color].b, clipper_colors[cur_color].a);

        fprintf (stderr, "Changing clipper's color to %s\n",
                 _index_to_color(cur_color));

        return;
     }
}

int
main(void)
{
   srand(time(NULL));

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 10, 10, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;

   ecore_evas_callback_destroy_set(d.ee, _on_destroy);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   /* the canvas pointer, de facto */
   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 255, 255, 255, 255);
   evas_object_move(d.bg, 0, 0);
   evas_object_resize(d.bg, WIDTH, HEIGHT);
   evas_object_show(d.bg);

   d.smt = evas_smart_example_add(d.evas);
   evas_object_move(d.smt, WIDTH / 4, HEIGHT / 4);
   evas_object_resize(d.smt, WIDTH / 2, HEIGHT / 2);
   evas_object_show(d.smt);

   d.clipper = evas_object_smart_clipped_clipper_get(d.smt);
   evas_object_color_set(
     d.clipper, clipper_colors[cur_color].r, clipper_colors[cur_color].g,
     clipper_colors[cur_color].b, clipper_colors[cur_color].a);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   fprintf(stdout, commands);
   ecore_main_loop_begin();

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "you got to have at least one evas engine built and linked"
                   " up to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}
