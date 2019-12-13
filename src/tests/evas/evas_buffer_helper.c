
#include <Evas.h>
#include <Evas_Engine_Buffer.h>
#include <stdio.h>
#include <errno.h>

/* Convenience routine to allocate and initialize the canvas.
 * In a real application we'd be using ecore_evas_buffer_new() instead.
 */
Evas *canvas_create(int width, int height)
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
void canvas_destroy(Evas *canvas)
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

/*
 * return internal buffer address
*/
void* canvas_buffer(Evas *canvas)
{
   Evas_Engine_Info_Buffer *einfo;
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(canvas);
   if (!einfo)
     {
      fputs("ERROR: could not get evas engine info!\n", stderr);
      return NULL;
     }
  return einfo->info.dest_buffer;
}

/* Convenience routine to update the scene.
 * In a real application Ecore Evas would be doing this for us.
 */
void canvas_draw(Evas *canvas)
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
void canvas_save(Evas *canvas, const char *dest)
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