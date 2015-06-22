#include <stdio.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Ecore_Buffer.h>
#include <Ecore_Buffer_Queue.h>

#define WIDTH 720
#define HEIGHT 960

struct _Window
{
   Evas *e;
   Ecore_Evas *ee;
   Evas_Object *bg, *img;
   Ecore_Buffer *buffer;
};

struct _Window win;
Eina_List *hdls;

static void
paint_pixels(void *image, int padding, int width, int height, uint32_t time)
{
   const int halfh = padding + (height - padding * 2) / 2;
   const int halfw = padding + (width  - padding * 2) / 2;
   int ir, or;
   uint32_t *pixel = image;
   int y;

   /* squared radii thresholds */
   or = (halfw < halfh ? halfw : halfh) - 8;
   ir = or - 32;
   or *= or;
   ir *= ir;

   pixel += padding * width;
   for (y = padding; y < height - padding; y++) {
        int x;
        int y2 = (y - halfh) * (y - halfh);

        pixel += padding;
        for (x = padding; x < width - padding; x++) {
             uint32_t v;

             /* squared distance from center */
             int r2 = (x - halfw) * (x - halfw) + y2;

             if (r2 < ir)
               v = (r2 / 32 + time / 64) * 0x0080401;
             else if (r2 < or)
               v = (y + time / 32) * 0x0080401;
             else
               v = (x + time / 16) * 0x0080401;
             v &= 0x00ffffff;
             v |= 0xff000000;

             *pixel++ = v;
        }

        pixel += padding;
   }
}

static void
_cb_post_render(Ecore_Evas *ee EINA_UNUSED)
{
   void *data;

   // Get pixel data and update.
   data = ecore_buffer_data_get(win.buffer);
   paint_pixels(data, 0, WIDTH, HEIGHT, ecore_loop_time_get() * 1000);
   evas_object_image_data_set(win.img, data);
   evas_object_image_data_update_add(win.img, 0, 0, WIDTH, HEIGHT);
}

int
main(void)
{
   Evas_Object *o;
   void *data;

   eina_init();
   ecore_init();
   ecore_evas_init();
   ecore_buffer_init();

   win.ee = ecore_evas_new(NULL, 0, 0, WIDTH, HEIGHT, NULL);
   win.e = ecore_evas_get(win.ee);

   o = evas_object_rectangle_add(win.e);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, WIDTH, HEIGHT);
   evas_object_color_set(o, 255, 0, 0, 255);
   evas_object_show(o);
   win.bg = o;

   o = evas_object_image_add(win.e);
   evas_object_image_fill_set(o, 0, 0, WIDTH, HEIGHT);
   evas_object_image_size_set(o, WIDTH, HEIGHT);

   evas_object_move(o, 0, 0);
   evas_object_resize(o, WIDTH, HEIGHT);
   evas_object_show(o);
   win.img = o;

   // Create buffer and drawing.
   win.buffer = ecore_buffer_new("shm", WIDTH, HEIGHT, 0, 0);
   data = ecore_buffer_data_get(win.buffer);
   paint_pixels(data, 0, WIDTH, HEIGHT, 0);
   evas_object_image_data_set(win.img, data);
   evas_object_image_data_update_add(win.img, 0, 0, WIDTH, HEIGHT);

   ecore_evas_show(win.ee);

   ecore_evas_callback_post_render_set(win.ee, _cb_post_render);

   ecore_main_loop_begin();

   ecore_buffer_free(win.buffer);
   ecore_buffer_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return 0;
}
