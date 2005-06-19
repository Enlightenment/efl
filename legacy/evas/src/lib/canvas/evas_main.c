#include "evas_common.h"
#include "evas_private.h"
#ifdef BUILD_ENGINE_SOFTWARE_X11
#include "evas_engine_api_software_x11.h"
#endif
#ifdef BUILD_ENGINE_SOFTWARE_XCB
#include "evas_engine_api_software_xcb.h"
#endif
#ifdef BUILD_ENGINE_DIRECTFB
#include "evas_engine_api_directfb.h"
#endif
#ifdef BUILD_ENGINE_FB
#include "evas_engine_api_fb.h"
#endif
#ifdef BUILD_ENGINE_BUFFER
#include "evas_engine_api_buffer.h"
#endif
#ifdef BUILD_ENGINE_SOFTWARE_WIN32_GDI
#include "evas_engine_api_software_win32_gdi.h"
#endif
#ifdef BUILD_ENGINE_SOFTWARE_QTOPIA
#include "evas_engine_api_software_qtopia.h"
#endif
#ifdef BUILD_ENGINE_GL_X11
#include "evas_engine_api_gl_x11.h"
#endif
#ifdef BUILD_ENGINE_CAIRO_X11
#include "evas_engine_api_cairo_x11.h"
#endif

static int initcount = 0;

int
evas_init(void)
{
   return ++initcount;
}

int
evas_shutdown(void)
{
   if (--initcount == 0)
      evas_font_dir_cache_free();

   return initcount;
}

/**
 * @defgroup Evas_Canvas Evas Canvas
 *
 * Functions that deal with the basic evas object.  They are the
 * functions you need to use at a minimum to get a working evas, and
 * to destroy it.
 *
 */

/**
 * Creates a new empty evas.
 *
 * Note that before you can use the evas, you will to at a minimum:
 * @li Set its render method with @ref evas_output_method_set .
 * @li Set its viewport size with @ref evas_output_viewport_set .
 * @li Set its size of the canvas with @ref evas_output_size_set .
 * @li Ensure that the render engine is given the correct settings with
 *     @ref evas_engine_info_set .
 *
 * This function should only fail if the memory allocation fails.
 *
 * @return  A new uninitialised Evas canvas on success.  Otherwise, @c NULL.
 * @ingroup Evas_Canvas
 */
Evas *
evas_new(void)
{
   Evas *e;

   e = calloc(1, sizeof(Evas));
   if (!e) return NULL;

   e->magic = MAGIC_EVAS;
   e->output.render_method = RENDER_METHOD_INVALID;
   e->viewport.w = 1;
   e->viewport.h = 1;
   return e;
}

/**
 * Frees the given evas and any objects created on it.
 *
 * Any objects with 'free' callbacks will have those callbacks called
 * in this function.
 *
 * @param   e The given evas.
 * @ingroup Evas_Canvas
 */
void
evas_free(Evas *e)
{
   Evas_Object_List *l;
   int i;
   int del;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   del = 1;
   while (del)
     {
	del = 0;
	for (l = (Evas_Object_List *)(e->layers); l; l = l->next)
	  {
	     Evas_Layer *lay;
	     Evas_Object_List *ll;

	     lay = (Evas_Layer *)l;
	     evas_layer_pre_free(lay);
	     for (ll = (Evas_Object_List *)lay->objects; ll; ll = ll->next)
	       {
		  Evas_Object *o;

		  o = (Evas_Object *)ll;
		  if (!o->delete_me)
		    del = 1;
	       }
	  }
     }
   while (e->layers)
     {
	Evas_Layer *lay;

	lay = e->layers;
	evas_layer_del(lay);
	evas_layer_free(lay);
     }

   evas_font_path_clear(e);
   e->pointer.object.in = evas_list_free(e->pointer.object.in);

   if (e->name_hash) evas_hash_free(e->name_hash);

   while (e->damages)
     {
	free(e->damages->data);
	e->damages = evas_list_remove(e->damages, e->damages->data);
     }
   while (e->obscures)
     {
	free(e->obscures->data);
	e->obscures = evas_list_remove(e->obscures, e->obscures->data);
     }

   if (e->engine.func) 
     {
	e->engine.func->info_free(e, e->engine.info);
	e->engine.func->context_free(e->engine.data.output, e->engine.data.context);
	e->engine.func->output_free(e->engine.data.output);
     }

   for (i = 0; i < e->modifiers.mod.count; i++)
     free(e->modifiers.mod.list[i]);
   if (e->modifiers.mod.list) free(e->modifiers.mod.list);

   for (i = 0; i < e->locks.lock.count; i++)
     free(e->locks.lock.list[i]);
   if (e->locks.lock.list) free(e->locks.lock.list);

   e->magic = 0;
   free(e);
}

