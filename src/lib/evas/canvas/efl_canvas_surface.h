#define EFL_CANVAS_SURFACE_PROTECTED

#include "evas_image_private.h"
#include "efl_canvas_surface.eo.h"
#include "efl_canvas_surface_tbm.eo.h"
#include "efl_canvas_surface_x11.eo.h"
#include "efl_canvas_surface_wayland.eo.h"

typedef struct _Efl_Canvas_Surface_Data
{
   Evas_Native_Surface surf;
   void *buffer;
} Efl_Canvas_Surface_Data;
