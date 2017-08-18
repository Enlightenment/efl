/**
 * Example of handling events for image objects in Evas.
 *
 * You'll need at least one engine built for it (excluding the buffer
 * one) and the png image loader/saver also built. See stdout/stderr
 * for output.
 *
 * @verbatim
 * gcc -o evas-images2 evas-images2.c `pkg-config --libs --cflags evas ecore ecore-evas`
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

static const char *img_path = PACKAGE_EXAMPLES_DIR EVAS_IMAGE_FOLDER "/enlightenment.png";
static const char *commands = \
  "commands are:\n"
  "\tp - change proxy image's source\n"
  "\ts - print noise image's stride value\n"
  "\ta - save noise image to disk (/tmp dir)\n"
  "\tv - change source visibility\n"
  "\te - enable/disable source events\n"
  "\th - print help\n";

const char *file_path = "/tmp/evas-images2-example.png";
const char *quality_str = "quality=100";

struct test_data
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *logo, *noise_img, *proxy_img, *text_obj, *bg;
};

static struct test_data d = {0};

static void
_mouse_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
            void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   printf("Mouse Down - obj(%p), coords(%d %d)\n",
          obj, ev->canvas.x, ev->canvas.y);
}

static void
_mouse_move(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
            void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   printf("Mouse Move - obj(%p), coords(%d %d)\n",
          obj, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_mouse_up(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
          void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   printf("Mouse Up - obj(%p), coords(%d %d)\n",
          obj, ev->canvas.x, ev->canvas.y);
}

static void
_multi_down(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
            void *event_info)
{
   Evas_Event_Multi_Down *ev = event_info;
   printf("Multi Down - obj(%p), coords(%d %d)\n",
          obj, ev->canvas.x, ev->canvas.y);
}

static void
_multi_move(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
             void *event_info)
{
   Evas_Event_Multi_Move *ev = event_info;
   printf("Multi Move - obj(%p), coords(%d %d)\n",
          obj, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_multi_up(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
          void *event_info)
{
   Evas_Event_Multi_Up *ev = event_info;
   printf("Multi Up - obj(%p), coords(%d %d)\n",
          obj, ev->canvas.x, ev->canvas.y);
}

static void
_mouse_in(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
          void *event_info)
{
   Evas_Event_Mouse_In *ev = event_info;
   printf("Mouse In - obj(%p), coords(%d %d)\n",
          obj, ev->canvas.x, ev->canvas.y);
}

static void
_mouse_out(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
           void *event_info)
{
   Evas_Event_Mouse_Out *ev = event_info;
   printf("Mouse Out - obj(%p), coords(%d %d)\n",
          obj, ev->canvas.x, ev->canvas.y);
}

static void
_hold(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
      void *event_info)
{
   Evas_Event_Hold *ev = event_info;
   printf("Hold - obj(%p), hold(%d)\n", obj, ev->hold);
}

static void
_on_preloaded(void        *data EINA_UNUSED,
              Evas        *e EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void        *event_info EINA_UNUSED)
{
    printf("Image has been pre-loaded!\n");
}

static void
_on_destroy(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

/* Keep the example's window size in sync with the background image's size */
static void
_canvas_resize_cb(Ecore_Evas *ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(d.bg, w, h);
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
        puts(commands);
        return;
     }

   if (strcmp(ev->key, "s") == 0) /* print proxy image' stride value */
     {
        int stride = evas_object_image_stride_get(d.noise_img);

        printf("Image has row stride value of %d, which accounts"
               " for %d pixels\n", stride, stride / 4);

        return;
     }

   if (strcmp(ev->key, "p") == 0) /* change proxy's source */
     {
        Evas_Object *source = evas_object_image_source_get(d.proxy_img);

        if (source == d.logo) source = d.noise_img;
        else if (source == d.noise_img) source = d.text_obj;
        else source = d.logo;

        evas_object_image_source_set(d.proxy_img, source);

        printf("Proxy image's source changed\n");

        return;
     }

   if (strcmp(ev->key, "a") == 0) /* save noise image to disk */
     {
        if (!evas_object_image_save(d.noise_img, file_path, NULL, quality_str))
          fprintf(stderr, "Cannot save image to '%s' (flags '%s')\n",
                  file_path, quality_str);
        else
          printf("Image saved to '%s' (flags '%s'), check it out with "
                 "an image viewer\n", file_path, quality_str);

        return;
     }

   if (strcmp(ev->key, "v") == 0) /* change source visibility */
     {
        Eina_Bool src_visible =
           evas_object_image_source_visible_get(d.proxy_img);
        evas_object_image_source_visible_set(d.proxy_img, !src_visible);
        return;
     }

   if (strcmp(ev->key, "e") == 0) /* change source events */
     {
        Eina_Bool src_events = evas_object_image_source_events_get(d.proxy_img);
        evas_object_image_source_events_set(d.proxy_img, !src_events);
        return;
     }
}