/**
 * @defgroup Evas_Output_Method Evas Render Engine Functions
 *
 * Functions that are used to set the render engine for a given function, and
 * then get that engine working.
 *
 * The following code snippet shows how they can be used to
 * initialise an evas that uses the X11 software engine:
 * @code
 * Evas *evas;
 * Evas_Engine_Info_Software_X11 *einfo;
 * extern Display *display;
 * extern Window win;
 *
 * evas = evas_new();
 * evas_output_method_set(evas, evas_render_method_lookup("software_x11"));
 * evas_output_size_set(evas, 640, 480);
 * evas_output_viewport_set(evas, 0, 0, 640, 480);
 * einfo = (Evas_Engine_Info_Software_X11 *)evas_engine_info_get(evas);
 * einfo->info.display = display;
 * einfo->info.visual = DefaultVisual(display, DefaultScreen(display));
 * einfo->info.colormap = DefaultColormap(display, DefaultScreen(display));
 * einfo->info.drawable = win;
 * einfo->info.depth = DefaultDepth(display, DefaultScreen(display));
 * evas_engine_info_set(evas, (Evas_Engine_Info *)einfo);
 * @endcode
 */

/**
 * Sets the output engine for the given evas.
 *
 * Once the output engine for an evas is set, any attempt to change it
 * will be ignored.  The value for @p render_method can be found using
 * @ref evas_render_method_lookup .
 *
 * @param   e             The given evas.
 * @param   render_method The numeric engine value to use.
 * @ingroup Evas_Output_Method
 */
void
evas_output_method_set(Evas *e, int render_method)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   /* if our engine to set it to is invalid - abort */
   if (render_method == RENDER_METHOD_INVALID) return;
   /* if the engine is already set up - abort */
   if (e->output.render_method != RENDER_METHOD_INVALID) return;
   /* set the render method */
   e->output.render_method = render_method;
#ifdef BUILD_ENGINE_SOFTWARE_X11
   if (e->output.render_method == RENDER_METHOD_SOFTWARE_X11)
     e->engine.func = &evas_engine_software_x11_func;
   else
#endif
#ifdef BUILD_ENGINE_SOFTWARE_XCB
     if (e->output.render_method == RENDER_METHOD_SOFTWARE_XCB)
     e->engine.func = &evas_engine_software_xcb_func;
   else
#endif
#ifdef BUILD_ENGINE_GL_X11
   if (e->output.render_method == RENDER_METHOD_GL_X11)
     e->engine.func = &evas_engine_gl_x11_func;
   else
#endif
#ifdef BUILD_ENGINE_CAIRO_X11
   if (e->output.render_method == RENDER_METHOD_CAIRO_X11)
     e->engine.func = &evas_engine_cairo_x11_func;
   else
#endif
#ifdef BUILD_ENGINE_DIRECTFB
   if (e->output.render_method == RENDER_METHOD_DIRECTFB)
     e->engine.func = &evas_engine_directfb_func;
   else
#endif
#ifdef BUILD_ENGINE_FB
   if (e->output.render_method == RENDER_METHOD_FB)
     e->engine.func = &evas_engine_fb_func;
   else
