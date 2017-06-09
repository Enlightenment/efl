/**
 * Example of using the buffer engine in Evas.
 *
 * In the evas-init-shutdown.c example we looked at how to turn Evas on
 * and off.  In this example we'll turn it on and off but also do
 * something in between.  We'll set up a canvas and add a few rectangles
 * to show how graphics objects are configured.
 *
 * In Evas, graphic rendering is done by a 'backend engine', which can
 * be changed to match the capabilities of the underlying hardware.  For
 * this example to keep things simple we will use the 'buffer engine'.
 * The buffer engine simply does all the rendering directly into a memory
 * buffer, with no hardware acceleration.
 *
 * In real world usage you probably would not be using the raw buffer
 * access that we'll be looking at here, but instead using higher level
 * functionality from Ecore and Ecore's Ecore-Evas submodule.  Ecore
 * provides convenience routines for creating and managing the canvas,
 * integrating with the main loop, and automating scene drawing.  Since
 * we're not yet ready to dig into Ecore's functionality, we'll
 * substitute our own dummy routines create_canvas(), destroy_canvas(),
 * and draw_scene(), so we can focus on the overall process flow in
 * main().
 *
 * For this example you must have Evas compiled with the buffer engine,
 * and have the evas-software-buffer pkg-config files installed.
 *
 * @verbatim
 * gcc -o evas-buffer-simple evas-buffer-simple.c `pkg-config --libs --cflags evas evas-software-buffer`
 * @endverbatim
 */

#include <Evas.h>
#include <Evas_Engine_Buffer.h>
#include <stdio.h>
#include <errno.h>

#define WIDTH (320)
#define HEIGHT (240)

static Evas *create_canvas(int width, int height);
static void destroy_canvas(Evas *canvas);
static void draw_scene(Evas *canvas);
static void save_scene(Evas *canvas, const char *dest);

int main(void)
{
   Evas *canvas;
   Evas_Object *bg, *r1, *r2, *r3;

   evas_init();

   /* After turning Evas on, we create an Evas canvas to work in.
    * Canvases are graphical workspaces used for placing and organizing
    * graphical objects.  Normally we'd be using Ecore-Evas to create
    * the canvas, but for this example we'll hide the details in a
    * separate routine for convenience.
    */
   canvas = create_canvas(WIDTH, HEIGHT);
   if (!canvas)
     return -1;

   /* Next set the background to solid white.  This is typically done by
    * creating a rectangle sized to the canvas, placed at the canvas
    * origin.
    *
    * Note that if the canvas were to change size, our background
    * rectangle will not automatically resize itself; we'd need to do
    * that manually with another evas_object_resize() call.  In a real
    * application using Ecore-Evas, functionality in Ecore will take
    * care of resizing things.  For this example, we'll just keep the
    * canvas dimensions fixed to avoid the problem.
    */
   bg = evas_object_rectangle_add(canvas);
   evas_object_color_set(bg, 255, 255, 255, 255); // white bg, no transparency
   evas_object_move(bg, 0, 0);                    // at origin
   evas_object_resize(bg, WIDTH, HEIGHT);         // covers full canvas
   evas_object_show(bg);

   puts("initial scene, with just background:");
   draw_scene(canvas);

   /* To make the scene interesting let's add a few more rectangles of
    * various sizes and colors, starting with a big red one.
    *
    * By default all Evas objects are created in a 'hidden' state,
    * meaning they are not visible, won't be checked for changes during
    * canvas rendering, and won't receive input events.  Thus, like we
    * did for the background object we must call evas_object_show() to
    * make our graphics objects usable.
    */
   r1 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r1, 255, 0, 0, 255); // 100% opaque red
   evas_object_move(r1, 10, 10);
   evas_object_resize(r1, 100, 100);
   evas_object_show(r1);

   /* Let's add a partly transparent rectangle on top of the red one.
    *
    * Graphics objects are treated as a stack in the canvas for drawing
    * purposes, so subsequent objects are drawn above the ones we've
    * already added to the canvas.  This is important in objects that
    * have partially transparent fill coloring since we'll see part of
    * what's "behind" our object.
    *
    * In Evas, color values are pre-multiplied by their alpha.  This means
    * that if we want a green rectangle that's half transparent, we'd have:
    *
    * non-premul: r=0, g=255, b=0    a=128 (50% alpha)
    * premul:
    *         r_premul = r * a / 255 =      0 * 128 / 255 =      0
    *         g_premul = g * a / 255 =    255 * 128 / 255 =    128
    *         b_premul = b * a / 255 =      0 * 128 / 255 =      0
    *
    * Since we're placing our half transparent green rectangle on top of
    * a red one, in the final output we will actually see a yellow square
    * (since in RGBA color green + red = yellow).
    */
   r2 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r2, 0, 128, 0, 128); // 50% opaque green
   evas_object_move(r2, 10, 10);
   evas_object_resize(r2, 50, 50);
   evas_object_show(r2);

   /* Lastly, for comparison add a dark green rectangle with no
    * transparency. */
   r3 = evas_object_rectangle_add(canvas);
   evas_object_color_set(r3, 0, 128, 0, 255); // 100% opaque dark green
   evas_object_move(r3, 60, 60);
   evas_object_resize(r3, 50, 50);
   evas_object_show(r3);

   puts("final scene (note updates):");
   draw_scene(canvas);

   /* In addition to displaying the canvas to the screen, let's also
    * output the buffer to a graphics file, for comparison.  Evas
    * supports a range of graphics file formats, but PPM is particularly
    * trivial to write, so our save_scene routine will output as PPM.
    */
   save_scene(canvas, "/tmp/evas-buffer-simple-render.ppm");

   destroy_canvas(canvas);

   evas_shutdown();

   return 0;
}

