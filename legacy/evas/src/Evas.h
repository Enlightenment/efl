#ifndef _EVAS_H 
#define _EVAS_H 1

#include <X11/Xlib.h>
#include <Imlib2.h>

/* types */
#ifndef _EVAS_PRIVATE_H
typedef void *                             Evas;
typedef void *                             Evas_Gradient;
typedef void *                             Evas_Object;
#endif
typedef struct _Evas_List *                Evas_List;
typedef struct _Evas_Point *               Evas_Point;

/* public structs */
struct _Evas_Point
{
   double x, y;
};

struct _Evas_List 
{
   Evas_List  prev, next;
   void      *data;
   /* private members - DONT TOUCH */
   Evas_List  last;
};

/* enums */
#define RENDER_METHOD_COUNT 5
enum _Evas_Render_Method
{
   RENDER_METHOD_ALPHA_SOFTWARE, /* imlib2 rendering to any drawable */
   RENDER_METHOD_BASIC_HARDWARE, /* X11 pixmap rendering to any drawable */
   RENDER_METHOD_3D_HARDWARE,    /* oepngl rendering to windows only */
   RENDER_METHOD_ALPHA_HARDWARE, /* unimplimented */
   RENDER_METHOD_IMAGE           /* imlib2 rendering to imlib image target */
};

enum _Evas_Callback_Type
{
   CALLBACK_MOUSE_IN,
   CALLBACK_MOUSE_OUT,
   CALLBACK_MOUSE_DOWN,
   CALLBACK_MOUSE_UP,
   CALLBACK_MOUSE_MOVE,
   CALLBACK_FREE
};

enum _Evas_Image_Format
{
   IMAGE_FORMAT_BGRA,
   IMAGE_FORMAT_ARGB,
   IMAGE_FORMAT_RGB,
   IMAGE_FORMAT_GRAY
};

/* enum types */
typedef enum _Evas_Callback_Type           Evas_Callback_Type;
typedef enum _Evas_Image_Format            Evas_Image_Format;
typedef enum _Evas_Render_Method           Evas_Render_Method;

/* functions */
#ifdef __cplusplus
extern "C" {
#endif
   
/* create and destroy */
Evas                evas_new_all(Display *display, Window parent_window,
				 int x, int y, int w, int h,
				 Evas_Render_Method render_method,
				 int colors, int font_cache, int image_cache,
				 char *font_dir);
Window              evas_get_window(Evas e);
Display            *evas_get_display(Evas e);
Visual             *evas_get_visual(Evas e);
Colormap            evas_get_colormap(Evas e);
int                 evas_get_colors(Evas e);
Imlib_Image         evas_get_image(Evas e);
Evas_Render_Method  evas_get_render_method(Evas e);
Evas                evas_new(void);
void                evas_free(Evas e);

/* for exposes or forced redraws (relative to output drawable) */
void evas_update_rect(Evas e, int x, int y, int w, int h);
/* for when the evas isnt fully visible you can clip out rects that are */
/* full obscured rects of the evas (ie windows ontop) */
void evas_add_obscured_rect(Evas e, int x, int y, int w, int h);
void evas_clear_obscured_rects(Evas e);

/* drawing */
Imlib_Updates evas_render_updates(Evas e);
void          evas_render(Evas e);

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

/* clipping - for now you can only use rectangles as clip objects */
void        evas_set_clip(Evas e, Evas_Object o, Evas_Object clip);
void        evas_unset_clip(Evas e, Evas_Object o);
Evas_Object evas_get_clip_object(Evas e, Evas_Object o);
Evas_List   evas_get_clip_list(Evas e, Evas_Object o);
       
/* deleting objects */
void evas_del_object(Evas e, Evas_Object o);

/* adding objects */
Evas_Object evas_add_image_from_file(Evas e, char *file);
/* ** ** not implimented yet ** ** */Evas_Object evas_add_image_from_data(Evas e, void *data, Evas_Image_Format format, int w, int h);
Evas_Object evas_add_text(Evas e, char *font, int size, char *text);
Evas_Object evas_add_rectangle(Evas e);
Evas_Object evas_add_line(Evas e);
Evas_Object evas_add_gradient_box(Evas e);
Evas_Object evas_add_poly(Evas e);
	 
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
void evas_add_point(Evas e, Evas_Object o, double x, double y);
void evas_clear_points(Evas e, Evas_Object o);
       
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
int evas_get_layer(Evas e, Evas_Object o);
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
Evas_Object evas_object_get_named(Evas e, char *name);
void evas_object_set_name(Evas e, Evas_Object o, char *name);   
char *evas_object_get_name(Evas e, Evas_Object o);
Evas_List evas_get_points(Evas e, Evas_Object o);

/* object visibility */
void evas_show(Evas e, Evas_Object o);
void evas_hide(Evas e, Evas_Object o);

/* image query ops */
int evas_get_image_alpha(Evas e, Evas_Object o);
void evas_get_image_size(Evas e, Evas_Object o, int *w, int *h);
void evas_get_image_border(Evas e, Evas_Object o, int *l, int *r, int *t, int *b);
Imlib_Load_Error evas_get_image_load_error(Evas e, Evas_Object o);

/* coordinate space transforms */
int evas_world_x_to_screen(Evas e, double x);
int evas_world_y_to_screen(Evas e, double y);
double evas_screen_x_to_world(Evas e, int x);
double evas_screen_y_to_world(Evas e, int y);
	 
/* text query ops */
char  *evas_get_text_string(Evas e, Evas_Object o);
char  *evas_get_text_font(Evas e, Evas_Object o);
int    evas_get_text_size(Evas e, Evas_Object o);
double evas_get_text_width(Evas e, Evas_Object o);
double evas_get_text_height(Evas e, Evas_Object o);
int    evas_text_at_position(Evas e, Evas_Object o, double x, double y, double *char_x, double *char_y, double *char_w, double *char_h);
void   evas_text_at(Evas e, Evas_Object o, int index, double *char_x, double *char_y, double *char_w, double *char_h);
void   evas_text_get_ascent_descent(Evas e, Evas_Object o, double *ascent, double *descent);
void   evas_text_get_max_ascent_descent(Evas e, Evas_Object o, double *ascent, double *descent);
void   evas_text_get_advance(Evas e, Evas_Object o, double *h_advance, double *v_advance);
double evas_text_get_inset(Evas e, Evas_Object o);
	 
/* object query ops */
void evas_get_color(Evas e, Evas_Object o, int *r, int *g, int *b, int *a);
Evas_Object evas_get_object_under_mouse(Evas e);
	 
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
int  evas_pointer_in(Evas e);
void evas_pointer_pos(Evas e, int *x, int *y);
int  evas_pointer_buttons(Evas e);
void evas_pointer_ungrab(Evas e);
   
/* callbacks */
void evas_callback_add(Evas e, Evas_Object o, Evas_Callback_Type callback, void (*func) (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y), void *data);
void evas_callback_del(Evas e, Evas_Object o, Evas_Callback_Type callback);

/* list ops */
Evas_List evas_list_append(Evas_List list, void *data);
Evas_List evas_list_prepend(Evas_List list, void *data);
Evas_List evas_list_append_relative(Evas_List list, void *data, void *relative);
Evas_List evas_list_prepend_relative(Evas_List list, void *data, void *relative);
Evas_List evas_list_remove(Evas_List list, void *data);
Evas_List evas_list_remove_list(Evas_List list, Evas_List remove_list);
void * evas_list_find(Evas_List list, void *data);
Evas_List evas_list_free(Evas_List list);
	
#ifdef __cplusplus
}
#endif

#endif