int
main(void)
{
   unsigned int i;
   unsigned int pixels[(WIDTH / 4) * (HEIGHT / 4)];
   Evas_Textblock_Style *st;

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
   evas_object_color_set(d.bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(d.bg, 0, 0); /* at canvas' origin */
   evas_object_resize(d.bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(d.bg);

   evas_object_focus_set(d.bg, EINA_TRUE);
   evas_object_event_callback_add(
     d.bg, EVAS_CALLBACK_KEY_DOWN, _on_keydown, NULL);

   d.logo = evas_object_image_filled_add(d.evas);

   evas_object_event_callback_add(
       d.logo, EVAS_CALLBACK_IMAGE_PRELOADED, _on_preloaded, NULL);
   evas_object_image_preload(d.logo, EINA_TRUE);

   evas_object_image_file_set(d.logo, img_path, NULL);
   evas_object_resize(d.logo, WIDTH / 2, HEIGHT / 2);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MOUSE_UP, _mouse_up, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MOUSE_IN, _mouse_in, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MOUSE_OUT, _mouse_out, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MULTI_DOWN, _multi_down, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MULTI_UP, _multi_up, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_MULTI_MOVE, _multi_move, 0);
   evas_object_event_callback_add(d.logo, EVAS_CALLBACK_HOLD, _hold, 0);

   evas_object_show(d.logo);

   /* creating noise image */
   for (i = 0; i < sizeof(pixels) / sizeof(pixels[0]); i++)
     pixels[i] = rand();

   d.noise_img = evas_object_image_add(d.evas);
   evas_object_image_size_set(d.noise_img, WIDTH / 4, HEIGHT / 4);
   evas_object_image_data_set(d.noise_img, pixels);
   evas_object_image_filled_set(d.noise_img, EINA_TRUE);
   evas_object_move(d.noise_img, (WIDTH * 5) / 8, HEIGHT / 8);
   evas_object_resize(d.noise_img, WIDTH / 4, HEIGHT / 4);
   evas_object_show(d.noise_img);
   printf("Creating noise image with size %d, %d\n",
          WIDTH / 4, HEIGHT / 4);

   d.text_obj = evas_object_textblock_add(d.evas);
   evas_object_textblock_text_markup_set(d.text_obj, "Hello world! :)");
   st = evas_textblock_style_new();
   evas_textblock_style_set(st, "DEFAULT='font=Sans font_size=16 color=#114 wrap=word'");
   evas_object_textblock_style_set(d.text_obj, st);
   evas_object_move(d.text_obj, WIDTH / 16, HEIGHT * 9 / 16);
   evas_object_resize(d.text_obj, WIDTH / 4, 0);
   evas_object_show(d.text_obj);

   d.proxy_img = evas_object_image_filled_add(d.evas);
   evas_object_image_source_set(d.proxy_img, d.logo);
   evas_object_move(d.proxy_img, WIDTH / 2, HEIGHT / 2);
   evas_object_resize(d.proxy_img, WIDTH / 2, HEIGHT / 2);
   evas_object_show(d.proxy_img);

   puts(commands);
   ecore_main_loop_begin();

   ecore_evas_free(d.ee);
   ecore_evas_shutdown();
   return 0;

error:
   fprintf(stderr, "error: Requires at least one Evas engine built and linked"
                   " to ecore-evas for this example to run properly.\n");
   ecore_evas_shutdown();
   return -1;
}
