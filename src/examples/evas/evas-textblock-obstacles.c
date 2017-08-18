/**
 * Example of textblock obstacles in Evas.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
 *
 * You start with two registered obstacle objects. They are not visible
 * at first, so the textblock simply shows the text that has been set to it.
 * Once the obstacle is visible (show/hide keys in the example), the text will
 * wrap around it.
 * This example allows you to test two obstacles registered to the same
 * textblock object. Also, you can play with size and position for each.
 * Use the 'h' key to show the provided options for this test.
 *
 * @verbatim
 * gcc -o evas-textblock-obstacles evas-textblock-obstacles.c `pkg-config --libs --cflags evas ecore ecore-evas`
 * @endverbatim
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define PACKAGE_EXAMPLES_DIR "."
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <errno.h>
#include "evas-common.h"

#define WIDTH  (320)
#define HEIGHT (240)

#define POINTER_CYCLE(_ptr, _array)                             \
  do                                                            \
    {                                                           \
       if ((unsigned int)(((unsigned char *)(_ptr)) - ((unsigned char *)(_array))) >= \
           sizeof(_array))                                      \
         _ptr = _array;                                         \
    }                                                           \
  while(0)

static const char *commands = \
  "commands are:\n"
  "\tt - change currently controlled obstacle\n"
  "\tv - show/hide current obstacle\n"
  "\ts - cycle current obstacle's size\n"
  "\tp - change current obstacle's position (random)\n"
  "\tw - cycle text wrapping modes (none/word/char/mixed)\n"
  "\th - print help\n";

struct text_preset_data
{
   const char        **font_ptr;
   const char         *font[3];

   const char        **wrap_ptr;
   const char         *wrap[4];

   Evas_Coord         *obs_size_ptr;
   Evas_Coord          obs_size[3];

   Evas_Object **obs_ptr; /* pointer to the currently controlled obstacle object */
   Evas_Object *obs[2];
};

struct test_data
{
   Ecore_Evas             *ee;
   Evas                   *evas;
   struct text_preset_data t_data;
   Evas_Object            *text, *bg;
   Evas_Coord             w, h;
   Evas_Textblock_Style   *st;
};

static struct test_data d = {0};

static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
   evas_object_resize(d.text, w, h);
   d.w = w;
   d.h = h;
}

static unsigned int
_getrand(unsigned int low, unsigned int high)
{
   return (rand() % (high - low)) + low;
}

static void
_style_set(const char *wrap)
{
   char buf[2000];
   snprintf(buf,
         2000,
         "DEFAULT='font=Sans font_size=16 color=#000 wrap=%s text_class=entry'"
         "br='\n'"
         "ps='ps'"
         "tab='\t'",
         wrap);
   evas_textblock_style_set(d.st, buf);
}

static void
_on_keydown(void        *data EINA_UNUSED,
            Evas        *evas EINA_UNUSED,
            Evas_Object *o EINA_UNUSED,
            void        *einfo)
{
   Evas_Event_Key_Down *ev = einfo;

   if (strcmp(ev->key, "h") == 0) /* print help */
     {
        printf("%s\n", commands);
        return;
     }

   if (strcmp(ev->key, "t") == 0) /* change obstacle type */
     {
        (d.t_data.obs_ptr)++;
        POINTER_CYCLE(d.t_data.obs_ptr, d.t_data.obs);

        printf("Now controlling obstacle: %p\n", *d.t_data.obs_ptr);

        return;
     }

   if (strcmp(ev->key, "v") == 0) /* change obstacle visibility */
     {
        Eo *obj = *d.t_data.obs_ptr;
        if (evas_object_visible_get(obj))
           evas_object_hide(obj);
        else
           evas_object_show(obj);
        printf("Show/hide toggle for obstacle %p\n",
               *d.t_data.obs_ptr);
        evas_object_textblock_obstacles_update(d.text);

        return;
     }

   if (strcmp(ev->key, "s") == 0) /* change obstacle size */
     {
        (d.t_data.obs_size_ptr)++;
        POINTER_CYCLE(d.t_data.obs_size_ptr, d.t_data.obs_size);

        evas_object_resize(*d.t_data.obs_ptr,
              *d.t_data.obs_size_ptr,
              *d.t_data.obs_size_ptr);

        evas_object_textblock_obstacles_update(d.text);

        printf("Changing obstacle size to: %d,%d\n",
               *d.t_data.obs_size_ptr, *d.t_data.obs_size_ptr);

        return;
     }

   if (strcmp(ev->key, "p") == 0) /* change obstacle position */
     {
        Evas_Coord x, y;
        Evas_Coord rx, ry, gx, gy;
        x = _getrand(0, d.w);
        y = _getrand(0, d.h);
        evas_object_move(*d.t_data.obs_ptr, x, y);
        evas_object_textblock_obstacles_update(d.text);

        printf("Changing obstacles position\n");
        evas_object_move(*d.t_data.obs_ptr, x, y);
        evas_object_geometry_get(d.t_data.obs[0], &rx, &ry, NULL, NULL);
        evas_object_geometry_get(d.t_data.obs[1], &gx, &gy, NULL, NULL);
        printf("Obstacle #1 (red)  : [%d,%d]\n", rx, ry);
        printf("Obstacle #2 (green): [%d,%d]\n", gx, gy);

        return;
     }

   if (strcmp(ev->key, "w") == 0) /* change obstacle position */
     {
        (d.t_data.wrap_ptr)++;
        POINTER_CYCLE(d.t_data.wrap_ptr, d.t_data.wrap);
        printf("Changing wrap mode to: %s\n",
               *d.t_data.wrap_ptr);
        _style_set(*d.t_data.wrap_ptr);
        evas_object_textblock_obstacles_update(d.text);

        return;
     }
}

