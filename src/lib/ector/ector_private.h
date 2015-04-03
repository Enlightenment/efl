#ifndef ECTOR_PRIVATE_H_
#define ECTOR_PRIVATE_H_

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

static inline void
_ector_renderer_replace(Ector_Renderer **d, const Ector_Renderer *s)
{
   Ector_Renderer *tmp = *d;

   *d = eo_ref(s);
   eo_unref(tmp);
}

typedef struct _Ector_Renderer_Generic_Base_Data Ector_Renderer_Generic_Base_Data;
typedef struct _Ector_Renderer_Generic_Gradient_Data Ector_Renderer_Generic_Gradient_Data;
typedef struct _Ector_Renderer_Generic_Gradient_Linear_Data Ector_Renderer_Generic_Gradient_Linear_Data;
typedef struct _Ector_Renderer_Generic_Gradient_Radial_Data Ector_Renderer_Generic_Gradient_Radial_Data;
typedef struct _Ector_Renderer_Generic_Shape_Data Ector_Renderer_Generic_Shape_Data;

struct _Ector_Renderer_Generic_Base_Data
{
   Eina_Matrix3 *m;

   struct {
      double x;
      double y;
   } origin;

   struct {
      int r, g, b, a;
   } color;

   Ector_Renderer *mask;

   Ector_Quality q;
   Eina_Bool visibility;
};

struct _Ector_Renderer_Generic_Gradient_Data
{
   Efl_Graphics_Gradient_Stop *colors;
   unsigned int colors_count;

   Efl_Graphics_Gradient_Spread s;
};

struct _Ector_Renderer_Generic_Gradient_Linear_Data
{
   struct {
      double x, y;
   } start, end;
};

struct _Ector_Renderer_Generic_Gradient_Radial_Data
{
   struct {
      double x, y;
   } radial, focal;
   double radius;
};

struct _Ector_Renderer_Generic_Shape_Data
{
   struct {
      Efl_Graphics_Path_Command *cmd;
      double *pts;
   } path;

   Ector_Renderer *fill;
   struct {
      Ector_Renderer *fill;
      Ector_Renderer *marker;

      double scale;
      double width;
      double centered;

      struct {
         int r, g, b, a;
      } color;

      Efl_Graphics_Dash *dash;
      unsigned int dash_length;

      Efl_Graphics_Cap cap;
      Efl_Graphics_Cap join;
   } stroke;
};

#endif
