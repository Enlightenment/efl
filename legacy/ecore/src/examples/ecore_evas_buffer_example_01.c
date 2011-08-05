/**
 * Simple Ecore_Evas example on the Evas buffer engine wrapper
 * functions.
 *
 * You must have Evas compiled with the buffer engine.
 *
 * Compile with:
 *
 * @verbatim
 * gcc -o evas-buffer-simple evas-buffer-simple.c `pkg-config --libs --cflags evas evas-software-buffer`
 * @endverbatim
 *
 */

#ifdef HAVE_CONFIG_H

#include "config.h"
#else
#define __UNUSED__
#endif

#include <Ecore_Evas.h>
#include <stdio.h>

#define WIDTH  (320)
#define HEIGHT (240)

static Ecore_Evas *ee;

/* support function to save scene as PPM image */
static void
_scene_save(Evas       *canvas,
            const char *dest)
{
   const unsigned int *pixels, *pixels_end;
   int width, height;
   FILE *f;

   evas_output_size_get(canvas, &width, &height);

   f = fopen(dest, "wb+");
   if (!f)
     {
        fprintf(stderr, "ERROR: could not open for writing '%s': %s\n",
                dest, strerror(errno));
        return;
     }

   pixels = ecore_evas_buffer_pixels_get(ee);
   pixels_end = pixels + (width * height);

   /* PPM P6 format is dead simple to write: */
   fprintf(f, "P6\n%d %d\n255\n", width, height);
   for (; pixels < pixels_end; pixels++)
     {
        int r, g, b;

        r = ((*pixels) & 0xff0000) >> 16;
        g = ((*pixels) & 0x00ff00) >> 8;
        b = (*pixels) & 0x0000ff;

        fprintf(f, "%c%c%c", r, g, b);
     }

   fclose(f);
   printf("Saved scene as '%s'\n", dest);
}

int
main(void)
{
   Evas *canvas;
   Evas_Object *bg, *r1, *r2, *r3;

   ecore_evas_init();

   ee = ecore_evas_buffer_new(WIDTH, HEIGHT);
   if (!ee) goto error;

   canvas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 255, 255, 255, 255); /* white bg */
   evas_object_move(bg, 0, 0); /* at origin */
   evas_object_resize(bg, WIDTH, HEIGHT); /* covers full canvas */
   evas_object_show(bg);

   r1 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r1, 255, 0, 0, 255); /* 100% opaque red */
   evas_object_move(r1, 10, 10);
   evas_object_resize(r1, 100, 100);
   evas_object_show(r1);

   r2 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r2, 0, 128, 0, 128); /* 50% opaque green */
   evas_object_move(r2, 10, 10);
   evas_object_resize(r2, 50, 50);
   evas_object_show(r2);

   r3 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r3, 0, 128, 0, 255); /* 100% opaque dark green */
   evas_object_move(r3, 60, 60);
   evas_object_resize(r3, 50, 50);
   evas_object_show(r3);

   ecore_evas_manual_render(ee);
   _scene_save(canvas, "/tmp/evas-buffer-simple-render.ppm");

   ecore_evas_free(ee);
   ecore_evas_shutdown();

   return 0;

error:
   fprintf(stderr, "You got to have at least one Evas engine built"
                   " and linked up to ecore-evas for this example to run"
                   " properly.\n");
   ecore_evas_shutdown();
   return -1;
}