#endif
#ifdef BUILD_ENGINE_BUFFER
   if (e->output.render_method == RENDER_METHOD_BUFFER)
     e->engine.func = &evas_engine_buffer_func;
   else
#endif
#ifdef BUILD_ENGINE_SOFTWARE_WIN32_GDI
   if (e->output.render_method == RENDER_METHOD_SOFTWARE_WIN32_GDI)
     e->engine.func = &evas_engine_software_win32_gdi_func;
   else
#endif
#ifdef BUILD_ENGINE_SOFTWARE_QTOPIA
   if (e->output.render_method == RENDER_METHOD_SOFTWARE_QTOPIA)
     e->engine.func = &evas_engine_software_qtopia_func;
   else
#endif
   return;
   /* get the engine info struct */
   if (e->output.render_method != RENDER_METHOD_INVALID)
     {
	if (e->engine.func->info) e->engine.info = e->engine.func->info(e);
     }
}

/**
 * Retrieves the number of the output engine used for the given evas.
 * @param   e The given evas.
 * @return  The ID number of the output engine being used.  @c 0 is
 *          returned if there is an error.
 * @ingroup Evas_Output_Method
 */
int
evas_output_method_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return RENDER_METHOD_INVALID;
   MAGIC_CHECK_END();

   return e->output.render_method;
}

/**
 * Retrieves the current render engine info struct from the given evas.
 *
 * The returned structure is publicly modifiable.  The contents are valid
 * until either @ref evas_engine_info_set or @ref evas_render are called.
 *
 * This structure does not need to be freed by the caller.
 *
 * @param   e The given evas.
 * @return  A pointer to the Engine Info structure.  @c NULL is returned if
 *          an engine has not yet been assigned.
 * @ingroup Evas_Output_Method
 */
Evas_Engine_Info *
evas_engine_info_get(Evas *e)
{
   Evas_Engine_Info *info;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   if (!e->engine.info) return NULL;

   info = e->engine.info;
   e->engine.info_magic = info->magic;

   return info;
}

/**
 * Applies the engine settings for the given evas from the given @c
 * Evas_Engine_Info structure.
 *
 * To get the Evas_Engine_Info structure to use, call @ref
 * evas_engine_info_get .  Do not try to obtain a pointer to an
 * @c Evas_Engine_Info structure in any other way.
 *
 * You will need to call this function at least once before you can
 * create objects on an evas or render that evas.  Some engines allow
 * their settings to be changed more than once.
 *
 * Once called, the @p info pointer should be considered invalid.
 *
 * Example:
 *
 * @param   e    The pointer to the Evas Canvas
 * @param   info The pointer to the Engine Info to use
 * @ingroup Evas_Output_Method
 */
void
evas_engine_info_set(Evas *e, Evas_Engine_Info *info)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!info) return;
   if (info != e->engine.info) return;
   if (info->magic != e->engine.info_magic) return;
   e->engine.func->setup(e, info);
}

/**
 * @defgroup Evas_Output_Size Evas Output and Viewport Resizing Functions
 *
 * Functions that set and retrieve the output and viewport size of an evas.
 */

/**
 * Sets the output size of the render engine of the given evas.
 *
 * The evas will render to a rectangle of the given size once this
 * function is called.  The output size is independent of the viewport
 * size.  The viewport will be stretched to fill the given rectangle.
 *
 * The units used for @p w and @p h depend on the engine used by the
 * evas.
 *
 * @param   e The given evas.
 * @param   w The width in output units, usually pixels.
 * @param   h The height in output units, usually pixels.
 * @ingroup Evas_Output_Size
 */
void
evas_output_size_set(Evas *e, int w, int h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((w == e->output.w) && (h == e->output.h)) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   e->output.w = w;
   e->output.h = h;
   e->output.changed = 1;
   e->output_validity++;
   e->changed = 1;
}

