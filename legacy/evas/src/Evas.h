#ifndef _EVAS_H 
#define _EVAS_H 1

#include <X11/Xlib.h>
#include <Imlib2.h>

typedef struct _Evas *                     Evas;
typedef struct _Evas_Gradient *            Evas_Gradient;
typedef struct _Evas_Object_Any *          Evas_Object;
typedef struct _Evas_Object_Any *          Evas_Object_Any;
typedef int                                Evas_Callback_Type;
typedef int                                Evas_Image_Format;
typedef int                                Evas_Render_Method;
typedef struct _Evas_Render_Data           Evas_Render_Data;
typedef struct _Evas_List *                Evas_List;
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

#define RENDER_METHOD_ALPHA_SOFTWARE    0
#define RENDER_METHOD_BASIC_HARDWARE    1
#define RENDER_METHOD_3D_HARDWARE       2
#define RENDER_METHOD_ALPHA_HARDWARE    3
#define RENDER_METHOD_IMAGE             4
#define RENDER_METHOD_COUNT             5

#define CALLBACK_MOUSE_IN   0
#define CALLBACK_MOUSE_OUT  1
#define CALLBACK_MOUSE_DOWN 2
#define CALLBACK_MOUSE_UP   3
#define CALLBACK_MOUSE_MOVE 4
#define CALLBACK_FREE       5

#define IMAGE_FORMAT_BGRA   0
#define IMAGE_FORMAT_ARGB   1
#define IMAGE_FORMAT_RGB    2
#define IMAGE_FORMAT_GRAY   3

#define OBJECT_IMAGE        1230
#define OBJECT_TEXT         1231
#define OBJECT_RECTANGLE    1232
#define OBJECT_LINE         1233
#define OBJECT_GRADIENT_BOX 1234

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
};

