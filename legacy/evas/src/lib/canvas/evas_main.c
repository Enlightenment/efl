#include "evas_common.h"
#include "evas_private.h"
#ifdef BUILD_ENGINE_SOFTWARE_X11
#include "evas_engine_api_software_x11.h"
#endif
#ifdef BUILD_ENGINE_DIRECTFB
#include "evas_engine_api_directfb.h"
#endif
#ifdef BUILD_ENGINE_FB
#include "evas_engine_api_fb.h"
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

#include "Evas.h"

/**
 * Allocate and new Evas empty canvas.
 * @return A zeroed out Evas, or NULL on failure
 * 
 * This function creates a new Evas Canvas object that is completely 
 * uninitialised. You cannot sensibly use this until it has been initialised.
 * You need to set up the rendering engine first for the canvas before you can
 * add any objects to the canvas or render it.
 * 
 * This function will always succeed and return a valid canvas pointer unless
 * the memory allocation fails, in which case NULL will be returned.
 * 
 * Example:
 * @code
 * Evas *evas;
 * 
 * evas = evas_new();
 * @endcode
 */
Evas *
evas_new(void)
{
   Evas *e;
   
   e = calloc(1, sizeof(Evas));
   if (!e) return NULL;

   e->magic = MAGIC_EVAS;
   e->output.render_method = RENDER_METHOD_INVALID;
   
   return e;
}

/**
 * Free an Evas.
 * @param e The Pointer to the Evas to be freed
 * 
 * When called this function frees the Evas Canvas @p e and All Evas Objects
 * created on this canvas. As the objects are freed if they have free callbacks
 * set, they will be called during the execution of this function.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * 
 * evas_free(evas);
 * @endcode
 */
