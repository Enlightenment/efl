#ifndef EFL_CANVAS_WL_H
# define EFL_CANVAS_WL_H
#include <Evas.h>
#include <Efl_Core.h>

#ifdef EAPI
# undef EAPI
#endif
#ifdef EAPI_WEAK
# undef EAPI_WEAK
#endif

# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#   define EAPI_WEAK
#  else
#   define EAPI
#   define EAPI_WEAK
#  endif
# endif

#define EWAPI EAPI EAPI_WEAK

#ifdef WAYLAND_UTIL_H
typedef struct wl_surface Efl_Canvas_Wl_Wl_Surface;
typedef struct wl_global Efl_Canvas_Wl_Wl_Global;
typedef struct wl_interface Efl_Canvas_Wl_Wl_Interface;
typedef struct wl_array Efl_Canvas_Wl_Wl_Array;
typedef void Efl_Canvas_Wl_Wl_Interface_Data;
typedef void Efl_Canvas_Wl_Wl_Interface_Bind_Cb;
#else
typedef struct Efl_Canvas_Wl_Wl_Surface Efl_Canvas_Wl_Wl_Surface;
typedef struct Efl_Canvas_Wl_Wl_Global Efl_Canvas_Wl_Wl_Global;
typedef struct Efl_Canvas_Wl_Wl_Interface Efl_Canvas_Wl_Wl_Interface;
typedef struct Efl_Canvas_Wl_Wl_Array Efl_Canvas_Wl_Wl_Array;
typedef void * Efl_Canvas_Wl_Wl_Interface_Data;
typedef void * Efl_Canvas_Wl_Wl_Interface_Bind_Cb;
#endif
#ifdef _XKBCOMMON_H_
typedef struct xkb_state Efl_Canvas_Wl_Xkb_State;
#else
typedef struct Efl_Canvas_Wl_Xkb_State Efl_Canvas_Wl_Xkb_State;
#endif
#include <efl_canvas_wl_surface.eo.h>
#include <efl_canvas_wl.eo.h>
/**
 * Get the Evas_Object for an extracted wl_surface resource created by an efl_canvas_wl object
 *
 * @note Passing anything other than a valid wl_surface resource from an efl_canvas_wl object will guarantee a crash.
 *
 * @param surface_resource The wl_resource for a wl_surface
 * @return The Evas_Object of the surface, NULL on failure
 * @since 1.24
 */
EAPI Evas_Object *efl_canvas_wl_extracted_surface_object_find(void *surface_resource);
#undef EAPI
#define EAPI
#undef EAPI_WEAK
#define EAPI_WEAK
#endif
