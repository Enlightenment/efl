/**
 * Simple Edje example illustrating color class functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc color-class.edc && gcc -o edje-table edje-color-class.c `pkg-config --libs --cflags evas ecore ecore-evas edje`
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

#define WIDTH  (400)
#define HEIGHT (400)

static const char *edje_file_path = PACKAGE_EXAMPLES_DIR "/color-class.edj";

typedef int color[4];           /* rgba */

static Ecore_Evas *ee, *ee2;
static Evas *evas, *evas2;
static Evas_Object *bg, *edje_obj, *bg2, *edje_obj2;
static const char *selected_class;

static color colors_init_data[] =
  {{255, 0, 0, 255},            /* red */
   {0, 255, 0, 255},            /* green */
   {0, 0, 255, 255},            /* blue */
   {0, 0, 0, 255},              /* black */
   {255, 255, 255, 255},        /* white */
   {128, 128, 128, 255},        /* gray */
   {255, 255, 0, 255},          /* yellow */
   {255, 0, 255, 255}           /* pink */
  };

static char *color_names[] =
  {"red", "green", "blue", "black", "white",
   "gray", "yellow", "pink"};

static Eina_Bool
_get_color_from_name(const char *n, color *c)
{
   int i;
   for (i = 0; i < 8; i++)
     if (!strcmp(n, color_names[i]))
       {
          (*c)[0] = (colors_init_data[i])[0];
          (*c)[1] = (colors_init_data[i])[1];
          (*c)[2] = (colors_init_data[i])[2];
          (*c)[3] = (colors_init_data[i])[3];
          return EINA_TRUE;
       }

   return EINA_FALSE;
}

static void
_color_classes_print(void)
{
   Eina_List *classes;
   char *class_name;

   fprintf(stdout, "Getting the color classes\n\n");
   classes = edje_color_class_list();
   EINA_LIST_FREE(classes, class_name)
     {
        int r1, r2, r3, g1, g2, g3, b1, b2, b3,
          a1, a2, a3;

        fprintf(stdout, "\ncolor class: %s\n", class_name);
        if (!edje_color_class_get(class_name, &r1, &g1, &b1, &a1,
                                  &r2, &g2, &b2, &a2, &r3, &g3, &b3, &a3))
          fprintf(stderr, "Cannot get the color class\n");
        else
          {

             fprintf(stdout,"Object color r: %d g: %d b: %d a: %d\n",
                     r1, g1, b1, a1);
             fprintf(stdout,"Text outline color r: %d g: %d b: %d a: %d\n",
                     r2, g2, b2, a2);
             fprintf(stdout,"Text shadow color r: %d g: %d b: %d a: %d\n",
                     r3, g3, b3, a3);
          }
        free(class_name);
     }
}

static void
_on_destroy(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

static void
_on_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button == 1)
     if (obj == edje_obj)
       edje_color_class_del(selected_class);
     else
       edje_object_color_class_del(edje_obj2, selected_class);
}

/* here just to keep our example's window size
 * in synchrony. */
static void
_canvas_resize_cb(Ecore_Evas *_ee)
{
   int w, h;

   ecore_evas_geometry_get(_ee, NULL, NULL, &w, &h);

   if (_ee == ee)
     {
        evas_object_resize(bg, w, h);
        evas_object_resize(edje_obj, w, h);
     }
   else
     {
        evas_object_resize(bg2, w, h);
        evas_object_resize(edje_obj2, w, h);
     }
}

static void
_color_class_callback_delete(void *data, Evas *evas, Evas_Object *obj,
                             const char *emission, void *source)
{
   if (!strcmp(data, "process"))
     fprintf(stdout, "Color class: %s deleted on process level\n", emission);
   else
     fprintf(stdout, "Color class: %s deleted on object level\n", emission);
}

