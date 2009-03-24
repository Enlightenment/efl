#include "evas_common.h"
#include "evas_private.h"

extern Eina_List *evas_modules;
static int initcount = 0;

EAPI int
evas_init(void)
{
   if (initcount == 0)
     {
	eina_init();
	evas_module_init();
	evas_async_events_init();
     }
   return ++initcount;
}

EAPI int
evas_shutdown(void)
{
   initcount--;
   if (initcount == 0)
     {
	evas_async_events_shutdown();
	evas_font_dir_cache_free();
	evas_common_shutdown();
	evas_module_shutdown();
	eina_shutdown();
     }
   return initcount;
}

/**
 * @defgroup Evas_Canvas Evas Canvas functions
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
EAPI Evas *
evas_new(void)
{
   Evas *e;

   e = calloc(1, sizeof(Evas));
   if (!e) return NULL;

   e->magic = MAGIC_EVAS;
   e->output.render_method = RENDER_METHOD_INVALID;
   e->viewport.w = 1;
   e->viewport.h = 1;
   e->hinting = EVAS_FONT_HINTING_BYTECODE;
   e->name_hash = eina_hash_string_superfast_new(NULL);

   eina_array_step_set(&e->delete_objects, 16);
   eina_array_step_set(&e->active_objects, 16);
   eina_array_step_set(&e->restack_objects, 16);
   eina_array_step_set(&e->render_objects, 16);
   eina_array_step_set(&e->pending_objects, 16);
   eina_array_step_set(&e->obscuring_objects, 16);
   eina_array_step_set(&e->temporary_objects, 16);
   eina_array_step_set(&e->calculate_objects, 16);

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
EAPI void
evas_free(Evas *e)
{
   Evas_Layer *lay;
   int i;
   int del;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (e->walking_list == 0) evas_render_idle_flush(e);

   if (e->walking_list > 0) return;
   del = 1;
   e->walking_list++;
   e->cleanup = 1;
   while (del)
     {
	del = 0;
	EINA_INLIST_FOREACH(e->layers, lay)
	  {
	     Evas_Object *o;

	     evas_layer_pre_free(lay);

	     EINA_INLIST_FOREACH(lay->objects, o)
	       {
		  if ((o->callbacks) && (o->callbacks->walking_list))
		    {
		       /* Defer free */
		       e->delete_me = 1;
		       e->walking_list--;
		       return;
		    }
		  if (!o->delete_me)
		    del = 1;
	       }
	  }
     }
   while (e->layers)
     {
	lay = e->layers;
	evas_layer_del(lay);
	evas_layer_free(lay);
     }
   e->walking_list--;

   evas_font_path_clear(e);
   e->pointer.object.in = eina_list_free(e->pointer.object.in);

   if (e->name_hash) eina_hash_free(e->name_hash);
   e->name_hash = NULL;

   while (e->damages)
     {
	free(e->damages->data);
	e->damages = eina_list_remove(e->damages, e->damages->data);
     }
   while (e->obscures)
     {
	free(e->obscures->data);
	e->obscures = eina_list_remove(e->obscures, e->obscures->data);
     }

   evas_fonts_zero_free(e);

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

   if (e->engine.module) evas_module_unref(e->engine.module);

   eina_array_flush(&e->delete_objects);
   eina_array_flush(&e->active_objects);
   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->render_objects);
   eina_array_flush(&e->pending_objects);
   eina_array_flush(&e->obscuring_objects);
   eina_array_flush(&e->temporary_objects);
   eina_array_flush(&e->calculate_objects);

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
EAPI void
evas_output_method_set(Evas *e, int render_method)
{
   Eina_List *l;
   Evas_Module *em;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   /* if our engine to set it to is invalid - abort */
   if (render_method == RENDER_METHOD_INVALID) return;
   /* if the engine is already set up - abort */
   if (e->output.render_method != RENDER_METHOD_INVALID) return;
   /* iterate trough the list to find the id */
   EINA_LIST_FOREACH(evas_modules, l, em)
     {
	Evas_Module_Engine *eme;

	if (em->type != EVAS_MODULE_TYPE_ENGINE) continue;
	if (!em->data) continue;
	eme = (Evas_Module_Engine *)em->data;
	if (eme->id != render_method) continue;
	if (!evas_module_load(em)) return;
	/* set the correct render */
	e->output.render_method = render_method;
	e->engine.func = (em->functions);
	evas_module_use(em);
	if (e->engine.module) evas_module_unref(e->engine.module);
	e->engine.module = em;
	evas_module_ref(em);
	/* get the engine info struct */
	if (e->engine.func->info) e->engine.info = e->engine.func->info(e);
	return;
     }
}

