#ifndef _EVAS_PRIVATE_H 
#define _EVAS_PRIVATE_H 1

#include <X11/Xlib.h>
#include <Imlib2.h>
#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif

typedef struct _Evas *                     Evas;
typedef struct _Evas_Gradient *            Evas_Gradient;
typedef struct _Evas_Object_Any *          Evas_Object;
typedef struct _Evas_Object_Any *          Evas_Object_Any;
typedef struct _Evas_Render_Data           Evas_Render_Data;
typedef struct _Evas_Data *                Evas_Data;
typedef struct _Evas_Layer *               Evas_Layer;
typedef struct _Evas_Color_Point *         Evas_Color_Point;
typedef struct _Evas_Callback *            Evas_Callback;
typedef struct _Evas_Rectangle *           Evas_Rectangle;
typedef struct _Evas_Object_Image *        Evas_Object_Image;
typedef struct _Evas_Object_Text *         Evas_Object_Text;
typedef struct _Evas_Object_Rectangle *    Evas_Object_Rectangle;
typedef struct _Evas_Object_Line *         Evas_Object_Line;
typedef struct _Evas_Object_Gradient_Box * Evas_Object_Gradient_Box;
typedef struct _Evas_Object_Poly *         Evas_Object_Poly;

#define OBJECT_IMAGE        1230
#define OBJECT_TEXT         1231
#define OBJECT_RECTANGLE    1232
#define OBJECT_LINE         1233
#define OBJECT_GRADIENT_BOX 1234
#define OBJECT_POLYGON      1235

#define IF_OBJ(_o, _t) if (((Evas_Object)_o)->type != _t)

#define INTERSECTS(x, y, w, h, xx, yy, ww, hh) \
((x < (xx + ww)) && \
(y < (yy + hh)) && \
((x + w) > xx) && \
((y + h) > yy))

#define CLIP_TO(_x, _y, _w, _h, _cx, _cy, _cw, _ch) \
{ \
  if (INTERSECTS(_x, _y, _w, _h, _cx, _cy, _cw, _ch)) \
    { \
      if (_x < _cx) \
	{ \
	  _w += _x - _cx; \
	  _x = _cx; \
	  if (_w < 0) _w = 0; \
	} \
      if ((_x + _w) > (_cx + _cw)) \
	_w = _cx + _cw - _x; \
      if (_y < _cy) \
	{ \
	  _h += _y - _cy; \
	  _y = _cy; \
	  if (_h < 0) _h = 0; \
	} \
      if ((_y + _h) > (_cy + _ch)) \
	_h = _cy + _ch - _y; \
    } \
  else \
    { \
      _w = 0; _h = 0; \
    } \
  }


#include "Evas.h"
struct _Evas_Render_Data
{
   int *method[RENDER_METHOD_COUNT];
};

struct _Evas
{
   struct  {
      Display      *display;
      Drawable      drawable;
      Visual       *visual;
      Colormap      colormap;
      unsigned char created_window;
      unsigned char screen;
      unsigned short colors;
      Imlib_Image   image;
      
      unsigned short drawable_width, drawable_height;
      
      struct  {
	 double     x, y, w, h;
      } __attribute__ ((packed)) viewport;
      
      struct {
	 double     mult_x, mult_y;
      } __attribute__ ((packed)) val_cache;
      
      Evas_Render_Method render_method;
      
      Evas_Render_Data renderer_data;
      
   }
   __attribute__ ((packed)) current, 
   __attribute__ ((packed)) previous;
   
   struct {
      unsigned char in;
      int x, y;
      int buttons;
      Evas_Object object, button_object;
   } __attribute__ ((packed)) mouse;
   
   
   void (*evas_renderer_data_free) (Evas _e);
   
   unsigned char changed;
   
   Evas_List     layers;
   Imlib_Updates updates;
   Imlib_Updates obscures;
} __attribute__ ((packed));

struct _Evas_Color_Point
{
   unsigned char r, g, b, a;
   int distance;
} __attribute__ ((packed));

struct _Evas_Gradient
{
   Evas_List  color_points;
   int        references;
} __attribute__ ((packed));

struct _Evas_Rectangle
{
   int x, y, w, h;
} __attribute__ ((packed));

struct _Evas_Data
{
   char *key;
   void *data;
} __attribute__ ((packed));

struct _Evas_Layer
{
   int        layer;
   Evas_List  objects;
   
   struct  {
      unsigned char        store;
   } __attribute__ ((packed)) current, __attribute__ ((packed)) previous;
   
