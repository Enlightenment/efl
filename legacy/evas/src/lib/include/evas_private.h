#ifndef EVAS_PRIVATE_H
#define EVAS_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <eina_safety_checks.h>
#include "Evas.h"

#include "../file/evas_module.h"
#include "../file/evas_path.h"
#include "../engines/common/evas_text_utils.h"
#include "../engines/common/language/evas_bidi_utils.h"
#include "../engines/common/language/evas_language_utils.h"

#ifdef EVAS_MAGIC_DEBUG
/* complain when peole pass in wrong object types etc. */
# define MAGIC_DEBUG
#endif

#define RENDER_METHOD_INVALID            0x00000000

typedef struct _Evas_Layer                  Evas_Layer;
typedef struct _Evas_Size                   Evas_Size;
typedef struct _Evas_Aspect                 Evas_Aspect;
typedef struct _Evas_Border                 Evas_Border;
typedef struct _Evas_Double_Pair            Evas_Double_Pair;
typedef struct _Evas_Size_Hints             Evas_Size_Hints;
typedef struct _Evas_Font_Dir               Evas_Font_Dir;
typedef struct _Evas_Font                   Evas_Font;
typedef struct _Evas_Font_Alias             Evas_Font_Alias;
typedef struct _Evas_Data_Node              Evas_Data_Node;
typedef struct _Evas_Func_Node              Evas_Func_Node;
typedef RGBA_Image_Loadopts                 Evas_Image_Load_Opts;
typedef struct _Evas_Func                   Evas_Func;
typedef struct _Evas_Image_Load_Func        Evas_Image_Load_Func;
typedef struct _Evas_Image_Save_Func        Evas_Image_Save_Func;
typedef struct _Evas_Object_Func            Evas_Object_Func;
typedef struct _Evas_Intercept_Func         Evas_Intercept_Func;
typedef struct _Evas_Key_Grab               Evas_Key_Grab;
typedef struct _Evas_Callbacks              Evas_Callbacks;
typedef struct _Evas_Format                 Evas_Format;
typedef struct _Evas_Map_Point              Evas_Map_Point;
typedef struct _Evas_Smart_Cb_Description_Array Evas_Smart_Cb_Description_Array;
typedef struct _Evas_Post_Callback          Evas_Post_Callback;

/* General types - used for script type chceking */
#define OPAQUE_TYPE(type) struct __##type { int a; }; \
   typedef struct __##type type

OPAQUE_TYPE(Evas_Font_Set); /* General type for RGBA_Font */
OPAQUE_TYPE(Evas_Font_Instance); /* General type for RGBA_Font_Int */
/* End of general types */

#define MAGIC_EVAS                 0x70777770
#define MAGIC_OBJ                  0x71777770
#define MAGIC_OBJ_RECTANGLE        0x71777771
#define MAGIC_OBJ_LINE             0x71777772
#define MAGIC_OBJ_POLYGON          0x71777774
#define MAGIC_OBJ_IMAGE            0x71777775
#define MAGIC_OBJ_TEXT             0x71777776
#define MAGIC_OBJ_SMART            0x71777777
#define MAGIC_OBJ_TEXTBLOCK        0x71777778
#define MAGIC_SMART                0x72777770
#define MAGIC_OBJ_SHAPE            0x72777773
#define MAGIC_OBJ_CONTAINER        0x72777774
#define MAGIC_OBJ_CUSTOM           0x72777775
#define MAGIC_EVAS_GL              0x72777776

