#ifndef _EVAS_H 
#define _EVAS_H

#ifndef EVAS_COMMON_H
#ifndef EVAS_PRIVATE_H

enum _Evas_Callback_Type
{
   EVAS_CALLBACK_MOUSE_IN, /**< Mouse In Event */
   EVAS_CALLBACK_MOUSE_OUT, /**< Mouse Out Event */
   EVAS_CALLBACK_MOUSE_DOWN, /**< Mouse Button Down Event */
   EVAS_CALLBACK_MOUSE_UP, /**< Mouse Button Up Event */
   EVAS_CALLBACK_MOUSE_MOVE, /**< Mouse Move Event */
   EVAS_CALLBACK_FREE, /**< Object Being Freed */
   EVAS_CALLBACK_KEY_DOWN, /**< Key Press Event */
   EVAS_CALLBACK_KEY_UP, /**< Key Release Event */
   EVAS_CALLBACK_FOCUS_IN, /**< Focus In Event */
   EVAS_CALLBACK_FOCUS_OUT /**< Focus Out Event */
};

/**
 * The type of event to trigger the callback
 */
typedef enum _Evas_Callback_Type Evas_Callback_Type;  

/**
 * A generic linked list node handle
 */
typedef struct _Evas_List             Evas_List;
/**
 * A generic rectangle handle
 */
typedef struct _Evas_Rectangle        Evas_Rectangle;

struct _Evas_List
{
   void      *data; /**< Pointer to list element payload */
   Evas_List *next; /**< Next member in the list */
   Evas_List *prev; /**< Previous member in the list */
   
   Evas_List *last; /**< Private member. Don't use this */
   int        count; /**< Private member. Don't use this */
};

struct _Evas_Rectangle
{
   int x, y, w, h; /**< the co-ordinates of the rectangle, starting top-left and width and height */
};

/** A Hash table handle */
typedef void Evas_Hash;
/** An Evas canvas handle */
typedef void Evas;
/** An Evas Object handle */
typedef void Evas_Object;
/** An Evas Performance handle */
typedef void Evas_Performance;
/** An Evas Key */
typedef void Evas_Key;
/** An Evas Modifier */
typedef void Evas_Modifier;
/** An Evas Lock */
typedef void Evas_Lock;
/** An Evas Smart Object handle */
typedef void Evas_Smart;
#endif
#endif

/** A generic Evas Engine information structure */
typedef struct _Evas_Engine_Info      Evas_Engine_Info;
/** Event structure for #EVAS_CALLBACK_MOUSE_DOWN event callbacks */
typedef struct _Evas_Event_Mouse_Down Evas_Event_Mouse_Down;
/** Event structure for #EVAS_CALLBACK_MOUSE_UP event callbacks */
typedef struct _Evas_Event_Mouse_Up   Evas_Event_Mouse_Up;
/** Event structure for #EVAS_CALLBACK_MOUSE_IN event callbacks */
typedef struct _Evas_Event_Mouse_In   Evas_Event_Mouse_In;
/** Event structure for #EVAS_CALLBACK_MOUSE_OUT event callbacks */
typedef struct _Evas_Event_Mouse_Out  Evas_Event_Mouse_Out;
/** Event structure for #EVAS_CALLBACK_MOUSE_MOVE event callbacks */
typedef struct _Evas_Event_Mouse_Move Evas_Event_Mouse_Move;
/** Event structure for #EVAS_CALLBACK_KEY_DOWN event callbacks */
typedef struct _Evas_Event_Key_Down   Evas_Event_Key_Down;
/** Event structure for #EVAS_CALLBACK_KEY_UP event callbacks */
typedef struct _Evas_Event_Key_Up     Evas_Event_Key_Up;

#define EVAS_LOAD_ERROR_NONE                       0
#define EVAS_LOAD_ERROR_GENERIC                    1
#define EVAS_LOAD_ERROR_DOES_NOT_EXIST             2
#define EVAS_LOAD_ERROR_PERMISSION_DENIED          3
#define EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED 4
#define EVAS_LOAD_ERROR_CORRUPT_FILE               5
#define EVAS_LOAD_ERROR_UNKNOWN_FORMAT             6

struct _Evas_Engine_Info
{
   int magic;
};

