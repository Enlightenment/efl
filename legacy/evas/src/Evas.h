#ifndef _EVAS_H 
#define _EVAS_H 1

#include <X11/Xlib.h>
#include <Imlib2.h>

typedef struct _Evas *                     Evas;
typedef struct _Evas_Gradient *            Evas_Gradient;
typedef struct _Evas_Object_Any *          Evas_Object;
typedef struct _Evas_Object_Any *          Evas_Object_Any;
typedef void *                             Evas_Group;
typedef int                                Evas_Callback_Type;
typedef int                                Evas_Image_Format;
typedef int                                Evas_Blend_Mode;
typedef struct _Evas_List *                Evas_List;
typedef struct _Evas_Layer *               Evas_Layer;
typedef struct _Evas_Color_Point *         Evas_Color_Point;
typedef struct _Evas_Object_Image *        Evas_Object_Image;
typedef struct _Evas_Object_Text *         Evas_Object_Text;
typedef struct _Evas_Object_Rectangle *    Evas_Object_Rectangle;
typedef struct _Evas_Object_Line *         Evas_Object_Line;
typedef struct _Evas_Object_Gradient_Box * Evas_Object_Gradient_Box;
typedef struct _Evas_Object_Bits *         Evas_Object_Bits;
typedef struct _Evas_Object_Evas *         Evas_Object_Evas;

#define RENDER_METHOD_ALPHA_SOFTWARE 0
#define RENDER_METHOD_BASIC_HARDWARE 1
#define RENDER_METHOD_ALPHA_HARDWARE 2

#define CALLBACK_MOUSE_IN   0
#define CALLBACK_MOUSE_OUT  1
#define CALLBACK_MOUSE_DOWN 2
#define CALLBACK_MOUSE_UP   3

#define IMAGE_FORMAT_BGRA   0
#define IMAGE_FORMAT_ARGB   1
#define IMAGE_FORMAT_RGB    2
#define IMAGE_FORMAT_GRAY   3

struct _Evas
{
   struct  {
      Display      *display;
      Drawable      drawable;
      Visual       *visual;
      Colormap      colormap;
      
      struct  {
	 int        x, y, w, h;
      } output;
      
      struct  {
	 double     x, y, w, h;
      } viewport;
      
      Evas_List    *layers;
      
      int           render_method;
      
      void      *renderer_data;
      
   } current, previous;
   
   /* externally provided updates for drawable relative rects */
   Evas_List    *updates;
};

struct _Evas_Color_Point
{
   int r, g, b, a;
   int distance;
};

struct _Evas_Gradient
{
   Evas_List *color_points;
};

struct _Evas_List
{
   Evas_List *prev, *next;
   void      *data;
};

struct _Evas_Layer
{
   int        layer;
   Evas_List *objects;
   Evas_List *groups;
   
   struct  {
      int        store;
   } current, previous;
   
   void      *renderer_data;
};

struct _Evas_Object_Any
{
   int        type;
   struct  {
      double     x, y, w, h;
      Evas_Blend_Mode mode;
      int        zoomscale;
   } current, previous;
   Evas_List *groups;
};

struct _Evas_Object_Image
{
   Evas_Object_Any object;
   struct  {
      char *file;
      double angle;
      struct _fill {
	 double x, y, w, h;
      } fill;
   } current, previous;
};

struct _Evas_Object_Text
{
   Evas_Object_Any object;
   struct  {
      char *text;
      double angle;
      int r, g, b, a;
   } current, previous;
};

struct _Evas_Object_Rectangle
{
   Evas_Object_Any object;
   struct  {
      int r, g, b, a;
   } current, previous;
};

struct _Evas_Object_Line
{
   Evas_Object_Any object;
   struct  {
      double x1, y1, x2, y2;
      int r, g, b, a;
   } current, previous;
};

struct _Evas_Object_Gradient_Box
{
   Evas_Object_Any object;
   struct  {
      Evas_Gradient gradient;
      double angle;
   } current, previous;
};

struct _Evas_Object_Bits
{
   Evas_Object_Any object;
   struct  {
      char *file;
   } current, previous;
};

struct _Evas_Object_Evas
{
   Evas_Object_Any object;
   struct  {
      Evas *evas;
   } current, previous;
};