void
evas_free(Evas *e)
{
   Evas_Object_List *l;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   for (l = (Evas_Object_List *)(e->layers); l; l = l->next)
     {
	Evas_Layer *lay;
	
	lay = (Evas_Layer *)l;
	evas_layer_pre_free(lay);
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

   e->engine.func->info_free(e, e->engine.info);
   e->engine.func->context_free(e->engine.data.output, e->engine.data.context);
   e->engine.func->output_free(e->engine.data.output);
   
   e->magic = 0;
   free(e);
}

/**
 * Set the output method (engine) for the Evas.
 * @param e The pointer to the Evas Canvas have its engine set
 * @param render_method The numeric engine value to use.
 * 
 * This call sets up which engine an Evas Canvas is to use from then on. This
 * can only be done once and following calls of this function once called once
 * will do nothing. The render method numeric value can be obtained using the
 * evas_render_method_lookup() call.
 * 
 * Example:
 * @code
 * int engine_id;
 * Evas *evas;
 * 
 * evas = evas_new();
 * engine_id = evas_render_method_lookup("software_x11");
 * evas_output_method_set(evas, engine_id);
 * @endcode
 */
void
evas_output_method_set(Evas *e, int render_method)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   
   /* if our engine to set it to is invalid - abort */
   if (render_method == RENDER_METHOD_INVALID) return;
   /* if the engine is alreayd set up - abort */
   if (e->output.render_method != RENDER_METHOD_INVALID) return;
   /* set the render method */
   e->output.render_method = render_method;
#ifdef BUILD_ENGINE_SOFTWARE_X11   
   if (e->output.render_method == RENDER_METHOD_SOFTWARE_X11)
     e->engine.func = &evas_engine_software_x11_func;
   else
#endif   
#ifdef BUILD_ENGINE_GL_X11
   if (e->output.render_method == RENDER_METHOD_GL_X11)
     e->engine.func = &evas_engine_gl_x11_func;
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
 * Get the output method (engine) for the Evas.
 * @param e The pointer to the Evas Canvas
 * @return A numeric engine value
 * 
 * This function returns the rendering engine currently used bu the given Evas
 * Canvas passed in. On success the rendering engine used by the Canvas is
 * returned. On failue 0 is returned.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * int engine_id;
 * 
 * engine_id = evas_output_method_get(evas);
 * 
 * @endcode
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
 * Get the render engine info struct from the canvas.
 * @param e The pointer to the Evas Canvas
 * @return A pointer to the Engine Info structure
 * 
 * Calling this function returns a pointer to a publicly modifyable structure
 * that the rendering engine for the given canvas has set up. Callin this
 * function before the rendering engine has been set will result in NULL being
 * returned, as will calling this function on an invalid canvas. The caller
 * does no need to free this structure and shoudl only assume that the pointer
 * tois is valid until evas_engine_info_set() is called or until evas_render()
 * is called. After these calls the contents of this structure and the pointer
 * to it are not guaranteed to be valid.
 * 
 * With some engines calling this call and modifying structure parameters,
 * then calling evas_engine_info_set() is used to modify engine parameters 
 * whilst the canvas is alive.
 * 
 * Example:
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
 * Set up the Engine Info from the info structure.
 * @param e The pointer to the Evas Canvas
 * @param info The pointer to the Engine Info to use
 * 
 * This function takes any parameters set in the provided info structure and
 * applies them to the rendering engine. You need to use the Engine Info
 * structure pointer provided by evas_engine_info_get() and not anything else.
 * 
 * You need to set up the Engine Info at least once before doing any rendering
 * or creating any objects on the Evas Canvas. Some engines support the 
 * changing of engine parameters during runtime and so the application
 * should call evas_engine_info_get(), modify parameters that need to be
 * modified, and then call evas_engine_info_set() again to have those changes
 * take effect.
 * 
 * Once evas_engine_info_set() is called the Engine Info structure pointer
 * should be considered invalid and should not be used again. Use 
 * evas_engine_info_get(0 to fetch it again if needed.
 * 
 * Example:
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
 * Set the render engine's output size in output units.
 * @param e The pointer to the Evas Canvas
 * @param w The width in output units
 * @param h The height in output units
 * 
 * This function sets the output display size for the Evas Canvas indicated to
 * be the size (for most engines in pixels). The Canvas will render to a
 * rectangle of this size on the output target once this call is called. This
 * is independant of the viewport (view into the canvas world) and will simply
 * stretch the viewport to fill the rectangle indicated by this call.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * 
 * evas_output_size_set(evas, 640, 480);
 * @endcode
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
 * Get the render engine's output size in output units.
 * @param e The pointer to the Evas Canvas
 * @param w The pointer to a width variable to be filled in
 * @param h The pointer to a height variable to be filled in
 * 
 * Calling this function writes the current canvas output size values into the
 * variables pointed to by @p w and @p h. On success the variables have the
 * output size values written to them in output units. If either @p w or @p h
 * is NULL, it will not be written to. If @p e is invalid, the results are
 * undefined.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * int width, height;
 * 
 * evas_output_size_get(evas, &width, &height);
 * @endcode
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
 * Set the render engine's output viewport in canvas units.
 * @param e The pointer to the Evas Canvas
 * @param x The top-left corner x value of the viewport
 * @param y The top-left corner y value of the viewport
 * @param w The width of the viewport
 * @param h The height of the viewport
 * 
 * This function sets the viewport (in canvas co-ordinate space) that will
 * be visible in the canvas ouput. The width and height of the viewport must
 * both be greater than 0. The rectangle described by the co-ordinates is the
 * rectangular region of the canvas co-ordinate space that is visibly mapped
 * and stretched to fill the output target of the canvas when rendering is
 * performed.
 * 
 * Co-ordinates do not have to map 1 to 1, but it is generally advised for 
 * ease of use that canvas co-ordinates to match output target units for 
 * better control, but this is not required.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * 
 * evas_output_viewport_set(evas, 0, 0, 640, 480);
 * @endcode
 */
void
evas_output_viewport_set(Evas *e, double x, double y, double w, double h)
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
 * 
 * Calling this function writes the current canvas output viewport size and
 * location values into the variables pointed to by @p x, @p y, @p w and @p h.
 * On success the variables have the output location and size values written
 * to them in canvas units. If either @p x, @p y, @p w or @p h are NULL, it
 * they will not be written to. If @p e is invalid, the results are
 * undefined.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * double x, y, width, height;
 * 
 * evas_output_viewport_get(evas, &x, &y, &w, &h);
 * @endcode
 */
void
evas_output_viewport_get(Evas *e, double *x, double *y, double *w, double *h)
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
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 * 
 * @param e The pointer to the Evas Canvas
 * @param x The screen/output x co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
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
 * double canvas_x;
 * 
 * canvas_x = evas_coord_screen_x_to_world(evas, screen_x);
 * @endcode
 */
double
evas_coord_screen_x_to_world(Evas *e, int x)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);   
   return 0;
   MAGIC_CHECK_END();
   return e->viewport.x + (((double)x * e->viewport.w) / (double)e->output.w);
}

/**
 * Convert/scale an ouput screen co-ordinate into canvas co-ordinates
 * 
 * @param e The pointer to the Evas Canvas
 * @param y The screen/output y co-ordinate
 * @return The screen co-ordinate translated to canvas unit co-ordinates
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
 * double canvas_y;
 * 
 * canvas_y = evas_coord_screen_y_to_world(evas, screen_y);
 * @endcode
 */
double
evas_coord_screen_y_to_world(Evas *e, int y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);   
   return 0;
   MAGIC_CHECK_END();
   return e->viewport.y + (((double)y * e->viewport.h) / (double)e->output.h);
}

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 * 
 * @param e The pointer to the Evas Canvas
 * @param x The canvas x co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
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
 * extern double canvas_x;
 * 
 * screen_x = evas_coord_world_x_to_screen(evas, canvas_x);
 * @endcode
 */