/**
 * Retrieves the number of the output engine used for the given evas.
 * @param   e The given evas.
 * @return  The ID number of the output engine being used.  @c 0 is
 *          returned if there is an error.
 * @ingroup Evas_Output_Method
 */
EAPI int
evas_output_method_get(const Evas *e)
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
EAPI Evas_Engine_Info *
evas_engine_info_get(const Evas *e)
{
   Evas_Engine_Info *info;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   if (!e->engine.info) return NULL;

   info = e->engine.info;
   ((Evas *)e)->engine.info_magic = info->magic;

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
 * @return  1 if no error occured, 0 otherwise
 * @ingroup Evas_Output_Method
 */
EAPI int
evas_engine_info_set(Evas *e, Evas_Engine_Info *info)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (!info) return 0;
   if (info != e->engine.info) return 0;
   if (info->magic != e->engine.info_magic) return 0;
   return e->engine.func->setup(e, info);
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
EAPI void
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
   evas_render_invalidate(e);
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
EAPI void
evas_output_size_get(const Evas *e, int *w, int *h)
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
EAPI void
evas_output_viewport_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((x == e->viewport.x) && (y == e->viewport.y) &&
       (w == e->viewport.w) && (h == e->viewport.h)) return;
   if (w <= 0) return;
   if (h <= 0) return;
   if ((x != 0) || (y != 0))
     {
	printf("EVAS: compat error. viewport x,y != 0,0 not supported\n");
	x = 0;
	y = 0;
     }
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
EAPI void
evas_output_viewport_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
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
EAPI Evas_Coord
evas_coord_screen_x_to_world(const Evas *e, int x)
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
EAPI Evas_Coord
evas_coord_screen_y_to_world(const Evas *e, int y)
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
EAPI int
evas_coord_world_x_to_screen(const Evas *e, Evas_Coord x)
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
EAPI int
evas_coord_world_y_to_screen(const Evas *e, Evas_Coord y)
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
EAPI int
evas_render_method_lookup(const char *name)
{
   static int i = 1;
   Evas_Module *em;
   Evas_Module_Engine *eem;
   
   if (!name) return RENDER_METHOD_INVALID;
   /* search on the engines list for the name */
   em = evas_module_find_type(EVAS_MODULE_TYPE_ENGINE, name);
   
   if (!em) return RENDER_METHOD_INVALID;
   
   eem = (Evas_Module_Engine *)em->data;
   if (!eem)
     {
	eem = malloc(sizeof(Evas_Module_Engine));
	em->data = eem;
	eem->id = i;
	i++;
     }
   return eem->id;
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
 * Eina_List *engine_list, *l;
 * char *engine_name;
 *
 * engine_list = evas_render_method_list();
 * if (!engine_list)
 *   {
 *     fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
 *     exit(-1);
 *   }
 * printf("Availible Evas Engines:\n");
 * EINA_LIST_FOREACH(engine_list, l, engine_name)
 *     printf("%s\n", engine_name);
 * evas_render_method_list_free(engine_list);
 * @endcode
 */
EAPI Eina_List *
evas_render_method_list(void)
{
   Eina_List *methods = NULL;

   /* FIXME: get from modules - this is currently coded-in */
#ifdef BUILD_ENGINE_SOFTWARE_DDRAW
   methods = eina_list_append(methods, strdup("software_ddraw"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_16_DDRAW
   methods = eina_list_append(methods, strdup("software_16_ddraw"));
#endif
#ifdef BUILD_ENGINE_DIRECT3D
   methods = eina_list_append(methods, strdup("direct3d"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_16_WINCE
   methods = eina_list_append(methods, strdup("software_16_wince"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_X11
   methods = eina_list_append(methods, strdup("software_x11"));
#endif
#ifdef BUILD_ENGINE_XRENDER_X11
   methods = eina_list_append(methods, strdup("xrender_x11"));
#endif
#ifdef BUILD_ENGINE_XRENDER_XCB
   methods = eina_list_append(methods, strdup("xrender_xcb"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_16_X11
   methods = eina_list_append(methods, strdup("software_16_x11"));
#endif
#ifdef BUILD_ENGINE_GL_X11
   methods = eina_list_append(methods, strdup("gl_x11"));
#endif
#ifdef BUILD_ENGINE_GL_GLEW
   methods = eina_list_append(methods, strdup("gl_glew"));
#endif
#ifdef BUILD_ENGINE_CAIRO_X11
   methods = eina_list_append(methods, strdup("cairo_x11"));
#endif
#ifdef BUILD_ENGINE_DIRECTFB
   methods = eina_list_append(methods, strdup("directfb"));
#endif
#ifdef BUILD_ENGINE_FB
   methods = eina_list_append(methods, strdup("fb"));
#endif
#ifdef BUILD_ENGINE_BUFFER
   methods = eina_list_append(methods, strdup("buffer"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_WIN32_GDI
   methods = eina_list_append(methods, strdup("software_win32_gdi"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_QTOPIA
   methods = eina_list_append(methods, strdup("software_qtopia"));
#endif
#ifdef BUILD_ENGINE_SOFTWARE_SDL
   methods = eina_list_append(methods, strdup("software_sdl"));
#endif

   return methods;
}

/**
 * This function should be called to free a list of engine names
 *
 * @param list The Eina_List base pointer for the engine list to be freed
 * @ingroup Evas_Output_Method
 *
 * When this function is called it will free the engine list passed in as
 * @p list. The list should only be a list of engines generated by calling
 * evas_render_method_list(). If @p list is NULL, nothing will happen.
 *
 * Example:
 * @code
 * Eina_List *engine_list, *l;
 * char *engine_name;
 *
 * engine_list = evas_render_method_list();
 * if (!engine_list)
 *   {
 *     fprintf(stderr, "ERROR: Evas supports no engines! Exit.\n");
 *     exit(-1);
 *   }
 * printf("Availible Evas Engines:\n");
 * EINA_LIST_FOREACH(engine_list, l, engine_name)
 *     printf("%s\n", engine_name);
 * evas_render_method_list_free(engine_list);
 * @endcode
 */
EAPI void
evas_render_method_list_free(Eina_List *list)
{
   while (list)
     {
	free(list->data);
	list = eina_list_remove(list, list->data);
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
EAPI void
evas_pointer_output_xy_get(const Evas *e, int *x, int *y)
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
EAPI void
evas_pointer_canvas_xy_get(const Evas *e, Evas_Coord *x, Evas_Coord *y)
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
EAPI int
evas_pointer_button_down_mask_get(const Evas *e)
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
EAPI Evas_Bool
evas_pointer_inside_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return (int)e->pointer.inside;
}

/**
 * Attaches a specific pointer to the evas for fetching later
 * 
 * @param e The canvas to attach the pointer to
 * @param data The pointer to attach
 */
EAPI void
evas_data_attach_set(Evas *e, void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->attach_data = data;
}

/**
 * Returns the pointer attached by evas_data_attach_set()
 * 
 * @param e The canvas to attach the pointer to
 * @return The pointer attached
 */
EAPI void *
evas_data_attach_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->attach_data;
}

void
_evas_walk(Evas *e)
{
   e->walking_list++;
}

void
_evas_unwalk(Evas *e)
{
   e->walking_list--;
   if ((e->walking_list == 0) && (e->delete_me)) evas_free(e);
}

