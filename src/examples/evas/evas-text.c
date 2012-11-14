/**
 * Simple Evas example illustrating text objects
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one). See stdout/stderr for output.
 *
 * @verbatim
 * gcc -o evas-text evas-text.c `pkg-config --libs --cflags evas ecore ecore-evas`
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

#define GREY {190, 190, 190, 255}
#define BLACK {0, 0, 0, 255}
#define WHITE {255, 255, 255, 255}
#define RED   {255, 0, 0, 255}
#define GREEN {0, 255, 0, 255}
#define BLUE  {0, 0, 255, 255}

#define POINTER_CYCLE(_ptr, _array)                             \
  do                                                            \
    {                                                           \
       if ((unsigned)(((void *)(_ptr)) - ((void *)(_array))) >= \
           sizeof(_array))                                      \
         _ptr = _array;                                         \
    }                                                           \
  while(0)

static const char *commands = \
  "commands are:\n"
  "\tt - change text's current style\n"
  "\tz - change text's font size\n"
  "\tf - change text's font family\n"
  "\tb - change text's base color\n"
  "\ts - change text's \'shadow\' color\n"
  "\to - change text's \'outline\' color\n"
  "\tw - change text's \'glow\' color\n"
  "\tg - change text's \'glow 2\' color\n"
  "\th - print help\n";

static const char *border_img_path = PACKAGE_EXAMPLES_DIR "/red.png";

struct color_tuple
{
   int r, g, b, a;
};

struct text_preset_data
{
   const char        **font_ptr;
   const char         *font[3];

   struct color_tuple *text_ptr;
   struct color_tuple  text[6];

   struct color_tuple *shadow_ptr;
   struct color_tuple  shadow[4];

   struct color_tuple *outline_ptr;
   struct color_tuple  outline[4];

   struct color_tuple *glow_ptr;
   struct color_tuple  glow[4];

   struct color_tuple *glow2_ptr;
   struct color_tuple  glow2[4];
};

struct test_data
{
   Ecore_Evas             *ee;
   Evas                   *evas;
   struct text_preset_data t_data;
   Evas_Object            *text, *bg, *border;
};

static struct test_data d = {0};

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

static const char *
_text_style_type_to_str(Evas_Text_Style_Type mode)
{
   switch (mode)
     {
      case EVAS_TEXT_STYLE_PLAIN:
        return "plain";

      case EVAS_TEXT_STYLE_SHADOW:
        return "shadow";

      case EVAS_TEXT_STYLE_OUTLINE:
        return "outline";

      case EVAS_TEXT_STYLE_SOFT_OUTLINE:
        return "soft outline";

      case EVAS_TEXT_STYLE_GLOW:
        return "glow";

      case EVAS_TEXT_STYLE_OUTLINE_SHADOW:
        return "outline shadow";

      case EVAS_TEXT_STYLE_FAR_SHADOW:
        return "far shadow";

      case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
        return "outline soft shadow";

      case EVAS_TEXT_STYLE_SOFT_SHADOW:
        return "soft shadow";

      case EVAS_TEXT_STYLE_FAR_SOFT_SHADOW:
        return "far soft shadow";

      default:
        return "invalid";
     }
}

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

   if (strcmp(ev->keyname, "t") == 0) /* change text's current style */
     {
        Evas_Text_Style_Type type = evas_object_text_style_get(d.text);

        type = (type + 1) % 10;

        evas_object_text_style_set(d.text, type);

        fprintf(stdout, "Changing text's style to \'%s\'\n",
                _text_style_type_to_str(type));

        return;
     }

   if (strcmp(ev->keyname, "f") == 0) /* change text's font */
     {
         int sz;

        (d.t_data.font_ptr)++;

        evas_object_text_font_get(d.text, NULL, &sz);

        POINTER_CYCLE(d.t_data.font_ptr, d.t_data.font);

        evas_object_text_font_set(d.text, *d.t_data.font_ptr, sz);

        fprintf(stdout, "Changing text's font to %s\n", *d.t_data.font_ptr);

        return;
     }

   if (strcmp(ev->keyname, "b") == 0) /* change text's base color */
     {
        (d.t_data.text_ptr)++;

        POINTER_CYCLE(d.t_data.text_ptr, d.t_data.text);

        evas_object_color_set(
          d.text, d.t_data.text_ptr->r, d.t_data.text_ptr->g,
          d.t_data.text_ptr->b, d.t_data.text_ptr->a);

        fprintf(stdout, "Changing base color for text to (%d, %d, %d, %d)\n",
                d.t_data.text_ptr->r, d.t_data.text_ptr->g,
                d.t_data.text_ptr->b, d.t_data.text_ptr->a);

        return;
     }

   if (strcmp(ev->keyname, "g") == 0) /* change text's glow 2 color */
     {
        (d.t_data.glow2_ptr)++;

        POINTER_CYCLE(d.t_data.glow2_ptr, d.t_data.glow2);

        evas_object_text_glow2_color_set(
          d.text, d.t_data.glow2_ptr->r, d.t_data.glow2_ptr->g,
          d.t_data.glow2_ptr->b, d.t_data.glow2_ptr->a);

        fprintf(stdout, "Changing glow 2 color for text to (%d, %d, %d, %d)\n",
                d.t_data.glow2_ptr->r, d.t_data.glow2_ptr->g,
                d.t_data.glow2_ptr->b, d.t_data.glow2_ptr->a);

        return;
     }

   if (strcmp(ev->keyname, "w") == 0) /* change text's glow color */
     {
        (d.t_data.glow_ptr)++;

        POINTER_CYCLE(d.t_data.glow_ptr, d.t_data.glow);

        evas_object_text_glow_color_set(
          d.text, d.t_data.glow_ptr->r, d.t_data.glow_ptr->g,
          d.t_data.glow_ptr->b, d.t_data.glow_ptr->a);

        fprintf(stdout, "Changing glow color for text to (%d, %d, %d, %d)\n",
                d.t_data.glow_ptr->r, d.t_data.glow_ptr->g,
                d.t_data.glow_ptr->b, d.t_data.glow_ptr->a);

        return;
     }

   if (strcmp(ev->keyname, "o") == 0) /* change text's outline color */
     {
        (d.t_data.outline_ptr)++;

        POINTER_CYCLE(d.t_data.outline_ptr, d.t_data.outline);

        evas_object_text_outline_color_set(
          d.text, d.t_data.outline_ptr->r, d.t_data.outline_ptr->g,
          d.t_data.outline_ptr->b, d.t_data.outline_ptr->a);

        fprintf(stdout, "Changing outline color for text to (%d, %d, %d, %d)\n",
                d.t_data.outline_ptr->r, d.t_data.outline_ptr->g,
                d.t_data.outline_ptr->b, d.t_data.outline_ptr->a);

        return;
     }

   if (strcmp(ev->keyname, "s") == 0) /* change text's shadow color */
     {
        (d.t_data.shadow_ptr)++;

        POINTER_CYCLE(d.t_data.shadow_ptr, d.t_data.shadow);

        evas_object_text_shadow_color_set(
          d.text, d.t_data.shadow_ptr->r, d.t_data.shadow_ptr->g,
          d.t_data.shadow_ptr->b, d.t_data.shadow_ptr->a);

        fprintf(stdout, "Changing shadow color for text to (%d, %d, %d, %d)\n",
                d.t_data.shadow_ptr->r, d.t_data.shadow_ptr->g,
                d.t_data.shadow_ptr->b, d.t_data.shadow_ptr->a);

        return;
     }

   if (strcmp(ev->keyname, "z") == 0) /* change text's font size */
     {
        const char *font;
        int size;

        evas_object_text_font_get(d.text, &font, &size);

        size = (size + 10) % 50;
        if (!size) size = 10;

        evas_object_text_font_set(d.text, font, size);

        fprintf(stdout, "Changing text's font size to %d\n", size);

        return;
     }
}