/* Convenience routine to allocate and initialize the canvas.
 * In a real application we'd be using ecore_evas_buffer_new() instead.
 */
static Evas *create_canvas(int width, int height)
{
   Evas *canvas;
   Evas_Engine_Info_Buffer *einfo;
   int method;
   void *pixels;

   /* Request a handle for the 'buffer' type of rendering engine. */
   method = evas_render_method_lookup("buffer");
   if (method <= 0)
     {
	fputs("ERROR: evas was not compiled with 'buffer' engine!\n", stderr);
	return NULL;
     }

   /* Create a general canvas object.
    * Note that we are responsible for freeing the canvas when we're done. */
   canvas = evas_new();
   if (!canvas)
     {
	fputs("ERROR: could not instantiate new evas canvas.\n", stderr);
	return NULL;
     }

   /* Specify that the canvas will be rendering using the buffer engine method.
    * We also size the canvas and viewport to the same width and height, with
    * the viewport set to the origin of the canvas.
    */
   evas_output_method_set(canvas, method);
   evas_output_size_set(canvas, width, height);
   evas_output_viewport_set(canvas, 0, 0, width, height);

   /* Before we can use the engine, we *must* set its configuration
    * parameters.  The available parameters are kept in a struct
    * named Evas_Engine_Info which is internal to Evas.  Thus to set
    * parameters we must first request the current info object from
    * our canvas:
    */
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(canvas);
   if (!einfo)
     {
	fputs("ERROR: could not get evas engine info!\n", stderr);
	evas_free(canvas);
	return NULL;
     }

   /* Create the underlying data buffer that our canvas will use.  This
    * is a simple array of ARGB32 pixels.  Each color component
    * (including alpha) is one byte, resulting in 4 bytes per pixel (or
    * 32 bits).  We can thus store each pixel in an integer data type,
    * thus calculating our data buffer as W x H x sizeof(int) bytes in
    * length.
    */
   pixels = malloc(width * height * sizeof(int));
   if (!pixels)
     {
	fputs("ERROR: could not allocate canvas pixels!\n", stderr);
	evas_free(canvas);
	return NULL;
     }

   /* Next set the various configuration parameters.  We
    * register the pixel buffer that the canvas will use,
    * indicate the pixel format as ARGB32, and the size of
    * each row of data. */
   einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
   einfo->info.dest_buffer = pixels;
   einfo->info.dest_buffer_row_bytes = width * sizeof(int);
   einfo->info.use_color_key = 0;
   einfo->info.alpha_threshold = 0;
   einfo->info.func.new_update_region = NULL;
   einfo->info.func.free_update_region = NULL;

   /* Finally, we configure the canvas with our chosen parameters. */
   evas_engine_info_set(canvas, (Evas_Engine_Info *)einfo);

   return canvas;
}

/* Convenience routine to shut down the canvas.
 * In a real application we'd be using ecore_evas_free() instead
 */
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

   /* Free the data buffer we allocated in create_buffer() */
   free(einfo->info.dest_buffer);

   /* Finally, free the canvas itself. */
   evas_free(canvas);
}

/* Convenience routine to update the scene.
 * In a real application Ecore Evas would be doing this for us.
 */
static void draw_scene(Evas *canvas)
{
   Eina_List *updates, *n;
   Eina_Rectangle *update;

   /* Render the canvas, and get a list of the updated rectangles. */
   updates = evas_render_updates(canvas);

   /* Just for informative purposes, print out the areas being updated: */
   EINA_LIST_FOREACH(updates, n, update)
     printf("UPDATED REGION: pos: %3d, %3d    size: %3dx%3d\n",
	    update->x, update->y, update->w, update->h);

   /* Free the list of update rectangles */
   evas_render_updates_free(updates);
}

/* Output the canvas buffer to a Portable Pixel Map (PPM) file */
static void save_scene(Evas *canvas, const char *dest)
{
   Evas_Engine_Info_Buffer *einfo;
   const unsigned int *pixels, *pixels_end;
   int width, height;
   FILE *f;

   /* Retrieve the current data buffer. */
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(canvas);
   if (!einfo)
     {
	fputs("ERROR: could not get evas engine info!\n", stderr);
	return;
     }

   /* Retrieve the canvas dimensions */
   evas_output_size_get(canvas, &width, &height);

   /* Open our output PPM file for writing */
   f = fopen(dest, "wb+");
   if (!f)
     {
	fprintf(stderr, "ERROR: could not open for writing '%s': %s\n",
		dest, strerror(errno));
	return;
     }

   /* Write out the pixel data to the PPM file */
   pixels = einfo->info.dest_buffer;
   pixels_end = pixels + (width * height);

   /* PPM P6 format is dead simple to write.  First we output a magic
    * number 'P6' to designate the file as PPM, then the width and
    * height on their own line in ASCII decimal, followed by the maximum
    * color value (255) on its own line in ASCII decimal, and finally a
    * the pixel data in RGB order with each color component written as
    * a char (byte).  No alpha information is stored.
    */
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