static void
_create_windows(void)
{
  /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   ee2 = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);

   ecore_evas_callback_destroy_set(ee, _on_destroy);
   ecore_evas_callback_resize_set(ee, _canvas_resize_cb);
   ecore_evas_title_set(ee, "Edje Color Class Example");
   ecore_evas_show(ee);

   ecore_evas_callback_destroy_set(ee2, _on_destroy);
   ecore_evas_callback_resize_set(ee2, _canvas_resize_cb);
   ecore_evas_title_set(ee2, "Edje Object Color Class Example");
   ecore_evas_show(ee2);

   evas = ecore_evas_get(ee);
   evas2 = ecore_evas_get(ee2);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0);                    /* at canvas' origin */
   evas_object_resize(bg, WIDTH, HEIGHT);         /* covers full canvas */
   evas_object_show(bg);

   bg2 = evas_object_rectangle_add(evas2);
   evas_object_color_set(bg2, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg2, 0, 0);                    /* at canvas' origin */
   evas_object_resize(bg2, WIDTH, HEIGHT);         /* covers full canvas */
   evas_object_show(bg2);

   edje_obj = edje_object_add(evas);
   evas_object_event_callback_add(edje_obj, EVAS_CALLBACK_MOUSE_DOWN,
                                  _on_mouse_down, NULL);

   edje_object_file_set(edje_obj, edje_file_path, "example_color_class");
   evas_object_move(edje_obj, 0, 0); /* at canvas' origin */
   evas_object_resize(edje_obj, WIDTH, HEIGHT);
   edje_object_part_text_set(edje_obj, "part_four", "EDJE EXAMPLE");
   edje_object_signal_callback_add(edje_obj, "color_class,del", "*",
                                   (Edje_Signal_Cb) _color_class_callback_delete,
                                   "process");
   evas_object_show(edje_obj);

   edje_obj2 = edje_object_add(evas2);
   evas_object_event_callback_add(edje_obj2, EVAS_CALLBACK_MOUSE_DOWN,
                                  _on_mouse_down, NULL);

   edje_object_file_set(edje_obj2, edje_file_path, "example_color_class");
   evas_object_move(edje_obj2, 0, 0); /* at canvas' origin */
   evas_object_resize(edje_obj2, WIDTH, HEIGHT);
   edje_object_part_text_set(edje_obj2, "part_four", "EDJE OBJECT EXAMPLE");
   edje_object_signal_callback_add(edje_obj2, "color_class,del", "*",
                                   (Edje_Signal_Cb) _color_class_callback_delete,
                                   "object");
   evas_object_show(edje_obj2);
}

int
main(int argc, char *argv[])
{
   color c1, c2, c3;
   int i;

   if (argc != 5)
     {
        fprintf(stderr, "You have to use: %s color_class_name color1, color2," \
                "color3\n", argv[0]);
        fprintf(stderr, "Available colors:\n");
        for (i = 0; i < 8; i++)
          fprintf(stderr, "%s\n", color_names[i]);

        return 1;
     }

   selected_class = argv[1];
   if (!(_get_color_from_name(argv[2], &c1) &&
         _get_color_from_name(argv[3], &c2) &&
         _get_color_from_name(argv[4], &c3)))
     {
        fprintf(stderr, "Color not available!\n");
        return 2;
     }

   ecore_evas_init();
   edje_init();

   _create_windows();

   edje_color_class_set(argv[1],                     /* class name   */
                        c1[0], c1[1], c1[2], c1[3],  /* Object color */
                        c2[0], c2[1], c2[2], c2[3],  /* Text outline */
                        c3[0], c3[1], c3[2], c3[3]); /* Text shadow  */

   /* Setting an arbitrary value just to see the difference between */
   /* process level and object level */
   edje_object_color_class_set(edje_obj2, argv[1], /* class name   */
                               128, 180, 77, 255,  /* Object color */
                               200, 22, 86, 255,   /* Text outline */
                               39, 90, 187, 255);  /* Text shadow  */

   _color_classes_print();
   ecore_main_loop_begin();
   ecore_evas_free(ee);
   ecore_evas_free(ee2);
   ecore_evas_shutdown();
   edje_shutdown();
   return 0;
}