static void
_obs_init(Evas_Object *obj)
{
   evas_object_resize(obj, 50, 50);
}

static void
_text_init()
{
   d.st = evas_textblock_style_new();
   evas_object_textblock_style_set(d.text, d.st);
   _style_set("word");

   evas_object_textblock_text_markup_set(d.text,
         "This example text demonstrates the textblock object"
         " with obstacle objects support."
         " Any evas object <item size=72x16></item>can register itself as an obstacle to the textblock"
         " object. Upon reg<color=#0ff>stering, it aff</color>ects the layout of the text in"
         " certain situations. Usually, when the obstacle shows above the text"
         " area, it will cause the layout of the text to split and move"
         " parts of it, so that all text area is apparent."
         );
}

int
main(void)
{
   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* example obstacles types */
   Evas_Object *rect, *rect2;

   /* init values one is going to cycle through while running this
    * example */
   struct text_preset_data init_data =
   {
      .font = {"DejaVu", "Courier", "Utopia"},
      .wrap = {"word", "char", "mixed", "none"},
      .obs_size = {50, 70, 100},
      .obs = {NULL, NULL},
   };

   d.t_data = init_data;
   d.t_data.font_ptr = d.t_data.font;
   d.t_data.obs_size_ptr = d.t_data.obs_size;
   d.t_data.obs_ptr = d.t_data.obs;

   /* this will give you a window with an Evas canvas under the first
    * engine available */
   d.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   if (!d.ee)
     goto error;
   printf("Window size set to [%d,%d]\n", WIDTH, HEIGHT);

   ecore_evas_callback_delete_request_set(d.ee, _on_destroy);
   ecore_evas_callback_resize_set(d.ee, _canvas_resize_cb);
   ecore_evas_show(d.ee);

   d.evas = ecore_evas_get(d.ee);

   d.bg = evas_object_rectangle_add(d.evas);
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.text = evas_object_textblock_add(d.evas);
   _text_init();
   evas_object_resize(d.text, WIDTH, HEIGHT);
   evas_object_move(d.text, 0, 0);
   evas_object_show(d.text);
   d.w = WIDTH;
   d.h = HEIGHT;

   /* init obstacles */
   rect = evas_object_rectangle_add(d.evas);
   d.t_data.obs[0] = rect;
   evas_object_color_set(rect, 255, 0, 0, 255);
   _obs_init(rect);
   rect2 = evas_object_rectangle_add(d.evas);
   d.t_data.obs[1] = rect2;
   evas_object_color_set(rect2, 0, 255, 0, 255);
   _obs_init(rect2);

   evas_object_textblock_obstacle_add(d.text, rect);
   evas_object_textblock_obstacle_add(d.text, rect2);

   evas_object_show(d.t_data.obs[0]);
   evas_object_show(d.t_data.obs[1]);

   printf("%s\n", commands);
   ecore_main_loop_begin();

   evas_textblock_style_free(d.st);
   ecore_evas_free(d.ee);
   ecore_evas_shutdown();

   return 0;

error:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}
