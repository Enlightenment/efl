#ifndef _EVAS_PRIVATE_H 
#define _EVAS_PRIVATE_H 1

#include <X11/Xlib.h>
#include <Imlib2.h>

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

#define OBJECT_IMAGE        1230
#define OBJECT_TEXT         1231
#define OBJECT_RECTANGLE    1232
#define OBJECT_LINE         1233
#define OBJECT_GRADIENT_BOX 1234

#define IF_OBJ(_o, _t) if (((Evas_Object)_o)->type != _t)

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
      int           created_window;
      int           screen;
      int           colors;
      Imlib_Image   image;
      
      int           drawable_width, drawable_height;
      
      struct  {
	 double     x, y, w, h;
      } viewport;
      
      Evas_Render_Method render_method;
      
      Evas_Render_Data renderer_data;
      
   } current, previous;
   
   struct {
      int in;
      int x, y;
      int buttons;
      Evas_Object object, button_object;
   } mouse;
   
   
   void (*evas_renderer_data_free) (Evas _e);
   
   int changed;
   
   Evas_List     layers;
   Imlib_Updates updates;
};

struct _Evas_Color_Point
{
   int r, g, b, a;
   int distance;
};

struct _Evas_Gradient
{
   Evas_List  color_points;
   int        references;
};

struct _Evas_Rectangle
{
   int x, y, w, h;
};

struct _Evas_Data
{
   char *key;
   void *data;
};

struct _Evas_Layer
{
   int        layer;
   Evas_List  objects;
   
   struct  {
      int        store;
   } current, previous;
   
   Evas_Render_Data renderer_data;
};

struct _Evas_Callback
{
   Evas_Callback_Type type;
   void *data;
   void (*callback) (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
};

struct _Evas_Object_Any
{
   int        type;
   struct  {
      double     x, y, w, h;
      int        zoomscale;
      int        layer;
      int        visible;
      int        stacking;
   } current, previous;

   int changed;
   
   int delete_me;
   
   int pass_events;
   
   void (*object_free) (Evas_Object _o);
   void (*object_renderer_data_free) (Evas _e, Evas_Object _o);
   
   Evas_List  callbacks;
   Evas_List  data;
   
   Evas_Render_Data renderer_data;
   
   char *name;
};

struct _Evas_Object_Image
{
   struct _Evas_Object_Any object;
   struct  {
      char *file;
      int   new_data;
      int   scale;
      struct {
	 int w, h;
      } image;
      struct {
	 double x, y, w, h;
      } fill;
      struct {
	 int l, r, t, b;
      } border;
      struct {
	 int r, g, b, a;
      } color;
   } current, previous;
};

struct _Evas_Object_Text
{
   struct _Evas_Object_Any object;
   struct  {
      char *text;
      char *font;
      int   size;
      struct {
	 int w, h;
      } string;
      int r, g, b, a;
   } current, previous;
};

struct _Evas_Object_Rectangle
{
   struct _Evas_Object_Any object;
   struct  {
      int r, g, b, a;
   } current, previous;
};

struct _Evas_Object_Line
{
   struct _Evas_Object_Any object;
   struct  {
      double x1, y1, x2, y2;
      int r, g, b, a;
   } current, previous;
};

struct _Evas_Object_Gradient_Box
{
   struct _Evas_Object_Any object;
   struct  {
      Evas_Gradient gradient;
      int    new_gradient;
      double angle;
   } current, previous;
};

#endif


