#ifndef ECTOR_PRIVATE_H_
#define ECTOR_PRIVATE_H_

#include "Ector.h"

#include "ector_gl_internal.h"

/*
 * variable and macros used for the eina_log module
 */
extern int _ector_log_dom_global;

/*
 * Macros that are used everywhere
 *
 * the first four macros are the general macros for the lib
 */
#ifdef ECTOR_DEFAULT_LOG_COLOR
# undef ECTOR_DEFAULT_LOG_COLOR
#endif /* ifdef ECTOR_DEFAULT_LOG_COLOR */
#define ECTOR_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...)  EINA_LOG_DOM_ERR(_ector_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...)  EINA_LOG_DOM_DBG(_ector_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...)  EINA_LOG_DOM_INFO(_ector_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...)  EINA_LOG_DOM_WARN(_ector_log_dom_global, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif /* ifdef CRI */
#define CRI(...) EINA_LOG_DOM_CRIT(_ector_log_dom_global, __VA_ARGS__)

/* The following macro are internal to Ector only at this stage */

typedef struct _Ector_Renderer_Data Ector_Renderer_Data;
typedef struct _Ector_Renderer_Gradient_Data Ector_Renderer_Gradient_Data;
typedef struct _Ector_Renderer_Gradient_Linear_Data Ector_Renderer_Gradient_Linear_Data;
typedef struct _Ector_Renderer_Gradient_Radial_Data Ector_Renderer_Gradient_Radial_Data;
typedef struct _Ector_Renderer_Shape_Data Ector_Renderer_Shape_Data;
typedef struct _Ector_Renderer_Buffer_Data Ector_Renderer_Buffer_Data;

struct _Ector_Renderer_Data
{
   Ector_Surface *surface;
   Eina_Matrix3 *m;

   struct {
      double x;
      double y;
   } origin;

   struct {
      int r, g, b, a;
   } color;

   Eina_Bool visibility : 1;
   Eina_Bool finalized : 1;
};

struct _Ector_Renderer_Gradient_Data
{
   Efl_Gfx_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Gfx_Gradient_Spread s;
};

struct _Ector_Renderer_Gradient_Linear_Data
{
   struct {
      double x, y;
   } start, end;
};

struct _Ector_Renderer_Gradient_Radial_Data
{
   struct {
      double x, y;
   } radial, focal;
   double radius;
};

struct _Ector_Renderer_Shape_Data
{
   Ector_Renderer *fill;
   struct {
      Ector_Renderer *fill;
      Ector_Renderer *marker;
   } stroke;
};

struct _Ector_Renderer_Buffer_Data
{
   Ector_Buffer *eo_buffer;
   struct {
      /* Efl_Gfx_Fill_Spread spread; */
      int x, y, w, h;
   } fill;
};

static inline unsigned int
_renderer_crc_get(Eo *obj, unsigned int crc)
{
   unsigned int id;

   id = ector_renderer_crc_get(obj);
   crc = eina_crc((void*) &id, sizeof (id), crc, EINA_FALSE);
   return crc;
}

#include "ector_buffer.h"

#endif