#ifdef MAGIC_DEBUG
# define MAGIC_CHECK_FAILED(o, t, m) \
{evas_debug_error(); \
 if (!o) evas_debug_input_null(); \
 else if (((t *)o)->magic == 0) evas_debug_magic_null(); \
 else evas_debug_magic_wrong((m), ((t *)o)->magic); \
}
# define MAGIC_CHECK(o, t, m) \
{if ((!o) || (!(((t *)o)->magic == (m)))) { \
MAGIC_CHECK_FAILED(o, t, m)
# define MAGIC_CHECK_END() }}
#else
# define MAGIC_CHECK_FAILED(o, t, m)
# define MAGIC_CHECK(o, t, m)  { if (!o) {
# define MAGIC_CHECK_END() }}
#endif

#define NEW_RECT(_r, _x, _y, _w, _h) (_r) = eina_rectangle_new(_x, _y, _w, _h);

#define MERR_NONE() _evas_alloc_error = EVAS_ALLOC_ERROR_NONE
#define MERR_FATAL() _evas_alloc_error = EVAS_ALLOC_ERROR_FATAL
#define MERR_BAD() _evas_alloc_error = EVAS_ALLOC_ERROR_RECOVERED

#define EVAS_OBJECT_IMAGE_FREE_FILE_AND_KEY(o)                              \
   if ((o)->cur.file)                                                       \
     {                                                                      \
        eina_stringshare_del((o)->cur.file);                                \
        if ((o)->prev.file == (o)->cur.file)                                \
          (o)->prev.file = NULL;                                            \
        (o)->cur.file = NULL;                                               \
     }                                                                      \
   if ((o)->cur.key)                                                        \
     {                                                                      \
        eina_stringshare_del((o)->cur.key);                                 \
        if ((o)->prev.key == (o)->cur.key)                                  \
          (o)->prev.key = NULL;                                             \
        (o)->cur.key = NULL;                                                \
     }                                                                      \
   if ((o)->prev.file)                                                      \
     {                                                                      \
        eina_stringshare_del((o)->prev.file);                               \
        (o)->prev.file = NULL;                                              \
     }                                                                      \
   if ((o)->prev.key)                                                       \
     {                                                                      \
        eina_stringshare_del((o)->prev.key);                                \
        (o)->prev.key = NULL;                                               \
     }

struct _Evas_Key_Grab
{
   char               *keyname;
   Evas_Modifier_Mask  modifiers;
   Evas_Modifier_Mask  not_modifiers;
   Evas_Object        *object;
   unsigned char       exclusive : 1;
   unsigned char       just_added : 1;
   unsigned char       delete_me : 1;
};

struct _Evas_Intercept_Func
{
   struct {
      Evas_Object_Intercept_Show_Cb func;
      void *data;
   } show;
   struct {
      Evas_Object_Intercept_Hide_Cb func;
      void *data;
   } hide;
   struct {
      Evas_Object_Intercept_Move_Cb func;
      void *data;
   } move;
   struct {
      Evas_Object_Intercept_Resize_Cb func;
      void *data;
   } resize;
   struct {
      Evas_Object_Intercept_Raise_Cb func;
      void *data;
   } raise;
   struct {
      Evas_Object_Intercept_Lower_Cb func;
      void *data;
   } lower;
   struct {
      Evas_Object_Intercept_Stack_Above_Cb func;
      void *data;
   } stack_above;
   struct {
      Evas_Object_Intercept_Stack_Below_Cb func;
      void *data;
   } stack_below;
   struct {
      Evas_Object_Intercept_Layer_Set_Cb func;
      void *data;
   } layer_set;
   struct {
      Evas_Object_Intercept_Color_Set_Cb func;
      void *data;
   } color_set;
   struct {
      Evas_Object_Intercept_Clip_Set_Cb func;
      void *data;
   } clip_set;
   struct {
      Evas_Object_Intercept_Clip_Unset_Cb func;
      void *data;
   } clip_unset;
};

struct _Evas_Smart_Cb_Description_Array
{
   unsigned int                      size;
   const Evas_Smart_Cb_Description **array;
};

struct _Evas_Smart
{
   DATA32            magic;

   int               usage;

   const Evas_Smart_Class *smart_class;

   Evas_Smart_Cb_Description_Array callbacks;

   unsigned char     delete_me : 1;
   unsigned char     class_allocated : 1;

};

struct _Evas_Modifier
{
   struct {
      int       count;
      char    **list;
   } mod;
   Evas_Modifier_Mask mask; /* ok we have a max of 64 modifiers */
};

struct _Evas_Lock
{
   struct {
      int       count;
      char    **list;
   } lock;
   Evas_Modifier_Mask mask; /* we have a max of 64 locks */
};

struct _Evas_Post_Callback
{
   Evas_Object               *obj;
   Evas_Object_Event_Post_Cb  func;
   const void                *data;
   unsigned char              delete_me : 1;
};

struct _Evas_Callbacks
{
   Eina_Inlist *callbacks;
   int               walking_list;
   unsigned char     deletions_waiting : 1;
};

struct _Evas
{
   EINA_INLIST;

   DATA32            magic;

   struct {
      unsigned char  inside : 1;
      int            mouse_grabbed;
      DATA32         button;
      Evas_Coord     x, y;
      struct {
          Eina_List *in;
      } object;

   } pointer;

   struct  {
      Evas_Coord     x, y, w, h;
      unsigned char  changed : 1;
   } viewport;

   struct {
      int            w, h;
      DATA32         render_method;
      unsigned char  changed : 1;
   } output;

   Eina_List        *damages;
   Eina_List        *obscures;

   Evas_Layer       *layers;

   Eina_Hash        *name_hash;

   int               output_validity;

   int               walking_list;
   int               events_frozen;

   struct {
      Evas_Module *module;
      Evas_Func *func;
      struct {
         void *output;

         void *context;
      } data;

      void *info;
      int   info_magic;
   } engine;

   Eina_Array     delete_objects;
   Eina_Array     active_objects;
   Eina_Array     restack_objects;
   Eina_Array     render_objects;
   Eina_Array     pending_objects;
   Eina_Array     obscuring_objects;
   Eina_Array     temporary_objects;
   Eina_Array     calculate_objects;
   Eina_Array     clip_changes;

   Eina_List     *post_events; // free me on evas_free

   Evas_Callbacks *callbacks;

   int            delete_grabs;
   int            walking_grabs;
   Eina_List     *grabs;

   Eina_List     *font_path;

   Evas_Object   *focused;
   void          *attach_data;
   Evas_Modifier  modifiers;
   Evas_Lock      locks;
   unsigned int   last_timestamp;
   int            last_mouse_down_counter;
   int            last_mouse_up_counter;
   int            nochange;
   Evas_Font_Hinting_Flags hinting;
   unsigned char  changed : 1;
   unsigned char  delete_me : 1;
   unsigned char  invalidate : 1;
   unsigned char  cleanup : 1;
   unsigned char  focus : 1;
};

struct _Evas_Layer
{
   EINA_INLIST;

   short             layer;
   Evas_Object      *objects;

   Evas             *evas;

   void             *engine_data;
   int               usage;
   unsigned char     delete_me : 1;
};

struct _Evas_Size
{
   Evas_Coord w, h;
};

struct _Evas_Aspect
{
   Evas_Aspect_Control mode;
   Evas_Size size;
};

struct _Evas_Border
{
   Evas_Coord l, r, t, b;
};

struct _Evas_Double_Pair
{
   double x, y;
};

struct _Evas_Size_Hints
{
   Evas_Size min, max, request;
   Evas_Aspect aspect;
   Evas_Double_Pair align, weight;
   Evas_Border padding;
};

struct _Evas_Map_Point
{
   double x, y, z, px, py;
   double u, v;
   unsigned char r, g, b, a;
};

struct _Evas_Map
{
   int                   count; // num of points
   Evas_Coord_Rectangle  normal_geometry; // bounding box of map geom actually
   void                 *surface; // surface holding map if needed
   int                   surface_w, surface_h; // current surface w & h alloc
   Evas_Coord            mx, my; // mouse x, y after conversion to map space
   struct {
      Evas_Coord         px, py, z0, foc;
   } persp;
   Eina_Bool             alpha : 1;
   Eina_Bool             smooth : 1;
   Evas_Map_Point        points[]; // actual points
};

#if 0 // filtering disabled
/* nash: Split into two bits */
typedef struct Evas_Filter_Info
{
   Evas_Filter filter;
   Evas_Filter_Mode mode;

   Eina_Bool dirty : 1;

   int datalen;
   void *data;
   void (*data_free)(void *);

   uint8_t *key;
   uint32_t len;
   Filtered_Image *cached;
} Evas_Filter_Info;

typedef Eina_Bool (*Evas_Software_Filter_Fn)(Evas_Filter_Info *, RGBA_Image *, RGBA_Image *);

int evas_filter_get_size(Evas_Filter_Info *info, int inw, int inh,
                     int *outw, int *outh, Eina_Bool inv);
Eina_Bool evas_filter_always_alpha(Evas_Filter_Info *info);
uint8_t *evas_filter_key_get(const Evas_Filter_Info *info, uint32_t *lenp);
// expose for use in engines
EAPI Evas_Software_Filter_Fn evas_filter_software_get(Evas_Filter_Info *info);
void evas_filter_free(Evas_Object *o);
#endif

struct _Evas_Object
{
   EINA_INLIST;

   DATA32                   magic;

   const char              *type;
   Evas_Layer              *layer;

   struct {
      Evas_Map             *map;
      Evas_Object          *clipper;
      Evas_Object          *mask;
      Evas_Object          *map_parent;
      double                scale;
      Evas_Coord_Rectangle  geometry;
      struct {
         struct {
            Evas_Coord      x, y, w, h;
            unsigned char   r, g, b, a;
            Eina_Bool       visible : 1;
            Eina_Bool       dirty : 1;
         } clip;
      } cache;
      short                 layer;
      struct {
         unsigned char      r, g, b, a;
      } color;
      Eina_Bool             usemap : 1;
      Eina_Bool             visible : 1;
      Eina_Bool             have_clipees : 1;
      Eina_Bool             anti_alias : 1;
      Evas_Render_Op        render_op : 4;
   } cur, prev;

   char                       *name;

   Evas_Intercept_Func        *interceptors;

   struct {
      Eina_List               *elements;
   } data;

   Eina_List                  *grabs;

   Evas_Callbacks             *callbacks;

   struct {
      Eina_List               *clipees;
      Eina_List               *changes;
   } clip;

   const Evas_Object_Func     *func;

   void                       *object_data;

   struct {
      Evas_Smart              *smart;
      Evas_Object             *parent;
   } smart;

   struct {
      Eina_List               *proxies;
      void                    *surface;
      int                      w,h;
      Eina_Bool                redraw;
   } proxy;

#if 0 // filtering disabled
   Evas_Filter_Info           *filter;
#endif

   Evas_Size_Hints            *size_hints;

   int                         last_mouse_down_counter;
   int                         last_mouse_up_counter;
   int                         mouse_grabbed;

   int                         last_event;

   struct {
        int                      in_move, in_resize;
   } doing;

   unsigned int                ref;

   unsigned char               delete_me;

   Evas_Object_Pointer_Mode    pointer_mode : 1;

   Eina_Bool                   store : 1;
   Eina_Bool                   pass_events : 1;
   Eina_Bool                   parent_pass_events : 1;
   Eina_Bool                   parent_cache_valid : 1;
   Eina_Bool                   repeat_events : 1;
   Eina_Bool                   restack : 1;
   Eina_Bool                   is_active : 1;
   Eina_Bool                   precise_is_inside : 1;
   Eina_Bool                   is_static_clip : 1;

   Eina_Bool                   render_pre : 1;
   Eina_Bool                   rect_del : 1;
   Eina_Bool                   mouse_in : 1;
   Eina_Bool                   pre_render_done : 1;
   Eina_Bool                   intercepted : 1;
   Eina_Bool                   focused : 1;
   Eina_Bool                   in_layer : 1;
   Eina_Bool                   no_propagate : 1;

   Eina_Bool                   changed : 1;
   Eina_Bool                   changed_move : 1;
   Eina_Bool                   changed_move_only : 1;
   Eina_Bool                   changed_nomove : 1;
   Eina_Bool                   del_ref : 1;
};

struct _Evas_Func_Node
{
   EINA_INLIST;
   void (*func) ();
   void *data;
   Evas_Callback_Type type;
   unsigned char delete_me : 1;
};

struct _Evas_Data_Node
{
   char *key;
   void *data;
};

struct _Evas_Font_Dir
{
   Eina_Hash *lookup;
   Eina_List *fonts;
   Eina_List *aliases;
   DATA64     dir_mod_time;
   DATA64     fonts_dir_mod_time;
   DATA64     fonts_alias_mod_time;
};

struct _Evas_Font
{
   struct {
      const char *prop[14];
   } x;
   struct {
      const char *name;
   } simple;
   const char *path;
   char     type;
};

struct _Evas_Font_Alias
{
   const char *alias;
   Evas_Font  *fn;
};

struct _Evas_Object_Func
{
   void (*free) (Evas_Object *obj);
   void (*render) (Evas_Object *obj, void *output, void *context, void *surface, int x, int y);
   void (*render_pre) (Evas_Object *obj);
   void (*render_post) (Evas_Object *obj);

   unsigned int  (*type_id_get) (Evas_Object *obj);
   unsigned int  (*visual_id_get) (Evas_Object *obj);
   void *(*engine_data_get) (Evas_Object *obj);

   void (*store) (Evas_Object *obj);
   void (*unstore) (Evas_Object *obj);

   int  (*is_visible) (Evas_Object *obj);
   int  (*was_visible) (Evas_Object *obj);

   int  (*is_opaque) (Evas_Object *obj);
   int  (*was_opaque) (Evas_Object *obj);

   int  (*is_inside) (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   int  (*was_inside) (Evas_Object *obj, Evas_Coord x, Evas_Coord y);

   void (*coords_recalc) (Evas_Object *obj);

   void (*scale_update) (Evas_Object *obj);

   int (*has_opaque_rect) (Evas_Object *obj);
   int (*get_opaque_rect) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

   int (*can_map) (Evas_Object *obj);
};

struct _Evas_Func
{
   void *(*info)                           (Evas *e);
   void (*info_free)                       (Evas *e, void *info);
   int  (*setup)                           (Evas *e, void *info);

   void (*output_free)                     (void *data);
   void (*output_resize)                   (void *data, int w, int h);
   void (*output_tile_size_set)            (void *data, int w, int h);
   void (*output_redraws_rect_add)         (void *data, int x, int y, int w, int h);
   void (*output_redraws_rect_del)         (void *data, int x, int y, int w, int h);
   void (*output_redraws_clear)            (void *data);
   void *(*output_redraws_next_update_get) (void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
   void (*output_redraws_next_update_push) (void *data, void *surface, int x, int y, int w, int h);
   void (*output_flush)                    (void *data);
   void (*output_idle_flush)               (void *data);
   void (*output_dump)                     (void *data);

   void *(*context_new)                    (void *data);
   Eina_Bool (*canvas_alpha_get)           (void *data, void *context);
   void (*context_free)                    (void *data, void *context);
   void (*context_clip_set)                (void *data, void *context, int x, int y, int w, int h);
   void (*context_clip_clip)               (void *data, void *context, int x, int y, int w, int h);
   void (*context_clip_unset)              (void *data, void *context);
   int  (*context_clip_get)                (void *data, void *context, int *x, int *y, int *w, int *h);
   void (*context_mask_set)                (void *data, void *context, void *mask, int x, int y, int w, int h);
   void (*context_mask_unset)              (void *data, void *context);
   void (*context_color_set)               (void *data, void *context, int r, int g, int b, int a);
   int  (*context_color_get)               (void *data, void *context, int *r, int *g, int *b, int *a);
   void (*context_multiplier_set)          (void *data, void *context, int r, int g, int b, int a);
   void (*context_multiplier_unset)        (void *data, void *context);
   int  (*context_multiplier_get)          (void *data, void *context, int *r, int *g, int *b, int *a);
   void (*context_cutout_add)              (void *data, void *context, int x, int y, int w, int h);
   void (*context_cutout_clear)            (void *data, void *context);
   void (*context_anti_alias_set)          (void *data, void *context, unsigned char aa);
   unsigned char (*context_anti_alias_get) (void *data, void *context);
   void (*context_color_interpolation_set) (void *data, void *context, int color_space);
   int  (*context_color_interpolation_get) (void *data, void *context);
   void (*context_render_op_set)           (void *data, void *context, int render_op);
   int  (*context_render_op_get)           (void *data, void *context);

   void (*rectangle_draw)                  (void *data, void *context, void *surface, int x, int y, int w, int h);

   void (*line_draw)                       (void *data, void *context, void *surface, int x1, int y1, int x2, int y2);

   void *(*polygon_point_add)              (void *data, void *context, void *polygon, int x, int y);
   void *(*polygon_points_clear)           (void *data, void *context, void *polygon);
   void (*polygon_draw)                    (void *data, void *context, void *surface, void *polygon, int x, int y);

   void *(*image_load)                     (void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo);
   void *(*image_new_from_data)            (void *data, int w, int h, DATA32 *image_data, int alpha, int cspace);
   void *(*image_new_from_copied_data)     (void *data, int w, int h, DATA32 *image_data, int alpha, int cspace);
   void (*image_free)                      (void *data, void *image);
   void (*image_size_get)                  (void *data, void *image, int *w, int *h);
   void *(*image_size_set)                 (void *data, void *image, int w, int h);
   void (*image_stride_get)                (void *data, void *image, int *stride);
   void *(*image_dirty_region)             (void *data, void *image, int x, int y, int w, int h);
   void *(*image_data_get)                 (void *data, void *image, int to_write, DATA32 **image_data, int *err);
   void *(*image_data_put)                 (void *data, void *image, DATA32 *image_data);
   void  (*image_data_preload_request)     (void *data, void *image, const void *target);
   void  (*image_data_preload_cancel)      (void *data, void *image, const void *target);
   void *(*image_alpha_set)                (void *data, void *image, int has_alpha);
   int  (*image_alpha_get)                 (void *data, void *image);
   void *(*image_border_set)               (void *data, void *image, int l, int r, int t, int b);
   void  (*image_border_get)               (void *data, void *image, int *l, int *r, int *t, int *b);
   void (*image_draw)                      (void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth);
   char *(*image_comment_get)              (void *data, void *image, char *key);
   char *(*image_format_get)               (void *data, void *image);
   void (*image_colorspace_set)            (void *data, void *image, int cspace);
   int  (*image_colorspace_get)            (void *data, void *image);
   void (*image_mask_create)               (void *data, void *image);
   void *(*image_native_set)               (void *data, void *image, void *native);
   void *(*image_native_get)               (void *data, void *image);

   void (*image_cache_flush)               (void *data);
   void (*image_cache_set)                 (void *data, int bytes);
   int  (*image_cache_get)                 (void *data);

   Evas_Font_Set *(*font_load)             (void *data, const char *name, int size, Font_Rend_Flags wanted_rend);
   Evas_Font_Set *(*font_memory_load)      (void *data, char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend);
   Evas_Font_Set *(*font_add)              (void *data, Evas_Font_Set *font, const char *name, int size, Font_Rend_Flags wanted_rend);
   Evas_Font_Set *(*font_memory_add)       (void *data, Evas_Font_Set *font, char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend);
   void (*font_free)                       (void *data, Evas_Font_Set *font);
   int  (*font_ascent_get)                 (void *data, Evas_Font_Set *font);
   int  (*font_descent_get)                (void *data, Evas_Font_Set *font);
   int  (*font_max_ascent_get)             (void *data, Evas_Font_Set *font);
   int  (*font_max_descent_get)            (void *data, Evas_Font_Set *font);
   void (*font_string_size_get)            (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int *w, int *h);
   int  (*font_inset_get)                  (void *data, Evas_Font_Set *font, const Evas_Text_Props *text_props);
   int  (*font_h_advance_get)              (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props);
   int  (*font_v_advance_get)              (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props);
   int  (*font_char_coords_get)            (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int pos, int *cx, int *cy, int *cw, int *ch);
   int  (*font_char_at_coords_get)         (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int x, int y, int *cx, int *cy, int *cw, int *ch);
   void (*font_draw)                       (void *data, void *context, void *surface, Evas_Font_Set *font, int x, int y, int w, int h, int ow, int oh, const Evas_Text_Props *intl_props);

   void (*font_cache_flush)                (void *data);
   void (*font_cache_set)                  (void *data, int bytes);
   int  (*font_cache_get)                  (void *data);

   /* Engine functions will over time expand from here */

   void (*font_hinting_set)                (void *data, Evas_Font_Set *font, int hinting);
   int  (*font_hinting_can_hint)           (void *data, int hinting);

/*    void (*image_rotation_set)              (void *data, void *image); */

   void (*image_scale_hint_set)            (void *data, void *image, int hint);
   int  (*image_scale_hint_get)            (void *data, void *image);
   int  (*font_last_up_to_pos)             (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int x, int y);

   void (*image_map_draw)                  (void *data, void *context, void *surface, void *image, int npoints, RGBA_Map_Point *p, int smooth, int level);
   void *(*image_map_surface_new)          (void *data, int w, int h, int alpha);
   void (*image_map_surface_free)          (void *data, void *surface);

   void (*image_content_hint_set)          (void *data, void *surface, int hint);
   int  (*image_content_hint_get)          (void *data, void *surface);
   int  (*font_pen_coords_get)             (void *data, Evas_Font_Set *font, const Evas_Text_Props *intl_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch);
   Eina_Bool (*font_text_props_info_create) (void *data __UNUSED__, Evas_Font_Instance *fi, const Eina_Unicode *text, Evas_Text_Props *intl_props, const Evas_BiDi_Paragraph_Props *par_props, size_t pos, size_t len);
   int  (*font_right_inset_get)            (void *data, Evas_Font_Set *font, const Evas_Text_Props *text_props);

#if 0 // filtering disabled
   void (*image_draw_filtered)             (void *data, void *context, void *surface, void *image, Evas_Filter_Info *filter);
   Filtered_Image *(*image_filtered_get)   (void *image, uint8_t *key, size_t len);
   Filtered_Image *(*image_filtered_save)  (void *image, void *filtered, uint8_t *key, size_t len);
   void (*image_filtered_free)             (void *image, Filtered_Image *);
#endif

   /* EFL-GL Glue Layer */
   void *(*gl_surface_create)            (void *data, void *config, int w, int h);
   int  (*gl_surface_destroy)            (void *data, void *surface);
   void *(*gl_context_create)            (void *data, void *share_context);
   int  (*gl_context_destroy)            (void *data, void *context);
   int  (*gl_make_current)               (void *data, void *surface, void *context);
   void *(*gl_proc_address_get)          (void *data, const char *name);
   int  (*gl_native_surface_get)         (void *data, void *surface, void *native_surface);
   void *(*gl_api_get)                   (void *data);
   int  (*image_load_error_get)          (void *data, void *image);
   int  (*font_run_end_get)              (void *data, Evas_Font_Set *font, Evas_Font_Instance **script_fi, Evas_Font_Instance **cur_fi, Evas_Script_Type script, const Eina_Unicode *text, int run_len);
};

struct _Evas_Image_Load_Func
{
  Eina_Bool threadable;
  Eina_Bool (*file_head) (Image_Entry *ie, const char *file, const char *key, int *error);
  Eina_Bool (*file_data) (Image_Entry *ie, const char *file, const char *key, int *error);
};

struct _Evas_Image_Save_Func
{
  int (*image_save) (RGBA_Image *im, const char *file, const char *key, int quality, int compress);
};

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *evas_object_new(Evas *e);
void evas_object_free(Evas_Object *obj, int clean_layer);
void evas_object_inject(Evas_Object *obj, Evas *e);
void evas_object_release(Evas_Object *obj, int clean_layer);
void evas_object_change(Evas_Object *obj);
void evas_object_clip_changes_clean(Evas_Object *obj);
void evas_object_render_pre_visible_change(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v);
void evas_object_render_pre_clipper_change(Eina_Array *rects, Evas_Object *obj);
void evas_object_render_pre_prev_cur_add(Eina_Array *rects, Evas_Object *obj);
void evas_object_render_pre_effect_updates(Eina_Array *rects, Evas_Object *obj, int is_v, int was_v);
void evas_rects_return_difference_rects(Eina_Array *rects, int x, int y, int w, int h, int xx, int yy, int ww, int hh);

void evas_object_clip_dirty(Evas_Object *obj);
void evas_object_recalc_clippees(Evas_Object *obj);
Evas_Layer *evas_layer_new(Evas *e);
void evas_layer_pre_free(Evas_Layer *lay);
void evas_layer_free_objects(Evas_Layer *lay);
void evas_layer_clean(Evas *e);
Evas_Layer *evas_layer_find(Evas *e, short layer_num);
void evas_layer_add(Evas_Layer *lay);
void evas_layer_del(Evas_Layer *lay);

int evas_object_was_in_output_rect(Evas_Object *obj, int x, int y, int w, int h);

int evas_object_was_opaque(Evas_Object *obj);
int evas_object_is_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
int evas_object_was_inside(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
int evas_object_clippers_was_visible(Evas_Object *obj);
void evas_object_clip_across_check(Evas_Object *obj);
void evas_object_clip_across_clippees_check(Evas_Object *obj);
void evas_object_mapped_clip_across_mark(Evas_Object *obj);
void evas_event_callback_call(Evas *e, Evas_Callback_Type type, void *event_info);
void evas_object_event_callback_call(Evas_Object *obj, Evas_Callback_Type type, void *event_info);
Eina_List *evas_event_objects_event_list(Evas *e, Evas_Object *stop, int x, int y);
int evas_mem_free(int mem_required);
int evas_mem_degrade(int mem_required);
void evas_debug_error(void);
void evas_debug_input_null(void);
void evas_debug_magic_null(void);
void evas_debug_magic_wrong(DATA32 expected, DATA32 supplied);
void evas_debug_generic(const char *str);
const char *evas_debug_magic_string_get(DATA32 magic);
void evas_object_smart_use(Evas_Smart *s);
void evas_object_smart_unuse(Evas_Smart *s);
void evas_smart_cb_descriptions_fix(Evas_Smart_Cb_Description_Array *a) EINA_ARG_NONNULL(1);
Eina_Bool evas_smart_cb_descriptions_resize(Evas_Smart_Cb_Description_Array *a, unsigned int size) EINA_ARG_NONNULL(1);
const Evas_Smart_Cb_Description *evas_smart_cb_description_find(const Evas_Smart_Cb_Description_Array *a, const char *name) EINA_ARG_NONNULL(1, 2) EINA_PURE;

Eina_Bool _evas_object_image_preloading_get(const Evas_Object *obj);
void _evas_object_image_preloading_set(Evas_Object *obj, Eina_Bool preloading);
void _evas_object_image_preloading_check(Evas_Object *obj);
void evas_object_smart_del(Evas_Object *obj);
void evas_object_smart_cleanup(Evas_Object *obj);
void evas_object_smart_member_raise(Evas_Object *member);
void evas_object_smart_member_lower(Evas_Object *member);
void evas_object_smart_member_stack_above(Evas_Object *member, Evas_Object *other);
void evas_object_smart_member_stack_below(Evas_Object *member, Evas_Object *other);
const Eina_Inlist *evas_object_smart_members_get_direct(const Evas_Object *obj);
void _evas_object_smart_members_all_del(Evas_Object *obj);
void evas_call_smarts_calculate(Evas *e);
void *evas_mem_calloc(int size);
void _evas_post_event_callback_call(Evas *e);
void _evas_post_event_callback_free(Evas *e);
void evas_event_callback_list_post_free(Eina_Inlist **list);
void evas_object_event_callback_all_del(Evas_Object *obj);
void evas_object_event_callback_cleanup(Evas_Object *obj);
void evas_event_callback_all_del(Evas *e);
void evas_event_callback_cleanup(Evas *e);
void evas_object_inform_call_show(Evas_Object *obj);
void evas_object_inform_call_hide(Evas_Object *obj);
void evas_object_inform_call_move(Evas_Object *obj);
void evas_object_inform_call_resize(Evas_Object *obj);
void evas_object_inform_call_restack(Evas_Object *obj);
void evas_object_inform_call_changed_size_hints(Evas_Object *obj);
void evas_object_inform_call_image_preloaded(Evas_Object *obj);
void evas_object_inform_call_image_unloaded(Evas_Object *obj);
void evas_object_intercept_cleanup(Evas_Object *obj);
int evas_object_intercept_call_show(Evas_Object *obj);
int evas_object_intercept_call_hide(Evas_Object *obj);
int evas_object_intercept_call_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
int evas_object_intercept_call_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
int evas_object_intercept_call_raise(Evas_Object *obj);
int evas_object_intercept_call_lower(Evas_Object *obj);
int evas_object_intercept_call_stack_above(Evas_Object *obj, Evas_Object *above);
int evas_object_intercept_call_stack_below(Evas_Object *obj, Evas_Object *below);
int evas_object_intercept_call_layer_set(Evas_Object *obj, int l);
int evas_object_intercept_call_color_set(Evas_Object *obj, int r, int g, int b, int a);
int evas_object_intercept_call_clip_set(Evas_Object *obj, Evas_Object *clip);
int evas_object_intercept_call_clip_unset(Evas_Object *obj);
void evas_object_grabs_cleanup(Evas_Object *obj);
void evas_key_grab_free(Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);
void evas_font_dir_cache_free(void);
const char *evas_font_dir_cache_find(char *dir, char *font);
Eina_List *evas_font_dir_available_list(const Evas* evas);
void evas_font_dir_available_list_free(Eina_List *available);
void evas_font_free(Evas *evas, void *font);
void evas_fonts_zero_free(Evas *evas);
void evas_fonts_zero_presure(Evas *evas);
void *evas_font_load(Evas *evas, const char *name, const char *source, int size);
void evas_font_load_hinting_set(Evas *evas, void *font, int hinting);
void evas_object_smart_member_cache_invalidate(Evas_Object *obj);
void evas_text_style_pad_get(Evas_Text_Style_Type style, int *l, int *r, int *t, int *b);
void _evas_object_text_rehint(Evas_Object *obj);
void _evas_object_textblock_rehint(Evas_Object *obj);

extern int _evas_alloc_error;
extern int _evas_event_counter;

struct _Evas_Imaging_Image
{
   RGBA_Image *image;
};

struct _Evas_Imaging_Font
{
   RGBA_Font *font;
};

int evas_async_events_init(void);
int evas_async_events_shutdown(void);
int evas_async_target_del(const void *target);

void _evas_preload_thread_init(void);
void _evas_preload_thread_shutdown(void);
Evas_Preload_Pthread *evas_preload_thread_run(void (*func_heavy)(void *data),
                                              void (*func_end)(void *data),
                                              void (*func_cancel)(void *data),
                                              const void *data);
Eina_Bool evas_preload_thread_cancel(Evas_Preload_Pthread *thread);

void _evas_walk(Evas *e);
void _evas_unwalk(Evas *e);

// expose for use in engines
EAPI int _evas_module_engine_inherit(Evas_Func *funcs, char *name);

void evas_render_invalidate(Evas *e);
void evas_render_object_recalc(Evas_Object *obj);

Eina_Bool evas_map_inside_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y);
Eina_Bool evas_map_coords_get(const Evas_Map *m, Evas_Coord x, Evas_Coord y, Evas_Coord *mx, Evas_Coord *my, int grab);

/****************************************************************************/
/*****************************************/
/********************/
#define MPOOL 1

#ifdef MPOOL 
typedef struct _Evas_Mempool Evas_Mempool;

struct _Evas_Mempool
{
  int           count;
  int           num_allocs;
  int           num_frees;
  Eina_Mempool *mp;
};
# define EVAS_MEMPOOL(x) \
   static Evas_Mempool x = {0, 0, 0, NULL}
# define EVAS_MEMPOOL_INIT(x, nam, siz, cnt, ret) \
   do { \
     if (!x.mp) { \
       x.mp = eina_mempool_add("chained_mempool", nam, NULL, sizeof(siz), cnt); \
       if (!x.mp) { \
         return ret; \
       } \
     } \
   } while (0)
# define EVAS_MEMPOOL_ALLOC(x, siz) \
   eina_mempool_malloc(x.mp, sizeof(siz))
# define EVAS_MEMPOOL_PREP(x, p, siz) \
   do { \
     x.count++; \
     x.num_allocs++; \
     memset(p, 0, sizeof(siz)); \
   } while (0)
# define EVAS_MEMPOOL_FREE(x, p) \
   do { \
     eina_mempool_free(x.mp, p); \
     x.count--; \
     x.num_frees++; \
     if (x.count <= 0) { \
       eina_mempool_del(x.mp); \
       x.mp = NULL; \
       x.count = 0; \
     } \
   } while (0)
#else
# define EVAS_MEMPOOL(x)
# define EVAS_MEMPOOL_INIT(x, nam, siz, cnt, ret)
# define EVAS_MEMPOOL_PREP(x, p, siz)
# define EVAS_MEMPOOL_ALLOC(x, siz) \
   calloc(1, sizeof(siz))
# define EVAS_MEMPOOL_FREE(x, p) \
   free(p)
#endif
/********************/
/*****************************************/
/****************************************************************************/

#define EVAS_API_OVERRIDE(func, api, prefix) \
     (api)->func = prefix##func
#define EVAS_API_RESET(func, api) \
     (api)->func = NULL

#include "evas_inline.x"

#ifdef __cplusplus
}
#endif
#endif