int
main(void)
{
   int size;
   const char *font;

   if (!ecore_evas_init())
     return EXIT_FAILURE;

   /* init values one is going to cycle through while running this
    * example */
   struct text_preset_data init_data =
   {
      .font = {"DejaVu", "Courier", "Utopia"},
      .text = {BLACK, WHITE, GREY, RED, GREEN, BLUE},
      .shadow = {WHITE, BLUE, GREEN, RED},
      .outline = {WHITE, RED, GREEN, BLUE},
      .glow = {WHITE, BLUE, GREEN, RED},
      .glow2 = {WHITE, RED, BLUE, GREEN}
   };

   d.t_data = init_data;
   d.t_data.font_ptr = d.t_data.font;
   d.t_data.text_ptr = d.t_data.text;
   d.t_data.glow_ptr = d.t_data.glow;
   d.t_data.glow2_ptr = d.t_data.glow2;
   d.t_data.outline_ptr = d.t_data.outline;
   d.t_data.shadow_ptr = d.t_data.shadow;

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
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.text = evas_object_text_add(d.evas);
   evas_object_text_style_set(d.text, EVAS_TEXT_STYLE_PLAIN);

   /* let the pre-set thingies be enforced */
   evas_object_color_set(
     d.text, d.t_data.text_ptr->r, d.t_data.text_ptr->g,
     d.t_data.text_ptr->b, d.t_data.text_ptr->a);

   evas_object_text_glow_color_set(
     d.text, d.t_data.glow_ptr->r, d.t_data.glow_ptr->g,
     d.t_data.glow_ptr->b, d.t_data.glow_ptr->a);

   evas_object_text_glow2_color_set(
     d.text, d.t_data.glow2_ptr->r, d.t_data.glow2_ptr->g,
     d.t_data.glow2_ptr->b, d.t_data.glow2_ptr->a);

   evas_object_text_outline_color_set(
     d.text, d.t_data.outline_ptr->r, d.t_data.outline_ptr->g,
     d.t_data.outline_ptr->b, d.t_data.outline_ptr->a);

   evas_object_text_shadow_color_set(
     d.text, d.t_data.shadow_ptr->r, d.t_data.shadow_ptr->g,
     d.t_data.shadow_ptr->b, d.t_data.shadow_ptr->a);

   evas_object_text_font_set(d.text, *d.t_data.font_ptr, 30);
   evas_object_text_text_set(d.text, "sample text");

   evas_object_resize(d.text, (3 * WIDTH) / 4, HEIGHT / 4);
   evas_object_move(d.text, WIDTH / 8, (3 * HEIGHT) / 8);
   evas_object_show(d.text);

   evas_object_text_font_get(d.text, &font, &size);
   fprintf(stdout, "Adding text object with font %s, size %d\n", font, size);

   /* this is a border around the text object above, here just to
    * emphasize its geometry */
   d.border = evas_object_image_filled_add(d.evas);
   evas_object_image_file_set(d.border, border_img_path, NULL);
   evas_object_image_border_set(d.border, 3, 3, 3, 3);
   evas_object_image_border_center_fill_set(d.border, EVAS_BORDER_FILL_NONE);

   evas_object_resize(d.border, ((3 * WIDTH) / 4) + 3, (HEIGHT / 4) + 3);
   evas_object_move(d.border, (WIDTH / 8) - 3, ((3 * HEIGHT) / 8) - 3);
   evas_object_show(d.border);

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