/**
 * Retrieve the output size of the render engine of the given evas.
 *
 * The output size is given in whatever the output units are for the
 * engine.
 *
 * If either @p w or @p h is @c NULL, then it is ignored.  If @p e is
 * invalid, the returned results are undefined.
 *
 * @param   e The given evas.
 * @param   w The pointer to an integer to store the width in.
 * @param   h The pointer to an integer to store the height in.
 * @ingroup Evas_Output_Size
 */
void
evas_output_size_get(Evas *e, int *w, int *h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();

   if (w) *w = e->output.w;
   if (h) *h = e->output.h;
}

/**
 * Sets the output viewport of the given evas in evas units.
 *
 * The output viewport is the area of the evas that will be visible to the
 * viewer.  The viewport will be stretched to fit the output target of the
 * evas when rendering is performed.
 *
 * @note The coordinate values do not have to map 1-to-1 with the output
 *       target.  However, it is generally advised that it is done for ease
 *       of use.
 *
 * @param   e The given evas.
 * @param   x The top-left corner x value of the viewport.
 * @param   y The top-left corner y value of the viewport.
 * @param   w The width of the viewport.  Must be greater than 0.
 * @param   h The height of the viewport.  Must be greater than 0.
 * @ingroup Evas_Output_Size
 */
void
evas_output_viewport_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((x == e->viewport.x) && (y == e->viewport.y) &&
       (w == e->viewport.w) && (h == e->viewport.h)) return;
   if (w <= 0) return;
   if (h <= 0) return;
   e->viewport.x = x;
   e->viewport.y = y;
   e->viewport.w = w;
   e->viewport.h = h;
   e->viewport.changed = 1;
   e->output_validity++;
   e->changed = 1;
}

/**
 * Get the render engine's output viewport co-ordinates in canvas units.
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a x variable to be filled in
 * @param y The pointer to a y variable to be filled in
 * @param w The pointer to a width variable to be filled in
 * @param h The pointer to a height variable to be filled in
 * @ingroup Evas_Output_Size
 *
 * Calling this function writes the current canvas output viewport size and
 * location values into the variables pointed to by @p x, @p y, @p w and @p h.
 * On success the variables have the output location and size values written to
 * them in canvas units. Any of @p x, @p y, @p w or @p h that are NULL will not
 * be written to. If @p e is invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord x, y, width, height;
 *
 * evas_output_viewport_get(evas, &x, &y, &w, &h);
 * @endcode
 */
void
evas_output_viewport_get(Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();

   if (x) *x = e->viewport.x;
   if (y) *y = e->viewport.y;
   if (w) *w = e->viewport.w;
   if (h) *h = e->viewport.h;
}

/**
 * @defgroup Evas_Coord_Mapping_Group Evas Coordinate Mapping Functions
 *
 * Functions that are used to map coordinates from the canvas to the
 * screen or the screen to the canvas.
 */

/**
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The screen/output x co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a horizontal co-ordinate as the @p x parameter and
 * converts it into canvas units, accounting for output size, viewport size
 * and location, returning it as the function return value. If @p e is
 * invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern int screen_x;
 * Evas_Coord canvas_x;
 *
 * canvas_x = evas_coord_screen_x_to_world(evas, screen_x);
 * @endcode
 */
Evas_Coord
evas_coord_screen_x_to_world(Evas *e, int x)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.w == e->viewport.w) return e->viewport.x + x;
   return (long long)e->viewport.x + (((long long)x * (long long)e->viewport.w) / (long long)e->output.w);
}

/**
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param y The screen/output y co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a vertical co-ordinate as the @p y parameter and
 * converts it into canvas units, accounting for output size, viewport size
 * and location, returning it as the function return value. If @p e is
 * invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * extern int screen_y;
 * Evas_Coord canvas_y;
 *
 * canvas_y = evas_coord_screen_y_to_world(evas, screen_y);
 * @endcode
 */