   Evas_Render_Data renderer_data;
} __attribute__ ((packed));

struct _Evas_Callback
{
   Evas_Callback_Type type;
   void *data;
   void (*callback) (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
} __attribute__ ((packed));

struct _Evas_Object_Any
{
   unsigned char  marker;
   unsigned char  changed;
   unsigned short type;
   struct  {
      double     x, y, w, h;
      unsigned char zoomscale;
      int        layer;
      unsigned char visible;
      unsigned char stacking;
   } 
   __attribute__ ((packed)) current, 
   __attribute__ ((packed)) previous;

   unsigned char delete_me;
   
   unsigned char pass_events;
   
   void (*object_free) (Evas_Object _o);
   void (*object_renderer_data_free) (Evas _e, Evas_Object _o);
   
   Evas_List  callbacks;
   Evas_List  data;
   
   struct {
      Evas_List   list;
      Evas_Object object;
      unsigned char changed;
   } __attribute__ ((packed)) clip;
   
   Evas_Render_Data renderer_data;
   
   char *name;
} __attribute__ ((packed));

struct _Evas_Object_Image
{
   struct _Evas_Object_Any object;
   struct  {
      char *file;
      unsigned char   new_data;
      unsigned char   scale;
      unsigned char   alpha;
      struct {
	 unsigned short w, h;
      } __attribute__ ((packed)) image;
      struct {
	 double x, y, w, h;
      } __attribute__ ((packed)) fill;
      struct {
	 unsigned short l, r, t, b;
      } __attribute__ ((packed)) border;
      struct {
	 unsigned char r, g, b, a;
      } __attribute__ ((packed)) color;
   } current, previous;
   Imlib_Load_Error load_error;
} __attribute__ ((packed));

struct _Evas_Object_Text
{
   struct _Evas_Object_Any object;
   struct  {
      char *text;
      char *font;
      unsigned short   size;
      struct {
	 int w, h;
      } __attribute__ ((packed)) string;
      unsigned char r, g, b, a;
   } 
   __attribute__ ((packed)) current, 
   __attribute__ ((packed)) previous;
} __attribute__ ((packed));

struct _Evas_Object_Rectangle
{
   struct _Evas_Object_Any object;
   struct  {
      unsigned char r, g, b, a;
   } 
   __attribute__ ((packed)) current, 
   __attribute__ ((packed)) previous;
} __attribute__ ((packed));

struct _Evas_Object_Line
{
   struct _Evas_Object_Any object;
   struct  {
      double x1, y1, x2, y2;
      unsigned char r, g, b, a;
   } 
   __attribute__ ((packed)) current, 
   __attribute__ ((packed)) previous;
} __attribute__ ((packed));

struct _Evas_Object_Gradient_Box
{
   struct _Evas_Object_Any object;
   struct  {
      Evas_Gradient gradient;
      unsigned char    new_gradient;
      double angle;
   }
   __attribute__ ((packed)) current, 
   __attribute__ ((packed)) previous;
} __attribute__ ((packed));

struct _Evas_Object_Poly
{
   struct _Evas_Object_Any object;
   struct  {
      unsigned char r, g, b, a;
      Evas_List points;
   }
   __attribute__ ((packed)) current,
   __attribute__ ((packed)) previous;
} __attribute__ ((packed));

static void
_evas_get_current_clipped_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h)
{
   if (!o->current.visible)
     {
	*x = 0.0;
	*y = 0.0;
	*w = 0.0;
	*h = 0.0;
	return;
     }
   if (o->clip.object)
     _evas_get_current_clipped_geometry(e, o->clip.object, x, y, w, h);
   CLIP_TO(*x, *y, *w, *h, 
	   o->current.x, o->current.y, o->current.w, o->current.h);
}

static void
_evas_get_previous_clipped_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h)
{
   if (!o->previous.visible)
     {
	*x = 0.0;
	*y = 0.0;
	*w = 0.0;
	*h = 0.0;
	return;
     }
   if (o->clip.object)
     _evas_get_current_clipped_geometry(e, o->clip.object, x, y, w, h);
   CLIP_TO(*x, *y, *w, *h, 
	   o->previous.x, o->previous.y, o->previous.w, o->previous.h);
}

/* If it seems to be a string, try destringing it */
#define TO_OBJECT(e, obj)				\
	((obj) && ((char *)(obj))[0]			\
	 ? evas_object_get_named((e), (char *)(obj))	\
	 : (obj))
#endif


