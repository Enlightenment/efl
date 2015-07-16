#ifndef ECTOR_SOFTWARE_H_
#define ECTOR_SOFTWARE_H_

#include <Ector.h>

#ifndef _ECTOR_SOFTWARE_SURFACE_EO_CLASS_TYPE
#define _ECTOR_SOFTWARE_SURFACE_EO_CLASS_TYPE

typedef Eo Ector_Software_Surface;

#endif

typedef struct _Software_Rasterizer Software_Rasterizer;

#include "software/ector_software_surface.eo.h"
#include "software/ector_renderer_software_base.eo.h"
#include "software/ector_renderer_software_shape.eo.h"
#include "software/ector_renderer_software_gradient_linear.eo.h"
#include "software/ector_renderer_software_gradient_radial.eo.h"

#endif