int
evas_coord_world_x_to_screen(Evas *e, double x)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);   
   return 0;
   MAGIC_CHECK_END();
   return (int)(((x - e->viewport.x) * (double)e->output.w) /  e->viewport.w);
}

/**
 * Convert/scale a canvas co-ordinate into output screen co-ordinates
 * 
 * @param e The pointer to the Evas Canvas
 * @param y The canvas y co-ordinate
 * @return The output/screen co-ordinate translated to output co-ordinates
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
 * extern double canvas_y;
 * 
 * screen_y = evas_coord_world_y_to_screen(evas, canvas_y);
 * @endcode
 */
int
evas_coord_world_y_to_screen(Evas *e, double y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);   
   return 0;
   MAGIC_CHECK_END();
   return (int)(((y - e->viewport.y) * (double)e->output.h) /  e->viewport.h);
}

/**
 * Look up a numeric ID from a string name of a rendering engine.
 * 
 * @param name The string name of an engine
 * @return A numeric (opaque) ID for the rendering engine
 * 
 * This function looks up a numeric return value for the named engine in the
 * string @p name. This is a normal C string, NUL byte terminated. The name
 * is case sensitive. If the rendering engine is available, a numeric ID for
 * that engine is returned that is no 0, if the engine is not available, 0
 * is returned, indicating an invalid engine.
 * 
 * The programmer should NEVER rely on the numeric ID an engine unless it is
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
#ifdef BUILD_ENGINE_GL_X11   
   if (!strcmp(name, "gl_x11")) return RENDER_METHOD_GL_X11;
#endif
#ifdef BUILD_ENGINE_DIRECTFB   
   if (!strcmp(name, "directfb"))     return RENDER_METHOD_DIRECTFB;
#endif
#ifdef BUILD_ENGINE_FB   
   if (!strcmp(name, "fb"))           return RENDER_METHOD_FB;
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
 * @return A linked lst whose data members are C strings of engine names
 * 
 * Calling this will return the program a handle (pointer) to an Evas linked
 * list. Each node in the linked lists will have the data pointer be a char *
 * pointer to the string name of the rendering engine available. The strings
 * should never be modified, neither should the list be modified. This list
 * should be cleaned up as soon as the program no longer needs it using
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
#ifdef BUILD_ENGINE_GL_X11   
   methods = evas_list_append(methods, strdup("gl_x11"));
#endif   
#ifdef BUILD_ENGINE_DIRECTFB   
   methods = evas_list_append(methods, strdup("directfb"));
#endif   
#ifdef BUILD_ENGINE_FB   
   methods = evas_list_append(methods, strdup("fb"));
#endif
#ifdef BBUILD_ENGINE_SOFTWARE_WIN32_GDI
   methods = evas_list_append(methods, strdup("software_win32_gdi"));
#endif   
#ifdef BBUILD_ENGINE_SOFTWARE_QTOPIA
   methods = evas_list_append(methods, strdup("software_qtopia"));
#endif   

   return NULL;
}

/**
 * This function should be called to free a list of engine names
 * 
 * @param list The Evas_List base pointer for the engine list to be freed
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
 * This function returns the current known pointer co-ordinates
 * 
 * @param e The pointer to the Evas Canvas
 * @param x The pointer to an integer to be filled in
 * @param y The pointer to an integer to be filled in
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
 * @param x The pointer to a double to be filled in
 * @param y The pointer to a double to be filled in
 * 
 * This function returns the current known canvas unit co-ordinates of the
 * mouse pointer and sets the contents of the doubles pointed to by @p x
 * and @p y to contain these co-ordinates. If @p e is not a valid canvas the
 * results of this function are undefined.
 * 
 * Example:
 * @code
 * extern Evas *evas;
 * double mouse_x, mouse_y;
 * 
 * evas_pointer_output_xy_get(evas, &mouse_x, &mouse_y);
 * printf("Mouse is at canvas position %f, %f\n", mouse_x, mouse_y);
 * @endcode
 */
void
evas_pointer_canvas_xy_get(Evas *e, double *x, double *y)
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
 * 
 * Calling this function will retunr a 32-bit integer with the appropriate bits
 * set to 1 that correspond to a mouse button being depressed. This limits
 * Evas to a mouse devices with a maximum of 32 buttons, but that is generally
 * in excess of any host systems pointing device abilities.
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
 * Returns if the mouse pointer is logically inside the canvas
 * 
 * @param e The pointer to the Evas Canvas
 * @return An integer (0 or 1) corresponding to the outside/inside pointer
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
 * else printf("Mouse is out\n");
 * @endcode
 */
int 
evas_pointer_inside_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return (int)e->pointer.inside;   
}