struct _Evas_Event_Mouse_Down
{
   int button;
   struct {
      int x, y;
   } output;
   struct {
      double x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
};

struct _Evas_Event_Mouse_Up
{
   int button;
   struct {
      int x, y;
   } output;
   struct {
      double x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
};

struct _Evas_Event_Mouse_In
{
   int buttons;
   struct {
      int x, y;
   } output;
   struct {
      double x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
};

struct _Evas_Event_Mouse_Out
{
   int buttons;
   struct {
      int x, y;
   } output;
   struct {
      double x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
};

struct _Evas_Event_Mouse_Move
{
   int buttons;
   struct {
      struct {
	 int x, y;
      } output;
      struct {
	 double x, y;
      } canvas;
   } cur, prev;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
};

struct _Evas_Event_Key_Down
{
   Evas_Key      *key;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
};

struct _Evas_Event_Key_Up
{
   Evas_Key      *key;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
};

#ifdef __cplusplus
extern "C" {
#endif

   Evas_List        *evas_list_append                  (Evas_List *list, void *data);
   Evas_List        *evas_list_prepend                 (Evas_List *list, void *data);
   Evas_List        *evas_list_append_relative         (Evas_List *list, void *data, void *relative);
   Evas_List        *evas_list_prepend_relative        (Evas_List *list, void *data, void *relative);
   Evas_List        *evas_list_remove                  (Evas_List *list, void *data);
   Evas_List        *evas_list_remove_list             (Evas_List *list, Evas_List *remove_list);
   void             *evas_list_find                    (Evas_List *list, void *data);
   Evas_List        *evas_list_find_list               (Evas_List *list, void *data);
   Evas_List        *evas_list_free                    (Evas_List *list);
   Evas_List        *evas_list_last                    (Evas_List *list);
   Evas_List        *evas_list_next                    (Evas_List *list);
   Evas_List        *evas_list_prev                    (Evas_List *list);
   void             *evas_list_data                    (Evas_List *list);
   int               evas_list_count                   (Evas_List *list);
   void             *evas_list_nth                     (Evas_List *list, int n);
   Evas_List        *evas_list_nth_list                (Evas_List *list, int n);
   Evas_List        *evas_list_reverse                 (Evas_List *list);
   int               evas_list_alloc_error             (void);
       
   /* FIXME: add:
    * api to add find, del members by data, size not just string and also
    * provide hash generation functions settable by the app
    * 
    * do we really need this? hmmm - let me think... there may be a better way
    */
   Evas_Hash        *evas_hash_add                     (Evas_Hash *hash, const char *key, void *data);
   Evas_Hash        *evas_hash_del                     (Evas_Hash *hash, const char *key, void *data);
   void             *evas_hash_find                    (Evas_Hash *hash, const char *key);
   int               evas_hash_size                    (Evas_Hash *hash);
   void              evas_hash_free                    (Evas_Hash *hash);
   void              evas_hash_foreach                 (Evas_Hash *hash, int (*func) (Evas_Hash *hash, const char *key, void *data, void *fdata), void *fdata);
   int               evas_hash_alloc_error             (void);
   
   Evas             *evas_new                          (void);
   void              evas_free                         (Evas *e);

   int               evas_render_method_lookup         (const char *name);
   Evas_List        *evas_render_method_list           (void);
   void              evas_render_method_list_free      (Evas_List *list);
   
   void              evas_output_method_set            (Evas *e, int render_method);
   int               evas_output_method_get            (Evas *e);
   
   Evas_Engine_Info *evas_engine_info_get              (Evas *e);
   void              evas_engine_info_set              (Evas *e, Evas_Engine_Info *info);
   
   void              evas_output_size_set              (Evas *e, int w, int h);
   void              evas_output_size_get              (Evas *e, int *w, int *h);
   void              evas_output_viewport_set          (Evas *e, double x, double y, double w, double h);
   void              evas_output_viewport_get          (Evas *e, double *x, double *y, double *w, double *h);
   
   double            evas_coord_screen_x_to_world      (Evas *e, int x);
   double            evas_coord_screen_y_to_world      (Evas *e, int y);   
   int               evas_coord_world_x_to_screen      (Evas *e, double x);
   int               evas_coord_world_y_to_screen      (Evas *e, double y);
       
   void              evas_damage_rectangle_add         (Evas *e, int x, int y, int w, int h);
   void              evas_obscured_rectangle_add       (Evas *e, int x, int y, int w, int h);
   void              evas_obscured_clear               (Evas *e);
   Evas_List        *evas_render_updates               (Evas *e);
   void              evas_render_updates_free          (Evas_List *updates);
   void              evas_render                       (Evas *e);
      
   Evas_Object      *evas_object_rectangle_add         (Evas *e);
   
   Evas_Object      *evas_object_line_add              (Evas *e);
   void              evas_object_line_xy_set           (Evas_Object *obj, double x1, double y1, double x2, double y2);
   void              evas_object_line_xy_get           (Evas_Object *obj, double *x1, double *y1, double *x2, double *y2);
   
   Evas_Object      *evas_object_gradient_add          (Evas *e);
   void              evas_object_gradient_color_add    (Evas_Object *obj, int r, int g, int b, int a, int distance);
   void              evas_object_gradient_colors_clear (Evas_Object *obj);
   void              evas_object_gradient_angle_set    (Evas_Object *obj, double angle);
   double            evas_object_gradient_angle_get    (Evas_Object *obj);
   
   Evas_Object      *evas_object_polygon_add           (Evas *e);
   void              evas_object_polygon_point_add     (Evas_Object *obj, double x, double y);
   void              evas_object_polygon_points_clear  (Evas_Object *obj);
   
   Evas_Object      *evas_object_image_add             (Evas *e);       
   void              evas_object_image_file_set        (Evas_Object *obj, char *file, char *key);
   void              evas_object_image_file_get        (Evas_Object *obj, char **file, char **key);
   void              evas_object_image_border_set      (Evas_Object *obj, int l, int r, int t, int b);
   void              evas_object_image_border_get      (Evas_Object *obj, int *l, int *r, int *t, int *b);
   void              evas_object_image_fill_set        (Evas_Object *obj, double x, double y, double w, double h);
   void              evas_object_image_fill_get        (Evas_Object *obj, double *x, double *y, double *w, double *h);
   void              evas_object_image_size_set        (Evas_Object *obj, int w, int h);
   void              evas_object_image_size_get        (Evas_Object *obj, int *w, int *h);
   int               evas_object_image_load_error_get  (Evas_Object *obj);
   void              evas_object_image_data_set        (Evas_Object *obj, int *data);
   int              *evas_object_image_data_get        (Evas_Object *obj, int for_writing);
   void              evas_object_image_data_copy_set   (Evas_Object *obj, int *data);
   void              evas_object_image_data_update_add (Evas_Object *obj, int x, int y, int w, int h);
   void              evas_object_image_alpha_set       (Evas_Object *obj, int has_alpha);
   int               evas_object_image_alpha_get       (Evas_Object *obj);
   void              evas_object_image_smooth_scale_set(Evas_Object *obj, int smooth_scale);
   int               evas_object_image_smooth_scale_get(Evas_Object *obj);
   void              evas_object_image_reload          (Evas_Object *obj);
       
   void              evas_object_image_cache_flush     (Evas *e);
   void              evas_object_image_cache_reload    (Evas *e);
   void              evas_object_image_cache_set       (Evas *e, int size);
   int               evas_object_image_cache_get       (Evas *e);
   
   /* FIXME: add:
    * multiple image format handler code (clean internal api)
    * image loader (provided by app) code (callbacks)
    * loadable image loader module query etc. code 
    */
   
   Evas_Object      *evas_object_text_add              (Evas *e);
   void              evas_object_text_font_set         (Evas_Object *obj, char *font, double size);
   void              evas_object_text_font_get         (Evas_Object *obj, char **font, double *size);
   void              evas_object_text_text_set         (Evas_Object *obj, char *text);
   char             *evas_object_text_text_get         (Evas_Object *obj);
   double            evas_object_text_ascent_get       (Evas_Object *obj);
   double            evas_object_text_descent_get      (Evas_Object *obj);
   double            evas_object_text_max_ascent_get   (Evas_Object *obj);
   double            evas_object_text_max_descent_get  (Evas_Object *obj);
   double            evas_object_text_horiz_advance_get(Evas_Object *obj);
   double            evas_object_text_vert_advance_get (Evas_Object *obj);
   double            evas_object_text_inset_get        (Evas_Object *obj);
   double            evas_object_text_horiz_advance_get(Evas_Object *obj);
   double            evas_object_text_vert_advance_get (Evas_Object *obj);
   int               evas_object_text_char_pos_get     (Evas_Object *obj, int pos, double *cx, double *cy, double *cw, double *ch);
   int               evas_object_text_char_coords_get  (Evas_Object *obj, double x, double y, double *cx, double *cy, double *cw, double *ch);

   /* FIXME: add:
    * text outline & shadowing code
    */
   
   void              evas_object_font_path_clear       (Evas *e);
   void              evas_object_font_path_append      (Evas *e, char *path);
   void              evas_object_font_path_prepend     (Evas *e, char *path);
   Evas_List        *evas_object_font_path_list        (Evas *e);
       
   void              evas_object_font_cache_flush      (Evas *e);
   void              evas_object_font_cache_set        (Evas *e, int size);
   int               evas_object_font_cache_get        (Evas *e);       
   
   void              evas_object_del                   (Evas_Object *obj);

   const char       *evas_object_type_get              (Evas_Object *obj);
   
   void              evas_object_layer_set             (Evas_Object *obj, int l);
   int               evas_object_layer_get             (Evas_Object *obj);
   
   void              evas_object_raise                 (Evas_Object *obj);
   void              evas_object_lower                 (Evas_Object *obj);
   void              evas_object_stack_above           (Evas_Object *obj, Evas_Object *above);
   void              evas_object_stack_below           (Evas_Object *obj, Evas_Object *below);
   Evas_Object      *evas_object_above_get             (Evas_Object *obj);
   Evas_Object      *evas_object_below_get             (Evas_Object *obj);
   Evas_Object      *evas_object_bottom_get            (Evas *e);
   Evas_Object      *evas_object_top_get               (Evas *e);
       
   void              evas_object_move                  (Evas_Object *obj, double x, double y);
   void              evas_object_resize                (Evas_Object *obj, double w, double h);
   void              evas_object_geometry_get          (Evas_Object *obj, double *x, double *y, double *w, double *h);   

   void              evas_object_show                  (Evas_Object *obj);
   void              evas_object_hide                  (Evas_Object *obj);
   int               evas_object_visible_get           (Evas_Object *obj);
   
   void              evas_object_color_set             (Evas_Object *obj, int r, int g, int b, int a);
   void              evas_object_color_get             (Evas_Object *obj, int *r, int *g, int *b, int *a);
   
   void              evas_object_clip_set              (Evas_Object *obj, Evas_Object *clip);
   Evas_Object      *evas_object_clip_get              (Evas_Object *obj);
   void              evas_object_clip_unset            (Evas_Object *obj);
   Evas_List        *evas_object_clipees_get           (Evas_Object *obj);
   
   void              evas_object_data_set              (Evas_Object *obj, const char *key, void *data);
   void             *evas_object_data_get              (Evas_Object *obj, const char *key);
   void             *evas_object_data_del              (Evas_Object *obj, const char *key);
   
   void              evas_object_name_set              (Evas_Object *obj, const char *name);
   char             *evas_object_name_get              (Evas_Object *obj);
   Evas_Object      *evas_object_name_find             (Evas *e, const char *name);   
   
   Evas             *evas_object_evas_get              (Evas_Object *obj);
   
   Evas_Object      *evas_object_top_at_xy_get         (Evas *e, double x, double y, int include_pass_events_objects, int include_hidden_objects);
   Evas_Object      *evas_object_top_at_pointer_get    (Evas *e);
   Evas_Object      *evas_object_top_in_rectangle_get  (Evas *e, double x, double y, double w, double h, int include_pass_events_objects, int include_hidden_objects);
   
   Evas_List        *evas_objects_at_xy_get            (Evas *e, double x, double y, int include_pass_events_objects, int include_hidden_objects);
   Evas_List        *evas_objects_in_rectangle_get     (Evas *e, double x, double y, double w, double h, int include_pass_events_objects, int include_hidden_objects);

   Evas_Smart       *evas_smart_new                    (char *name, void (*func_add) (Evas_Object *o), void (*func_del) (Evas_Object *o), void (*func_layer_set) (Evas_Object *o, int l), void (*func_raise) (Evas_Object *o), void (*func_lower) (Evas_Object *o), void (*func_stack_above) (Evas_Object *o, Evas_Object *above), void (*func_stack_below) (Evas_Object *o, Evas_Object *below), void (*func_move) (Evas_Object *o, double x, double y), void (*func_resize) (Evas_Object *o, double w, double h), void (*func_show) (Evas_Object *o), void (*func_hide) (Evas_Object *o), void (*func_color_set) (Evas_Object *o, int r, int g, int b, int a), void (*func_clip_set) (Evas_Object *o, Evas_Object *clip), void (*func_clip_unset) (Evas_Object *o), void *data);
   void              evas_smart_free                   (Evas_Smart *s);
   void             *evas_smart_data_get               (Evas_Smart *s);
       
   Evas_Object      *evas_object_smart_add             (Evas *e, Evas_Smart *s);
   void              evas_object_smart_member_add      (Evas_Object *obj, Evas_Object *smart_obj);
   void              evas_object_smart_member_del      (Evas_Object *obj);
   Evas_Smart       *evas_object_smart_smart_get       (Evas_Object *obj);
   void             *evas_object_smart_data_get        (Evas_Object *obj);
   void              evas_object_smart_data_set        (Evas_Object *obj, void *data);
   void              evas_object_smart_callback_add    (Evas_Object *obj, char *event, void (*func) (void *data, Evas_Object *obj, void *event_info), void *data);
   void             *evas_object_smart_callback_del    (Evas_Object *obj, char *event, void (*func) (void *data, Evas_Object *obj, void *event_info));
   void              evas_object_smart_callback_call   (Evas_Object *obj, char *event, void *event_info);
       
   void              evas_pointer_output_xy_get        (Evas *e, int *x, int *y);
   void              evas_pointer_canvas_xy_get        (Evas *e, double *x, double *y);
   int               evas_pointer_button_down_mask_get (Evas *e);
   int               evas_pointer_inside_get           (Evas *e);
   
   void              evas_event_freeze                 (Evas *e);
   void              evas_event_thaw                   (Evas *e);
   int               evas_event_freeze_get             (Evas *e);
   void              evas_event_feed_mouse_down_data   (Evas *e, int b, void *data);
   void              evas_event_feed_mouse_up_data     (Evas *e, int b, void *data);
   void              evas_event_feed_mouse_move_data   (Evas *e, int x, int y, void *data);
   void              evas_event_feed_mouse_in_data     (Evas *e, void *data);
   void              evas_event_feed_mouse_out_data    (Evas *e, void *data);
/* FIXME: implement...
   void              evas_event_feed_key_down_data     (Evas *e, char *keyname, void *data);
   void              evas_event_feed_key_up_data       (Evas *e, char *keyname, void *data);
...end implement */
   void              evas_event_feed_mouse_down        (Evas *e, int b);
   void              evas_event_feed_mouse_up          (Evas *e, int b);
   void              evas_event_feed_mouse_move        (Evas *e, int x, int y);
   void              evas_event_feed_mouse_in          (Evas *e);
   void              evas_event_feed_mouse_out         (Evas *e);
/* FIXME: implement...
   void              evas_event_feed_key_down          (Evas *e, char *keyname);
   void              evas_event_feed_key_up            (Evas *e, char *keyname);

   void              evas_object_focus_set             (Evas_Object *o, int focus);
   int               evas_object_focus_get             (Evas_Object *o);

   Evas_Object      *evas_focus_get                    (Evas *e);

   Evas_Modifier    *evas_key_modifier_get             (Evas *e);
   Evas_Lock        *evas_key_lock_get                 (Evas *e);

   char             *evas_key_name_get                 (Evas_Key *k);
   
   int               evas_key_modifier_is_set_get      (Evas_Modifier *m, char *keyname);

   int               evas_key_lock_is_set_get          (Evas_Lock *l, char *keyname);
   
   void              evas_key_modifier_add             (Evas *e, char *keyname);
   void              evas_key_modifier_del             (Evas *e, char *keyname);
   void              evas_key_lock_add                 (Evas *e, char *keyname);
   void              evas_key_lock_del                 (Evas *e, char *keyname);
   
   void              evas_key_modifier_on              (Evas *e, char *keyname);
   void              evas_key_modifier_off             (Evas *e, char *keyname);
   void              evas_key_lock_on                  (Evas *e, char *keyname);
   void              evas_key_lock_off                 (Evas *e, char *keyname);
...end implement */
   
   void              evas_object_pass_events_set       (Evas_Object *obj, int pass);
   int               evas_object_pass_events_get       (Evas_Object *obj);
   void              evas_object_repeat_events_set     (Evas_Object *obj, int repeat);
   int               evas_object_repeat_events_get     (Evas_Object *obj);

   void              evas_object_event_callback_add    (Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), void *data);
   void             *evas_object_event_callback_del    (Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info));
       
#ifdef __cplusplus
}
#endif

#endif


