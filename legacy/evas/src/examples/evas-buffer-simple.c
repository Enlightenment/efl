/**
 * Simple Evas example using the Buffer engine.
 *
 * You must have Evas compiled with the buffer engine, and have the
 * evas-software-buffer pkg-config files installed.
 *
 * Compile with:
 *
 * @verbatim
 * gcc -o evas-buffer-simple evas-buffer-simple.c `pkg-config --libs --cflags evas evas-software-buffer`
 * @endverbatim
 *
 */
#include <Evas.h>
#include <Evas_Engine_Buffer.h>
#include <stdio.h>
#include <errno.h>

#define WIDTH (320)
#define HEIGHT (240)

/*
 * create_canvas(), destroy_canvas() and draw_scene() are support functions.
 *
 * They are only required to use raw Evas, but for real world usage,
 * it is recommended to use ecore and its ecore-evas submodule, that
 * provide convenience canvas creators, integration with main loop and
 * automatic render of updates (draw_scene()) when system goes back to
 * main loop.
 */
static Evas *create_canvas(int width, int height);
static void destroy_canvas(Evas *canvas);
static void draw_scene(Evas *canvas);

// support function to save scene as PPM image
static void save_scene(Evas *canvas, const char *dest);

int main(void)
{
   Evas *canvas;
   Evas_Object *bg, *r1, *r2, *r3;

   evas_init();

   // create your canvas
   // NOTE: consider using ecore_evas_buffer_new() instead!
   canvas = create_canvas(WIDTH, HEIGHT);
   if (!canvas)
     return -1;

   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 255, 255, 255, 255); // white bg
   evas_object_move(bg, 0, 0);                    // at origin
   evas_object_resize(bg, WIDTH, HEIGHT);         // covers full canvas
   evas_object_show(bg);

   puts("initial scene, with just background:");
   draw_scene(canvas);

   r1 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r1, 255, 0, 0, 255); // 100% opaque red
   evas_object_move(r1, 10, 10);
   evas_object_resize(r1, 100, 100);
   evas_object_show(r1);

   // pay attention to transparency! Evas color values are pre-multiplied by
   // alpha, so 50% opaque green is:
   // non-premul: r=0, g=255, b=0    a=128 (50% alpha)
   // premul:
   //         r_premul = r * a / 255 =      0 * 128 / 255 =      0
   //         g_premul = g * a / 255 =    255 * 128 / 255 =    128
   //         b_premul = b * a / 255 =      0 * 128 / 255 =      0
   //
   // this 50% green is over a red background, so it will show in the
   // final output as yellow (green + red = yellow)
   r2 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r2, 0, 128, 0, 128); // 50% opaque green
   evas_object_move(r2, 10, 10);
   evas_object_resize(r2, 50, 50);
   evas_object_show(r2);

   r3 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r3, 0, 128, 0, 255); // 100% opaque dark green
   evas_object_move(r3, 60, 60);
   evas_object_resize(r3, 50, 50);
   evas_object_show(r3);

   puts("final scene (note updates):");
   draw_scene(canvas);
   save_scene(canvas, "/tmp/evas-buffer-simple-render.ppm");

   // NOTE: use ecore_evas_buffer_new() and here ecore_evas_free()
   destroy_canvas(canvas);

   evas_shutdown();

   return 0;
}

static Evas *create_canvas(int width, int height)
{
   Evas *canvas;
   Evas_Engine_Info_Buffer *einfo;
   int method;
   void *pixels;

   method = evas_render_method_lookup("buffer");
   if (method <= 0)
     {
	fputs("ERROR: evas was not compiled with 'buffer' engine!\n", stderr);
	return NULL;
     }

   canvas = evas_new();
   if (!canvas)
     {
	fputs("ERROR: could not instantiate new evas canvas.\n", stderr);
	return NULL;
     }

   evas_output_method_set(canvas, method);
   evas_output_size_set(canvas, width, height);
   evas_output_viewport_set(canvas, 0, 0, width, height);

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(canvas);
   if (!einfo)
     {
	fputs("ERROR: could not get evas engine info!\n", stderr);
	evas_free(canvas);
	return NULL;
     }

   // ARGB32 is sizeof(int), that is 4 bytes, per pixel
   pixels = malloc(width * height * sizeof(int));
   if (!pixels)
     {
	fputs("ERROR: could not allocate canvas pixels!\n", stderr);
	evas_free(canvas);
	return NULL;
     }

   einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
   einfo->info.dest_buffer = pixels;
   einfo->info.dest_buffer_row_bytes = width * sizeof(int);
   einfo->info.use_color_key = 0;
   einfo->info.alpha_threshold = 0;
   einfo->info.func.new_update_region = NULL;
   einfo->info.func.free_update_region = NULL;
   evas_engine_info_set(canvas, (Evas_Engine_Info *)einfo);

   return canvas;
}

static void destroy_canvas(Evas *canvas)
{
   Evas_Engine_Info_Buffer *einfo;

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(canvas);
   if (!einfo)
     {
	fputs("ERROR: could not get evas engine info!\n", stderr);
	evas_free(canvas);
	return;
     }

   free(einfo->info.dest_buffer);
   evas_free(canvas);
}

static void draw_scene(Evas *canvas)
{
   Eina_List *updates, *n;
   Eina_Rectangle *update;

   // render and get the updated rectangles:
   updates = evas_render_updates(canvas);

   // informative only here, just print the updated areas:
   EINA_LIST_FOREACH(updates, n, update)
     printf("UPDATED REGION: pos: %3d, %3d    size: %3dx%3d\n",
	    update->x, update->y, update->w, update->h);

   // free list of updates
   evas_render_updates_free(updates);
}

static void save_scene(Evas *canvas, const char *dest)
{
   Evas_Engine_Info_Buffer *einfo;
   const unsigned int *pixels, *pixels_end;
   int width, height;
   FILE *f;

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(canvas);
   if (!einfo)
     {
	fputs("ERROR: could not get evas engine info!\n", stderr);
	return;
     }
   evas_output_size_get(canvas, &width, &height);

   f = fopen(dest, "wb+");
   if (!f)
     {
	fprintf(stderr, "ERROR: could not open for writing '%s': %s\n",
		dest, strerror(errno));
	return;
     }

   pixels = einfo->info.dest_buffer;
   pixels_end = pixels + (width * height);

   // PPM P6 format is dead simple to write:
   fprintf(f, "P6\n%d %d\n255\n",  width, height);
   for (; pixels < pixels_end; pixels++)
     {
	int r, g, b;

	r = ((*pixels) & 0xff0000) >> 16;
	g = ((*pixels) & 0x00ff00) >> 8;
	b = (*pixels) & 0x0000ff;

	fprintf(f, "%c%c%c", r, g, b);
     }

   fclose(f);
   printf("saved scene as '%s'\n", dest);
}
