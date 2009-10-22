#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <ApplicationServices/ApplicationServices.h>

/* log domain variable declared in evas_engine.c */
extern int _evas_engine_quartz_log_dom ;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_quartz_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_quartz_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_quartz_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_quartz_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_quartz_log_dom, __VA_ARGS__)

typedef struct _Evas_Quartz_Context Evas_Quartz_Context;

struct _Evas_Quartz_Context
{
   int         w, h;

   struct
   {
      double   r, g, b, a;
   } col;

   struct
   {
      double   r, g, b, a;
      int      set : 1;
   } mul;

   unsigned char aa, clipped;
};

typedef struct _Evas_Quartz_Polygon Evas_Quartz_Polygon;

struct _Evas_Quartz_Polygon
{
   Eina_List *points;
};

typedef struct _Evas_Quartz_Polygon_Point Evas_Quartz_Polygon_Point;

struct _Evas_Quartz_Polygon_Point
{
   int x, y;
};

typedef struct _Evas_Quartz_Image Evas_Quartz_Image;

struct _Evas_Quartz_Image
{
   RGBA_Image  *im;
   CGImageRef  cgim;

   int         references;
};

typedef struct _Evas_Quartz_Font Evas_Quartz_Font;

struct _Evas_Quartz_Font
{
   CTFontRef         font;
   CFDictionaryRef   attr;
   int               hint;
   int               size;
};

typedef struct _Evas_Quartz_Gradient Evas_Quartz_Gradient;

struct _Evas_Quartz_Gradient
{
   DATA32            *buf;
   RGBA_Gradient     *grad;
   Evas_Quartz_Image *im;
   unsigned char     changed : 1;
   int               sw, sh;
};

#endif
