/**
 * Simple Edje example illustrating color class functions.
 *
 * You'll need at least one Evas engine built for it (excluding the
 * buffer one). See stdout/stderr for output.
 *
 * @verbatim
 * edje_cc color-class.edc && gcc -o edje-color-class edje-color-class.c `pkg-config --libs --cflags eina evas ecore ecore-evas edje`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define EINA_UNUSED
#endif

#ifndef PACKAGE_DATA_DIR
#define PACKAGE_DATA_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define WIDTH  (400)
#define HEIGHT (400)

typedef int color[4];           /* rgba */

static Ecore_Evas *ee1, *ee2;
static Evas *evas1, *evas2;
static Evas_Object *bg1, *edje_obj1, *bg2, *edje_obj2;
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

   printf("Getting the color classes\n\n");
   classes = edje_color_class_list();
   EINA_LIST_FREE(classes, class_name)
     {
        int r1, r2, r3, g1, g2, g3, b1, b2, b3,
          a1, a2, a3;

        printf("\ncolor class: %s\n", class_name);
        if (!edje_color_class_get(class_name, &r1, &g1, &b1, &a1,
                                  &r2, &g2, &b2, &a2, &r3, &g3, &b3, &a3))
          fprintf(stderr, "Cannot get the color class\n");
        else
          {

             printf("Object color r: %d g: %d b: %d a: %d\n",
                     r1, g1, b1, a1);
             printf("Text outline color r: %d g: %d b: %d a: %d\n",
                     r2, g2, b2, a2);
             printf("Text shadow color r: %d g: %d b: %d a: %d\n",
                     r3, g3, b3, a3);
          }
        free(class_name);
     }
}

static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_on_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->button == 1)
     {
        if (obj == edje_obj1)
          edje_color_class_del(selected_class);
        else
          edje_object_color_class_del(edje_obj2, selected_class);
     }
}

/* here just to keep our example's window size
 * in synchrony. */
static void
_canvas_resize_cb(Ecore_Evas *_ee)
{
   int w, h;

   ecore_evas_geometry_get(_ee, NULL, NULL, &w, &h);

   if (_ee == ee1)
     {
        evas_object_resize(bg1, w, h);
        evas_object_resize(edje_obj1, w, h);
     }
   else
     {
        evas_object_resize(bg2, w, h);
        evas_object_resize(edje_obj2, w, h);
     }
}

static void
_color_class_callback_delete(void *data, Evas_Object *obj EINA_UNUSED,
                             const char *emission, void *source EINA_UNUSED)
{
   if (!strcmp(data, "process"))
     printf("Color class: %s deleted on process level\n", emission);
   else
     printf("Color class: %s deleted on object level\n", emission);
}

static int
_create_windows(const char *edje_file_path)
{
  /* this will give you a window with an Evas canvas under the first
    * engine available */
   ee1 = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee1)
     return 0;
   ee2 = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!ee2)
     return 0;

   ecore_evas_callback_destroy_set(ee1, _on_destroy);
   ecore_evas_callback_resize_set(ee1, _canvas_resize_cb);
   ecore_evas_title_set(ee1, "Edje Color Class Example");

   ecore_evas_callback_destroy_set(ee2, _on_destroy);
   ecore_evas_callback_resize_set(ee2, _canvas_resize_cb);
   ecore_evas_title_set(ee2, "Edje Object Color Class Example");

   evas1 = ecore_evas_get(ee1);
   evas2 = ecore_evas_get(ee2);

   bg1 = evas_object_rectangle_add(evas1);
   evas_object_color_set(bg1, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg1, 0, 0);                    /* at canvas' origin */
   evas_object_resize(bg1, WIDTH, HEIGHT);         /* covers full canvas */
   evas_object_show(bg1);

   bg2 = evas_object_rectangle_add(evas2);
   evas_object_color_set(bg2, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg2, 0, 0);                    /* at canvas' origin */
   evas_object_resize(bg2, WIDTH, HEIGHT);         /* covers full canvas */
   evas_object_show(bg2);

   edje_obj1 = edje_object_add(evas1);
   evas_object_event_callback_add(edje_obj1, EVAS_CALLBACK_MOUSE_DOWN,
                                  _on_mouse_down, NULL);

   edje_object_file_set(edje_obj1, edje_file_path, "example_color_class");
   evas_object_move(edje_obj1, 0, 0); /* at canvas' origin */
   evas_object_resize(edje_obj1, WIDTH, HEIGHT);
   edje_object_part_text_set(edje_obj1, "part_four", "EDJE EXAMPLE");
   edje_object_signal_callback_add(edje_obj1, "color_class,del", "*",
                                   (Edje_Signal_Cb) _color_class_callback_delete,
                                   "process");
   evas_object_show(edje_obj1);

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

   return 1;
}

int
main(int argc, char *argv[])
{
   const char  *edje_file = PACKAGE_DATA_DIR"/color-class.edj";
   color        c1, c2, c3;
   int          i;

   if (argc != 5)
     {
        fprintf(stderr, "You have to use: %s color_class_name color1, color2," \
                "color3\n", argv[0]);
        fprintf(stderr, "Available colors:\n");
        for (i = 0; i < 8; i++)
          fprintf(stderr, "%s\n", color_names[i]);

        return EXIT_FAILURE;
     }

   selected_class = argv[1];
   if (!(_get_color_from_name(argv[2], &c1) &&
         _get_color_from_name(argv[3], &c2) &&
         _get_color_from_name(argv[4], &c3)))
     {
        fprintf(stderr, "Color not available!\n");
        return EXIT_FAILURE;
     }

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   if (!edje_init())
     goto shutdown_ecore_evas;

   if (!_create_windows(edje_file)) goto shutdown_edje;

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

   ecore_evas_show(ee1);
   ecore_evas_show(ee2);

   ecore_main_loop_begin();

   ecore_evas_free(ee1);
   ecore_evas_shutdown();
   edje_shutdown();

   return EXIT_SUCCESS;

 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return EXIT_FAILURE;
}