#ifdef __cplusplus
extern "C" {
#endif
   
/* create and destroy */
Evas evas_new(void);
void evas_free(Evas e);

/* for exposes or forced redraws (relative to output drawable) */
void evas_update_rect(Evas e, int x, int y, int w, int h);

/* drawing */
void evas_render(Evas e);

/* query for settings to use */
Visual *evas_get_optimal_visual(Display *disp);
Colormap evas_get_optimal_colormap(Display *disp);

/* the output settings */
void evas_set_output(Evas e, Display *disp, Drawable d, Visual *v, Colormap c);
void evas_set_output_rect(Evas e, int x, int y, int w, int h);
void evas_set_viewport(Evas e, double x, double y, double w, double h);

/* deleting objects */
void evas_del_object(Evas e, Evas_Object o);

/* adding objects */
Evas_Object evas_add_image_from_file(Evas e, char *file);
Evas_Object evas_add_image_from_data(Evas e, void *data, Evas_Image_Format format, int w, int h);
Evas_Object evas_add_text(Evas e, char *font, int size, char *text);
Evas_Object evas_add_rectangle(Evas e, int r, Evas_Group g, int b);
Evas_Object evas_add_line(Evas e, int r, Evas_Group g, int b, int a);
Evas_Object evas_add_gradient_box(Evas e);
Evas_Object evas_add_bits(Evas e, char *file);
Evas_Object evas_add_evas(Evas e, Evas evas);

/* set object settings */
void evas_set_image_file(Evas e, Evas_Object o, char *file);
void evas_set_image_fill(Evas e, Evas_Object o, double x, double y, double w, double h);
void evas_set_bits_file(Evas e, Evas_Object o, char *file);
void evas_set_color(Evas e, Evas_Object o, int r, Evas_Group g, int b, int a);
void evas_set_gradient(Evas e, Evas_Object o, Evas_Gradient grad);
void evas_set_angle(Evas e, Evas_Object o, double angle);
void evas_set_blend_mode(Evas e, Evas_Blend_Mode mode);
void evas_set_zoom_scale(Evas e, Evas_Object o, int scale);
void evas_set_line_xy(Evas e, Evas_Object o, double x1, double y1, double x2, double y2);
   
/* layer stacking for object */
void evas_set_layer(Evas e, Evas_Object o, int l);
void evas_set_layer_store(Evas e, int l, int store);
   
/* gradient creating / deletion / modification */
Evas_Gradient evas_gradient_new(void);
void evas_gradient_free(Evas_Gradient grad);
void evas_gradient_add_color(Evas_Gradient grad, int r, int g, int b, int a, int dist);

/* stacking within a layer */
void evas_raise(Evas e, Evas_Object o);
void evas_lower(Evas e, Evas_Object o);
void evas_stack_above(Evas e, Evas_Object o, int above);
void evas_stack_below(Evas e, Evas_Object o, int above);

/* object geoemtry */
void evas_move(Evas e, Evas_Object o, double x, double y);
void evas_resize(Evas e, Evas_Object o, double w, double h);
void evas_get_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h);

/* object visibility */
void evas_show(Evas e, Evas_Object o);
void evas_hide(Evas e, Evas_Object o);

/* group operations */
Evas_Group evas_add_group(Evas e);
void evas_add_to_group(Evas e, Evas_Object o, Evas_Group g);
void evas_disband_group(Evas e, Evas_Group g);
void evas_free_group(Evas e, Evas_Group g);
void evas_del_from_group(Evas e, Evas_Object o, Evas_Group g);

/* evas bits ops */
void evas_bits_get_padding(Evas e, Evas_Object o, double *l, double *r, double *t, double *b);
void evas_bits_get_min(Evas e, Evas_Object o, double *w, double *h);
void evas_bits_get_max(Evas e, Evas_Object o, double *w, double *h);
void evas_bits_get_classed_bit_geoemtry(Evas e, Evas_Object o, char *class, double *x, double *y, double *w, double *h);

/* image query ops */
void evas_get_image_size(Evas e, Evas_Object o, int *w, int *h);

/* events */
void evas_event_button_down(Evas e, int x, int y, int b);
void evas_event_button_up(Evas e, int x, int y, int b);
void evas_event_move(Evas e, int x, int y);
void evas_event_enter(Evas e);
void evas_event_leave(Evas e);

/* callbacks */
void evas_callback_add(Evas e, Evas_Object o, Evas_Callback_Type callback, void (*func) (void *_data, Evas _e, char *_class, Evas_Object _o, int _b, int _x, int _y), void *data);
void evas_callback_del(Evas e, Evas_Object o, Evas_Callback_Type callback);

#ifdef __cplusplus
}
#endif

#endif