Evas_Coord
evas_coord_screen_y_to_world(Evas *e, int y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.h == e->viewport.h) return e->viewport.y + y;
   return (long long)e->viewport.y + (((long long)y * (long long)e->viewport.h) / (long long)e->output.h);
}

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The canvas x co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a horizontal co-ordinate as the @p x parameter and
 * converts it into output units, accounting for output size, viewport size
 * and location, returning it as the function return value. If @p e is
 * invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int screen_x;
 * extern Evas_Coord canvas_x;
 *
 * screen_x = evas_coord_world_x_to_screen(evas, canvas_x);
 * @endcode
 */
int
evas_coord_world_x_to_screen(Evas *e, Evas_Coord x)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.w == e->viewport.w) return x - e->viewport.x;
   return (int)((((long long)x - (long long)e->viewport.x) * (long long)e->output.w) /  (long long)e->viewport.w);
}

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param y The canvas y co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
 * @ingroup Evas_Coord_Mapping_Group
 *
 * This function takes in a vertical co-ordinate as the @p x parameter and
 * converts it into output units, accounting for output size, viewport size
 * and location, returning it as the function return value. If @p e is
 * invalid, the results are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int screen_y;
 * extern Evas_Coord canvas_y;
 *
 * screen_y = evas_coord_world_y_to_screen(evas, canvas_y);
 * @endcode
 */
int
evas_coord_world_y_to_screen(Evas *e, Evas_Coord y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.h == e->viewport.h) return y - e->viewport.y;
   return (int)((((long long)y - (long long)e->viewport.y) * (long long)e->output.h) /  (long long)e->viewport.h);
}

/**
 * Look up a numeric ID from a string name of a rendering engine.
 *
 * @param name The string name of an engine
 * @return A numeric (opaque) ID for the rendering engine
 * @ingroup Evas_Output_Method
 *
 * This function looks up a numeric return value for the named engine in the
 * string @p name. This is a normal C string, NUL byte terminated. The name
 * is case sensitive. If the rendering engine is available, a numeric ID for
 * that engine is returned that is not 0. If the engine is not available, 0
 * is returned, indicating an invalid engine.
 *
 * The programmer should NEVER rely on the numeric ID of an engine unless it is
 * returned by this function. Programs should NOT be written accessing render
 * method ID's directly, without first obtaining it from this function.
 *
 * Example:
 * @code
 * int engine_id;
 * Evas *evas;
 *
 * evas = evas_new();
 * if (!evas)
 *   {
 *     fprintf(stderr, "ERROR: Canvas creation failed. Fatal error.\n");
 *     exit(-1);
 *   }
 * engine_id = evas_render_method_lookup("software_x11");
 * if (!engine_id)
 *   {
 *     fprintf(stderr, "ERROR: Requested rendering engine is absent.\n");
 *     exit(-1);
 *   }
 * evas_output_method_set(evas, engine_id);
 * @endcode
 */
int
evas_render_method_lookup(const char *name)
{
   if (!name) return RENDER_METHOD_INVALID;

#ifdef BUILD_ENGINE_SOFTWARE_X11
   if (!strcmp(name, "software_x11")) return RENDER_METHOD_SOFTWARE_X11;
#endif
#ifdef BUILD_ENGINE_SOFTWARE_XCB
   if (!strcmp(name, "software_xcb")) return RENDER_METHOD_SOFTWARE_XCB;
#endif
#ifdef BUILD_ENGINE_GL_X11
   if (!strcmp(name, "gl_x11")) return RENDER_METHOD_GL_X11;
#endif
#ifdef BUILD_ENGINE_CAIRO_X11
   if (!strcmp(name, "cairo_x11")) return RENDER_METHOD_CAIRO_X11;
#endif
#ifdef BUILD_ENGINE_DIRECTFB
   if (!strcmp(name, "directfb")) return RENDER_METHOD_DIRECTFB;
#endif
#ifdef BUILD_ENGINE_FB
   if (!strcmp(name, "fb")) return RENDER_METHOD_FB;
#endif
#ifdef BUILD_ENGINE_BUFFER
   if (!strcmp(name, "buffer")) return RENDER_METHOD_BUFFER;
#endif
#ifdef BUILD_ENGINE_SOFTWARE_WIN32_GDI
   if (!strcmp(name, "software_win32_gdi")) return RENDER_METHOD_SOFTWARE_WIN32_GDI;
#endif
#ifdef BUILD_ENGINE_SOFTWARE_QTOPIA
   if (!strcmp(name, "software_qtopia")) return RENDER_METHOD_SOFTWARE_QTOPIA;
#endif
   return RENDER_METHOD_INVALID;
}

