#ifndef EVAS_CAIRO_COMMON_H
#define EVAS_CAIRO_COMMON_H

#include "evas_common.h"
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <cairo.h>
#include <cairo-xlib.h>

typedef struct _Evas_Cairo_Context                   Evas_Cairo_Context;
typedef struct _Evas_Cairo_Image                     Evas_Cairo_Image;
typedef struct _Evas_Cairo_Polygon                   Evas_Cairo_Polygon;
typedef struct _Evas_Cairo_Polygon_Point             Evas_Cairo_Polygon_Point;

struct _Evas_Cairo_Context
{
   cairo_t         *cairo;

   struct {
      double r, g, b, a;
   } col;
   struct {
      unsigned int set : 1;
      double r, g, b, a;
   } mul;
};

struct _Evas_Cairo_Image
{
   RGBA_Image         *im;
   int                 references;
   DATA32             *mulpix;
   cairo_surface_t    *surface;
   cairo_pattern_t    *pattern;
};

struct _Evas_Cairo_Polygon
{
   Eina_List *points;
};

struct _Evas_Cairo_Polygon_Point
{
   int x, y;
};

#endif