struct _Evas_List
{
   Evas_List  prev, next;
   void      *data;
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
Visual   *evas_get_optimal_visual(Evas e, Display *disp);
Colormap  evas_get_optimal_colormap(Evas e, Display *disp);
void      evas_get_drawable_size(Evas e, int *w, int *h);
void      evas_get_viewport(Evas e, double *x, double *y, double *w, double *h);
   
/* the output settings */
void evas_set_output(Evas e, Display *disp, Drawable d, Visual *v, Colormap c);
void evas_set_output_image(Evas e, Imlib_Image image);
void evas_set_output_colors(Evas e, int colors);
void evas_set_output_size(Evas e, int w, int h);
void evas_set_output_viewport(Evas e, double x, double y, double w, double h);
void evas_set_output_method(Evas e, Evas_Render_Method method);
void evas_set_scale_smoothness(Evas e, int smooth);
	 
/* deleting objects */
void evas_del_object(Evas e, Evas_Object o);

/* adding objects */
Evas_Object evas_add_image_from_file(Evas e, char *file);
/* ** ** not implimented yet ** ** */Evas_Object evas_add_image_from_data(Evas e, void *data, Evas_Image_Format format, int w, int h);
Evas_Object evas_add_text(Evas e, char *font, int size, char *text);
Evas_Object evas_add_rectangle(Evas e);
Evas_Object evas_add_line(Evas e);
Evas_Object evas_add_gradient_box(Evas e);

/* set object settings */
void evas_set_image_file(Evas e, Evas_Object o, char *file);
/* ** ** not implimented yet ** ** */void evas_set_image_data(Evas e, Evas_Object o, void *data, Evas_Image_Format format, int w, int h);
void evas_set_image_fill(Evas e, Evas_Object o, double x, double y, double w, double h);
void evas_set_image_border(Evas e, Evas_Object o, int l, int r, int t, int b);
void evas_set_color(Evas e, Evas_Object o, int r, int g, int b, int a);
void evas_set_text(Evas e, Evas_Object o, char *text);
void evas_set_font(Evas e, Evas_Object o, char *font, int size);
void evas_set_gradient(Evas e, Evas_Object o, Evas_Gradient grad);
void evas_set_angle(Evas e, Evas_Object o, double angle);
void evas_set_zoom_scale(Evas e, Evas_Object o, int scale);
void evas_set_line_xy(Evas e, Evas_Object o, double x1, double y1, double x2, double y2);
void evas_set_pass_events(Evas e, Evas_Object o, int pass_events);

/* cache settings for performance */
void evas_set_font_cache(Evas e, int size);
int  evas_get_font_cache(Evas e);
void evas_flush_font_cache(Evas e);
void evas_set_image_cache(Evas e, int size);
int  evas_get_image_cache(Evas e);
void evas_flush_image_cache(Evas e);

/* font path */
void evas_font_add_path(Evas e, char *path);
void evas_font_del_path(Evas e, char *path);
   
/* layer stacking for object */
void evas_set_layer(Evas e, Evas_Object o, int l);
/* ** ** not implimented yet ** ** */void evas_set_layer_store(Evas e, int l, int store);

/* gradient creating / deletion / modification */
Evas_Gradient evas_gradient_new(void);
void evas_gradient_free(Evas_Gradient grad);
void evas_gradient_add_color(Evas_Gradient grad, int r, int g, int b, int a, int dist);

/* stacking within a layer */
void evas_raise(Evas e, Evas_Object o);
void evas_lower(Evas e, Evas_Object o);
void evas_stack_above(Evas e, Evas_Object o, Evas_Object above);
void evas_stack_below(Evas e, Evas_Object o, Evas_Object below);

/* object geometry */
void evas_move(Evas e, Evas_Object o, double x, double y);
void evas_resize(Evas e, Evas_Object o, double w, double h);
void evas_get_geometry(Evas e, Evas_Object o, double *x, double *y, double *w, double *h);

/* object query */
Evas_List evas_objects_in_rect(Evas e, double x, double y, double w, double h);
Evas_List evas_objects_at_position(Evas e, double x, double y);
Evas_Object evas_object_in_rect(Evas e, double x, double y, double w, double h);
Evas_Object evas_object_at_position(Evas e, double x, double y);
   
/* object visibility */
void evas_show(Evas e, Evas_Object o);
void evas_hide(Evas e, Evas_Object o);

/* image query ops */
void evas_get_image_size(Evas e, Evas_Object o, int *w, int *h);
void evas_get_image_border(Evas e, Evas_Object o, int *l, int *r, int *t, int *b);

/* coordinate space transforms */
int evas_world_x_to_screen(Evas e, double x);
int evas_world_y_to_screen(Evas e, double y);
double evas_screen_x_to_world(Evas e, int x);
double evas_screen_y_to_world(Evas e, int y);
	 
/* text query ops */
char  *evas_get_text_string(Evas e, Evas_Object o);
char  *evas_get_text_font(Evas e, Evas_Object o);
int    evas_get_text_size(Evas e, Evas_Object o);
int    evas_text_at_position(Evas e, Evas_Object o, double x, double y, int *char_x, int *char_y, int *char_w, int *char_h);
void   evas_text_at(Evas e, Evas_Object o, int index, int *char_x, int *char_y, int *char_w, int *char_h);
void   evas_text_get_ascent_descent(Evas e, Evas_Object o, double *ascent, double *descent);
void   evas_text_get_max_ascent_descent(Evas e, Evas_Object o, double *ascent, double *descent);
void   evas_text_get_advance(Evas e, Evas_Object o, double *h_advance, double *v_advance);
double evas_text_get_inset(Evas e, Evas_Object o);
	 
/* object query ops */
void evas_get_color(Evas e, Evas_Object o, int *r, int *g, int *b, int *a);
Evas_Object evas_get_object_under_mouse(Evas e);
Evas_Object evas_get_object_at_pos(Evas e, double x, double y);
	 
/* data attachment ops */
void evas_put_data(Evas e, Evas_Object o, char *key, void *data);
void *evas_get_data(Evas e, Evas_Object o, char *key);  
void *evas_remove_data(Evas e, Evas_Object o, char *key);
	 
/* events */
void evas_event_button_down(Evas e, int x, int y, int b);
void evas_event_button_up(Evas e, int x, int y, int b);
void evas_event_move(Evas e, int x, int y);
void evas_event_enter(Evas e);
void evas_event_leave(Evas e);

/* callbacks */
void evas_callback_add(Evas e, Evas_Object o, Evas_Callback_Type callback, void (*func) (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y), void *data);
void evas_callback_del(Evas e, Evas_Object o, Evas_Callback_Type callback);

/* list ops */
Evas_List evas_list_append(Evas_List list, void *data);
Evas_List evas_list_prepend(Evas_List list, void *data);
Evas_List evas_list_append_relative(Evas_List list, void *data, void *relative);
Evas_List evas_list_prepend_relative(Evas_List list, void *data, void *relative);
Evas_List evas_list_remove(Evas_List list, void *data);
void * evas_list_find(Evas_List list, void *data);
Evas_List evas_list_free(Evas_List list);
	
#ifdef __cplusplus
}
#endif

#endif