/**
 * List all the rendering engines compiled into the copy of the Evas library
 *
 * @return A linked list whose data members are C strings of engine names
 * @ingroup Evas_Output_Method
 *
 * Calling this will return a handle (pointer) to an Evas linked list. Each node
 * in the linked list will have the data pointer be a (char *) pointer to the
 * string name of the rendering engine available. The strings should never be
 * modified, neither should the list be modified. This list should be cleaned up
 * as soon as the program no longer needs it using
 * evas_render_method_list_free(). If no engines are available from Evas, NULL
 * will be returned.
 *
 * Example:
 * @code
 * Evas_List *engine_list, *l;
 *
 * engine_list = evas_render_method_list();
 * if (!engine_list)
 *   {
 *     fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
 *     exit(-1);
 *   }
 * printf("Availible Evas Engines:\n");
 * for (l = engine_list; l; l = l->next)
 *   {
 *     char *engine_name;
 *
 *     engine_name = l->data;
 *     printf("%s\n", engine_name);
 *   }
 * evas_render_method_list_free(engine_list);
 * @endcode
 */
Evas_List *
evas_render_method_list(void)
{
   Evas_List *methods = NULL;

#ifdef BUILD_ENGINE_SOFTWARE_X11
   methods = evas_list_append(methods, strdup("software_x11"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_XCB
   methods = evas_list_append(methods, strdup("software_xcb"));
#endif
#ifdef BUILD_ENGINE_GL_X11
   methods = evas_list_append(methods, strdup("gl_x11"));
#endif
#ifdef BUILD_ENGINE_CAIRO_X11
   methods = evas_list_append(methods, strdup("cairo_x11"));
#endif
#ifdef BUILD_ENGINE_DIRECTFB
   methods = evas_list_append(methods, strdup("directfb"));
#endif
#ifdef BUILD_ENGINE_FB
   methods = evas_list_append(methods, strdup("fb"));
#endif
#ifdef BUILD_ENGINE_BUFFER
   methods = evas_list_append(methods, strdup("buffer"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_WIN32_GDI
   methods = evas_list_append(methods, strdup("software_win32_gdi"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_QTOPIA
   methods = evas_list_append(methods, strdup("software_qtopia"));
#endif

   return methods;
}

/**
 * This function should be called to free a list of engine names
 *
 * @param list The Evas_List base pointer for the engine list to be freed
 * @ingroup Evas_Output_Method
 *
 * When this function is called it will free the engine list passed in as
 * @p list. The list should only be a list of engines generated by calling
 * evas_render_method_list(). If @p list is NULL, nothing will happen.
 *
 * Example:
 * @code
 * Evas_List *engine_list, *l;
 *
 * engine_list = evas_render_method_list();
 * if (!engine_list)
 *   {
 *     fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
 *     exit(-1);
 *   }
 * printf("Availible Evas Engines:\n");
 * for (l = engine_list; l; l = l->next)
 *   {
 *     char *engine_name;
 *
 *     engine_name = l->data;
 *     printf("%s\n", engine_name);
 *   }
 * evas_render_method_list_free(engine_list);
 * @endcode
 */
void
evas_render_method_list_free(Evas_List *list)
{
   while (list)
     {
	free(list->data);
	list = evas_list_remove(list, list->data);
     }
}

/**
 * @defgroup Evas_Pointer_Group Evas Pointer Functions
 *
 * Functions that deal with the status of the pointer.
 */

/**
 * This function returns the current known pointer co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to an integer to be filled in
 * @param y The pointer to an integer to be filled in
 * @ingroup Evas_Pointer_Group
 *
 * This function returns the current known screen/output co-ordinates of the
 * mouse pointer and sets the contents of the integers pointed to by @p x
 * and @p y to contain these co-ordinates. If @p e is not a valid canvas the
 * results of this function are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at screen position %i, %i\n", mouse_x, mouse_y);
 * @endcode
 */
void
evas_pointer_output_xy_get(Evas *e, int *x, int *y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

/**
 * This function returns the current known pointer co-ordinates
 *
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to a Evas_Coord to be filled in
 * @param y The pointer to a Evas_Coord to be filled in
 * @ingroup Evas_Pointer_Group
 *
 * This function returns the current known canvas unit co-ordinates of the
 * mouse pointer and sets the contents of the Evas_Coords pointed to by @p x
 * and @p y to contain these co-ordinates. If @p e is not a valid canvas the
 * results of this function are undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * Evas_Coord mouse_x, mouse_y;
 *
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at canvas position %f, %f\n", mouse_x, mouse_y);
 * @endcode
 */
void
evas_pointer_canvas_xy_get(Evas *e, Evas_Coord *x, Evas_Coord *y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

/**
 * Returns a bitmask with the mouse buttons currently pressed, set to 1
 *
 * @param e The pointer to the Evas Canvas
 * @return A bitmask of the currently depressed buttons on the cavas
 * @ingroup Evas_Pointer_Group
 *
 * Calling this function will return a 32-bit integer with the appropriate bits
 * set to 1 that correspond to a mouse button being depressed. This limits
 * Evas to a mouse devices with a maximum of 32 buttons, but that is generally
 * in excess of any host system's pointing device abilities.
 *
 * A canvas by default begins with no mouse buttons being pressed and only
 * calls to evas_event_feed_mouse_down(), evas_event_feed_mouse_down_data(),
 * evas_event_feed_mouse_up() and evas_event_feed_mouse_up_data() will alter
 * that.
 *
 * The least significant bit corresponds to the first mouse button (button 1)
 * and the most significant bit corresponds to the last mouse button
 * (button 32).
 *
 * If @p e is not a valid canvas, the return value is undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 * int button_mask, i;
 *
 * button_mask = evas_pointer_button_down_mask_get(evas);
 * printf("Buttons currently pressed:\n");
 * for (i = 0; i < 32; i++)
 *   {
 *     if ((button_mask & (1 << i)) != 0) printf("Button %i\n", i + 1);
 *   }
 * @endcode
 */
int
evas_pointer_button_down_mask_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return (int)e->pointer.button;
}

/**
 * Returns whether the mouse pointer is logically inside the canvas
 *
 * @param e The pointer to the Evas Canvas
 * @return An integer that is 1 if the mouse is inside the canvas, 0 otherwise
 * @ingroup Evas_Pointer_Group
 *
 * When this function is called it will return a value of either 0 or 1,
 * depending on if evas_event_feed_mouse_in(), evas_event_feed_mouse_in_data(),
 * or evas_event_feed_mouse_out(), evas_event_feed_mouse_out_data() have been
 * called to feed in a mouse enter event into the canvas.
 *
 * A return value of 1 indicates the mouse is logically inside the canvas, and
 * 0 implies it is logically outside the canvas.
 *
 * A canvas begins with the mouse being assumed outside (0).
 *
 * If @p e is not a valid canvas, the return value is undefined.
 *
 * Example:
 * @code
 * extern Evas *evas;
 *
 * if (evas_pointer_inside_get(evas)) printf("Mouse is in!\n");
 * else printf("Mouse is out!\n");
 * @endcode
 */
Evas_Bool
evas_pointer_inside_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return (int)e->pointer.inside;
}
