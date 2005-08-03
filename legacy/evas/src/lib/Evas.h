#ifndef _EVAS_H
#define _EVAS_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef GCC_HASCLASSVISIBILITY
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#endif

/**
 * @file
 * @brief These routines are used for Evas library interaction.
 */

typedef enum _Evas_Callback_Type
{
   EVAS_CALLBACK_MOUSE_IN, /**< Mouse In Event */
   EVAS_CALLBACK_MOUSE_OUT, /**< Mouse Out Event */
   EVAS_CALLBACK_MOUSE_DOWN, /**< Mouse Button Down Event */
   EVAS_CALLBACK_MOUSE_UP, /**< Mouse Button Up Event */
   EVAS_CALLBACK_MOUSE_MOVE, /**< Mouse Move Event */
   EVAS_CALLBACK_MOUSE_WHEEL, /**< Mouse Wheel Event */
   EVAS_CALLBACK_FREE, /**< Object Being Freed */
   EVAS_CALLBACK_KEY_DOWN, /**< Key Press Event */
   EVAS_CALLBACK_KEY_UP, /**< Key Release Event */
   EVAS_CALLBACK_FOCUS_IN, /**< Focus In Event */
   EVAS_CALLBACK_FOCUS_OUT, /**< Focus Out Event */
   EVAS_CALLBACK_SHOW, /**< Show Event */
   EVAS_CALLBACK_HIDE, /**< Hide Event */
   EVAS_CALLBACK_MOVE, /**< Move Event */
   EVAS_CALLBACK_RESIZE, /**< Resize Event */
   EVAS_CALLBACK_RESTACK /**< Restack Event */
} Evas_Callback_Type; /**< The type of event to trigger the callback */

typedef enum _Evas_Button_Flags
{
   EVAS_BUTTON_NONE = 0, /**< No extra mouse button data */
   EVAS_BUTTON_DOUBLE_CLICK = (1 << 0), /**< This mouse button press was the 2nd press of a double click */
   EVAS_BUTTON_TRIPLE_CLICK = (1 << 1) /**< This mouse button press was the 3rd press of a triple click */
} Evas_Button_Flags; /**< Flags for Mouse Button events */

typedef struct _Evas_List             Evas_List; /**< A generic linked list node handle */
typedef struct _Evas_Rectangle        Evas_Rectangle; /**< A generic rectangle handle */
typedef struct _Evas_Smart_Class      Evas_Smart_Class; /**< A smart object base class */

typedef struct _Evas_Hash Evas_Hash; /**< A Hash table handle */
typedef struct _Evas Evas; /**< An Evas canvas handle */
typedef struct _Evas_Object Evas_Object; /**< An Evas Object handle */
typedef void Evas_Performance; /**< An Evas Performance handle */
typedef struct _Evas_Modifier Evas_Modifier; /**< An Evas Modifier */
typedef struct _Evas_Lock Evas_Lock; /**< An Evas Lock */
typedef struct _Evas_Smart Evas_Smart; /**< An Evas Smart Object handle */
typedef unsigned long long Evas_Modifier_Mask; /**< An Evas modifier mask type */

typedef int    Evas_Coord;
typedef int    Evas_Font_Size;
typedef int    Evas_Angle;
typedef char   Evas_Bool;

struct _Evas_List /** A linked list node */
{
   void      *data; /**< Pointer to list element payload */
   Evas_List *next; /**< Next member in the list */
   Evas_List *prev; /**< Previous member in the list */
   void      *accounting; /**< Private list accounting info - don't touch */
};

struct _Evas_Rectangle /** A rectangle */
{
   int x; /**< top-left x co-ordinate of rectangle */
   int y; /**< top-left y co-ordinate of rectangle */
   int w; /**< width of rectangle */
   int h; /**< height of rectangle */
};

struct _Evas_Smart_Class /** a smart object class */
{
   const char *name; /** the string name of the class */

   void  (*add)         (Evas_Object *o);
   void  (*del)         (Evas_Object *o);
   void  (*layer_set)   (Evas_Object *o, int l);
   void  (*raise)       (Evas_Object *o);
   void  (*lower)       (Evas_Object *o);
   void  (*stack_above) (Evas_Object *o, Evas_Object *above);
   void  (*stack_below) (Evas_Object *o, Evas_Object *below);
   void  (*move)        (Evas_Object *o, Evas_Coord x, Evas_Coord y);
   void  (*resize)      (Evas_Object *o, Evas_Coord w, Evas_Coord h);
   void  (*show)        (Evas_Object *o);
   void  (*hide)        (Evas_Object *o);
   void  (*color_set)   (Evas_Object *o, int r, int g, int b, int a);
   void  (*clip_set)    (Evas_Object *o, Evas_Object *clip);
   void  (*clip_unset)  (Evas_Object *o);

   const void *data;
};

typedef struct _Evas_Pixel_Import_Source Evas_Pixel_Import_Source; /**< A source description of pixels for importing pixels */
typedef struct _Evas_Engine_Info      Evas_Engine_Info; /**< A generic Evas Engine information structure */
typedef struct _Evas_Event_Mouse_Down Evas_Event_Mouse_Down; /**< Event structure for #EVAS_CALLBACK_MOUSE_DOWN event callbacks */
typedef struct _Evas_Event_Mouse_Up   Evas_Event_Mouse_Up; /**< Event structure for #EVAS_CALLBACK_MOUSE_UP event callbacks */
typedef struct _Evas_Event_Mouse_In   Evas_Event_Mouse_In; /**< Event structure for #EVAS_CALLBACK_MOUSE_IN event callbacks */
typedef struct _Evas_Event_Mouse_Out  Evas_Event_Mouse_Out; /**< Event structure for #EVAS_CALLBACK_MOUSE_OUT event callbacks */
typedef struct _Evas_Event_Mouse_Move Evas_Event_Mouse_Move; /**< Event structure for #EVAS_CALLBACK_MOUSE_MOVE event callbacks */
typedef struct _Evas_Event_Mouse_Wheel Evas_Event_Mouse_Wheel; /**< Event structure for #EVAS_CALLBACK_MOUSE_WHEEL event callbacks */
typedef struct _Evas_Event_Key_Down   Evas_Event_Key_Down; /**< Event structure for #EVAS_CALLBACK_KEY_DOWN event callbacks */
typedef struct _Evas_Event_Key_Up     Evas_Event_Key_Up; /**< Event structure for #EVAS_CALLBACK_KEY_UP event callbacks */

#define EVAS_LOAD_ERROR_NONE                       0 /**< No error on load */
#define EVAS_LOAD_ERROR_GENERIC                    1 /**< A non-specific error occured */
#define EVAS_LOAD_ERROR_DOES_NOT_EXIST             2 /**< File (or file path) does not exist */
#define EVAS_LOAD_ERROR_PERMISSION_DENIED          3 /**< Permission deinied to an existing file (or path) */
#define EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED 4 /**< Allocation of resources failure prevented load */
#define EVAS_LOAD_ERROR_CORRUPT_FILE               5 /**< File corrupt (but was detected as a known format) */
#define EVAS_LOAD_ERROR_UNKNOWN_FORMAT             6 /**< File is not a known format */

#define EVAS_ALLOC_ERROR_NONE                      0 /**< No allocation error */
#define EVAS_ALLOC_ERROR_FATAL                     1 /**< Allocation failed despite attempts to free up memory */
#define EVAS_ALLOC_ERROR_RECOVERED                 2 /**< Allocation succeeded, but extra memory had to be found by freeing up speculative resources */

#define EVAS_TEXT_INVALID -1
#define EVAS_TEXT_SPECIAL -2

struct _Evas_Pixel_Import_Source
{
   int format; /**< pixel format type ie ARGB32, YUV420P_601 etc. */
   int w, h; /**< width and height of source in pixels */
   void **rows; /**< an array of pointers (size depends on format) pointing to left edge of each scanline */
};

#define EVAS_PIXEL_FORMAT_NONE                     0 /**< No pixel format */
#define EVAS_PIXEL_FORMAT_ARGB32                   1 /**< ARGB 32bit pixel format with A in the high byte per 32bit pixel word */
#define EVAS_PIXEL_FORMAT_YUV420P_601              2 /**< YUV 420 Planar format with CCIR 601 color encoding wuth contiguous planes in the order Y, U and V */

struct _Evas_Engine_Info /** Generic engine information. Generic info is useless */
{
   int magic; /**< Magic number */
};

struct _Evas_Event_Mouse_Down /** Mouse button press event */
{
   int button; /**< Mouse button number that went down (1 - 32) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
};

struct _Evas_Event_Mouse_Up /** Mouse butotn relase event */
{
   int button; /**< Mouse button number that was raised (1 - 32) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   Evas_Button_Flags flags;
   unsigned int      timestamp;
};

struct _Evas_Event_Mouse_In /** Mouse enter event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
};

struct _Evas_Event_Mouse_Out /** Mouse leave event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */
   struct {
      int x, y;
   } output;
   struct {
      Evas_Coord x, y;
   } canvas;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
};

struct _Evas_Event_Mouse_Move /** Mouse button down event */
{
   int buttons; /**< Button pressed mask, Bits set to 1 are buttons currently pressed (bit 0 = mouse button 1, bit 1 = mouse button 2 etc.) */
   struct {
      struct {
	 int x, y;
      } output;
      struct {
	 Evas_Coord x, y;
      } canvas;
   } cur, prev;
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;
   unsigned int   timestamp;
};

struct _Evas_Event_Mouse_Wheel /** Wheel event */
{
   int direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int z; /* ...,-2,-1 = down, 1,2,... = up */

   struct {
      int x, y;
   } output;

   struct {
      Evas_Coord x, y;
   } canvas;

   void		 *data;
   Evas_Modifier *modifiers;
   Evas_Lock	 *locks;
   unsigned int   timestamp;
};

struct _Evas_Event_Key_Down /** Key press event */
{
   char          *keyname; /**< The string name of the key pressed */
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   const char    *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char    *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char    *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int   timestamp;
};

struct _Evas_Event_Key_Up /** Key release event */
{
   char          *keyname; /**< The string name of the key released */
   void          *data;
   Evas_Modifier *modifiers;
   Evas_Lock     *locks;

   const char    *key; /**< The logical key : (eg shift+1 == exclamation) */
   const char    *string; /**< A UTF8 string if this keystroke has produced a visible string to be ADDED */
   const char    *compose; /**< A UTF8 string if this keystroke has modified a string in the middle of being composed - this string replaces the previous one */
   unsigned int   timestamp;
};

#ifdef __cplusplus
extern "C" {
#endif

   EAPI Evas_List        *evas_list_append                  (Evas_List *list, const void *data);
   EAPI Evas_List        *evas_list_prepend                 (Evas_List *list, const void *data);
   EAPI Evas_List        *evas_list_append_relative         (Evas_List *list, const void *data, const void *relative);
   EAPI Evas_List        *evas_list_prepend_relative        (Evas_List *list, const void *data, const void *relative);
   EAPI Evas_List        *evas_list_remove                  (Evas_List *list, const void *data);
   EAPI Evas_List        *evas_list_remove_list             (Evas_List *list, Evas_List *remove_list);
   EAPI void             *evas_list_find                    (Evas_List *list, const void *data);
   EAPI Evas_List        *evas_list_find_list               (Evas_List *list, const void *data);
   EAPI Evas_List        *evas_list_free                    (Evas_List *list);
   EAPI Evas_List        *evas_list_last                    (Evas_List *list);
   EAPI Evas_List        *evas_list_next                    (Evas_List *list);
   EAPI Evas_List        *evas_list_prev                    (Evas_List *list);
   EAPI void             *evas_list_data                    (Evas_List *list);
   EAPI int               evas_list_count                   (Evas_List *list);
   EAPI void             *evas_list_nth                     (Evas_List *list, int n);
   EAPI Evas_List        *evas_list_nth_list                (Evas_List *list, int n);
   EAPI Evas_List        *evas_list_reverse                 (Evas_List *list);
   EAPI Evas_List        *evas_list_sort		       (Evas_List *list, int size, int(*func)(void*,void*));
   EAPI int               evas_list_alloc_error             (void);

   /* FIXME: add:
    * api to add find, del members by data, size not just string and also
    * provide hash generation functions settable by the app
    *
    * do we really need this? hmmm - let me think... there may be a better way
    */
   EAPI Evas_Hash        *evas_hash_add                     (Evas_Hash *hash, const char *key, const void*data);
   EAPI Evas_Hash        *evas_hash_del                     (Evas_Hash *hash, const char *key, const void*data);
   EAPI void             *evas_hash_find                    (Evas_Hash *hash, const char *key);
   EAPI int               evas_hash_size                    (Evas_Hash *hash);
   EAPI void              evas_hash_free                    (Evas_Hash *hash);
   EAPI void              evas_hash_foreach                 (Evas_Hash *hash, Evas_Bool (*func) (Evas_Hash *hash, const char *key, void *data, void *fdata), const void *fdata);
   EAPI int               evas_hash_alloc_error             (void);

   EAPI int               evas_alloc_error                  (void);

   EAPI int               evas_init                         (void);
   EAPI int               evas_shutdown                     (void);

   EAPI Evas             *evas_new                          (void);
   EAPI void              evas_free                         (Evas *e);

   EAPI int               evas_render_method_lookup         (const char *name);
   EAPI Evas_List        *evas_render_method_list           (void);
   EAPI void              evas_render_method_list_free      (Evas_List *list);

   EAPI void              evas_output_method_set            (Evas *e, int render_method);
   EAPI int               evas_output_method_get            (Evas *e);

   EAPI Evas_Engine_Info *evas_engine_info_get              (Evas *e);
   EAPI void              evas_engine_info_set              (Evas *e, Evas_Engine_Info *info);

   EAPI void              evas_output_size_set              (Evas *e, int w, int h);
   EAPI void              evas_output_size_get              (Evas *e, int *w, int *h);
   EAPI void              evas_output_viewport_set          (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
   EAPI void              evas_output_viewport_get          (Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

   EAPI Evas_Coord        evas_coord_screen_x_to_world      (Evas *e, int x);
   EAPI Evas_Coord        evas_coord_screen_y_to_world      (Evas *e, int y);
   EAPI int               evas_coord_world_x_to_screen      (Evas *e, Evas_Coord x);
   EAPI int               evas_coord_world_y_to_screen      (Evas *e, Evas_Coord y);

   EAPI void              evas_pointer_output_xy_get        (Evas *e, int *x, int *y);
   EAPI void              evas_pointer_canvas_xy_get        (Evas *e, Evas_Coord *x, Evas_Coord *y);
   EAPI int               evas_pointer_button_down_mask_get (Evas *e);
   EAPI Evas_Bool         evas_pointer_inside_get           (Evas *e);
/* DOC UP TO HERE */
   EAPI void              evas_damage_rectangle_add         (Evas *e, int x, int y, int w, int h);
   EAPI void              evas_obscured_rectangle_add       (Evas *e, int x, int y, int w, int h);
   EAPI void              evas_obscured_clear               (Evas *e);
   EAPI Evas_List        *evas_render_updates               (Evas *e);
   EAPI void              evas_render_updates_free          (Evas_List *updates);
   EAPI void              evas_render                       (Evas *e);

   EAPI Evas_Object      *evas_object_rectangle_add         (Evas *e);

   EAPI Evas_Object      *evas_object_line_add              (Evas *e);
   EAPI void              evas_object_line_xy_set           (Evas_Object *obj, Evas_Coord x1, Evas_Coord y1, Evas_Coord x2, Evas_Coord y2);
   EAPI void              evas_object_line_xy_get           (Evas_Object *obj, Evas_Coord *x1, Evas_Coord *y1, Evas_Coord *x2, Evas_Coord *y2);

   EAPI Evas_Object      *evas_object_gradient_add          (Evas *e);
   EAPI void              evas_object_gradient_color_add    (Evas_Object *obj, int r, int g, int b, int a, int distance);
   EAPI void              evas_object_gradient_colors_clear (Evas_Object *obj);
   EAPI void              evas_object_gradient_angle_set    (Evas_Object *obj, Evas_Angle angle);
   EAPI Evas_Angle        evas_object_gradient_angle_get    (Evas_Object *obj);

   EAPI Evas_Object      *evas_object_polygon_add           (Evas *e);
   EAPI void              evas_object_polygon_point_add     (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   EAPI void              evas_object_polygon_points_clear  (Evas_Object *obj);

   EAPI Evas_Object      *evas_object_image_add             (Evas *e);
   EAPI void              evas_object_image_file_set        (Evas_Object *obj, const char *file, const char *key);
   EAPI void              evas_object_image_file_get        (Evas_Object *obj, char **file, char **key);
   EAPI void              evas_object_image_border_set      (Evas_Object *obj, int l, int r, int t, int b);
   EAPI void              evas_object_image_border_get      (Evas_Object *obj, int *l, int *r, int *t, int *b);
   EAPI void              evas_object_image_border_center_fill_set(Evas_Object *obj, Evas_Bool fill);
   EAPI Evas_Bool         evas_object_image_border_center_fill_get(Evas_Object *obj);
   EAPI void              evas_object_image_fill_set        (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
   EAPI void              evas_object_image_fill_get        (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   EAPI void              evas_object_image_size_set        (Evas_Object *obj, int w, int h);
   EAPI void              evas_object_image_size_get        (Evas_Object *obj, int *w, int *h);
   EAPI int               evas_object_image_load_error_get  (Evas_Object *obj);
   EAPI void              evas_object_image_data_set        (Evas_Object *obj, void *data);
   EAPI void             *evas_object_image_data_get        (Evas_Object *obj, Evas_Bool for_writing);
   EAPI void              evas_object_image_data_copy_set   (Evas_Object *obj, void *data);
   EAPI void              evas_object_image_data_update_add (Evas_Object *obj, int x, int y, int w, int h);
   EAPI void              evas_object_image_alpha_set       (Evas_Object *obj, Evas_Bool has_alpha);
   EAPI Evas_Bool         evas_object_image_alpha_get       (Evas_Object *obj);
   EAPI void              evas_object_image_smooth_scale_set(Evas_Object *obj, Evas_Bool smooth_scale);
   EAPI Evas_Bool         evas_object_image_smooth_scale_get(Evas_Object *obj);
   EAPI void              evas_object_image_reload          (Evas_Object *obj);
   EAPI Evas_Bool         evas_object_image_pixels_import          (Evas_Object *obj, Evas_Pixel_Import_Source *pixels);
   EAPI void              evas_object_image_pixels_get_callback_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *o), void *data);
   EAPI void              evas_object_image_pixels_dirty_set       (Evas_Object *obj, Evas_Bool dirty);
   EAPI Evas_Bool         evas_object_image_pixels_dirty_get       (Evas_Object *obj);

   EAPI void              evas_image_cache_flush            (Evas *e);
   EAPI void              evas_image_cache_reload           (Evas *e);
   EAPI void              evas_image_cache_set              (Evas *e, int size);
   EAPI int               evas_image_cache_get              (Evas *e);

   EAPI Evas_Object      *evas_object_text_add              (Evas *e);
   EAPI void              evas_object_text_font_source_set  (Evas_Object *obj, const char *font);
   EAPI const char       *evas_object_text_font_source_get  (Evas_Object *obj);
   EAPI void              evas_object_text_font_set         (Evas_Object *obj, const char *font, Evas_Font_Size size);
   EAPI void              evas_object_text_font_get         (Evas_Object *obj, char **font, Evas_Font_Size *size);
   EAPI void              evas_object_text_text_set         (Evas_Object *obj, const char *text);
   EAPI const char       *evas_object_text_text_get         (Evas_Object *obj);
   EAPI Evas_Coord        evas_object_text_ascent_get       (Evas_Object *obj);
   EAPI Evas_Coord        evas_object_text_descent_get      (Evas_Object *obj);
   EAPI Evas_Coord        evas_object_text_max_ascent_get   (Evas_Object *obj);
   EAPI Evas_Coord        evas_object_text_max_descent_get  (Evas_Object *obj);
   EAPI Evas_Coord        evas_object_text_horiz_advance_get(Evas_Object *obj);
   EAPI Evas_Coord        evas_object_text_vert_advance_get (Evas_Object *obj);
   EAPI Evas_Coord        evas_object_text_inset_get        (Evas_Object *obj);
   EAPI int               evas_object_text_char_pos_get     (Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
   EAPI int               evas_object_text_char_coords_get  (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);

   EAPI int               evas_string_char_next_get         (const char *str, int pos, int *decoded);
   EAPI int               evas_string_char_prev_get         (const char *str, int pos, int *decoded);

   EAPI void              evas_font_path_clear              (Evas *e);
   EAPI void              evas_font_path_append             (Evas *e, const char *path);
   EAPI void              evas_font_path_prepend            (Evas *e, const char *path);
   EAPI const Evas_List  *evas_font_path_list               (Evas *e);

   EAPI void              evas_font_cache_flush             (Evas *e);
   EAPI void              evas_font_cache_set               (Evas *e, int size);
   EAPI int               evas_font_cache_get               (Evas *e);

   EAPI Evas_Object           *evas_object_textblock_add                  (Evas *e);
   EAPI void                   evas_object_textblock_clear                (Evas_Object *obj);
   EAPI void                   evas_object_textblock_cursor_pos_set       (Evas_Object *obj, int pos);
   EAPI int                    evas_object_textblock_cursor_pos_get       (Evas_Object *obj);
   EAPI int                    evas_object_textblock_length_get           (Evas_Object *obj);
   EAPI int                    evas_object_textblock_cursor_line_get      (Evas_Object *obj);
   EAPI int                    evas_object_textblock_lines_get            (Evas_Object *obj);
   EAPI int                    evas_object_textblock_line_start_pos_get   (Evas_Object *obj);
   EAPI int                    evas_object_textblock_line_end_pos_get     (Evas_Object *obj);
   EAPI Evas_Bool              evas_object_textblock_line_get             (Evas_Object *obj, int line, Evas_Coord *lx, Evas_Coord *ly, Evas_Coord *lw, Evas_Coord *lh);
   EAPI Evas_Bool              evas_object_textblock_char_pos_get         (Evas_Object *obj, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
   EAPI int                    evas_object_textblock_char_coords_get      (Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
   EAPI void                   evas_object_textblock_text_insert          (Evas_Object *obj, const char *text);
   EAPI char                  *evas_object_textblock_text_get             (Evas_Object *obj, int len);
   EAPI void                   evas_object_textblock_text_del             (Evas_Object *obj, int len);
   EAPI void                   evas_object_textblock_format_insert        (Evas_Object *obj, const char *format);
   EAPI int                    evas_object_textblock_format_next_pos_get  (Evas_Object *obj);
   EAPI int                    evas_object_textblock_format_next_count_get(Evas_Object *obj);
   EAPI const char            *evas_object_textblock_format_next_get      (Evas_Object *obj, int n);
   EAPI void                   evas_object_textblock_format_next_del      (Evas_Object *obj, int n);
   EAPI int                    evas_object_textblock_format_prev_pos_get  (Evas_Object *obj);
   EAPI int                    evas_object_textblock_format_prev_count_get(Evas_Object *obj);
   EAPI const char            *evas_object_textblock_format_prev_get      (Evas_Object *obj, int n);
   EAPI void                   evas_object_textblock_format_prev_del      (Evas_Object *obj, int n);
   EAPI char                  *evas_object_textblock_format_current_get   (Evas_Object *obj);
   EAPI void                   evas_object_textblock_format_size_get      (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
   EAPI void                   evas_object_textblock_native_size_get      (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
   EAPI int                    evas_object_textblock_native_lines_get     (Evas_Object *obj);

   
   
   
   /* NEW texblock api - intended to replace the old - not complete yet */
   typedef struct _Evas_Textblock_Style  Evas_Textblock_Style;
   typedef struct _Evas_Textblock_Cursor Evas_Textblock_Cursor;
   
   EAPI Evas_Object                 *evas_object_textblock2_add(Evas *e);
   
   EAPI Evas_Textblock_Style        *evas_textblock2_style_new(void);
   EAPI void                         evas_textblock2_style_free(Evas_Textblock_Style *ts);
   EAPI void                         evas_textblock2_style_set(Evas_Textblock_Style *ts, const char *text);
   EAPI const char                  *evas_textblock2_style_get(Evas_Textblock_Style *ts);
   
   EAPI void                         evas_object_textblock2_style_set(Evas_Object *obj, Evas_Textblock_Style *ts);
   EAPI const Evas_Textblock_Style  *evas_object_textblock2_style_get(Evas_Object *obj);
   
   EAPI void                         evas_object_textblock2_text_markup_set(Evas_Object *obj, const char *text);
   EAPI char                        *evas_object_textblock2_text_markup_get(Evas_Object *obj);
    
   EAPI const Evas_Textblock_Cursor *evas_object_textblock2_cursor_get(Evas_Object *obj);
   EAPI Evas_Textblock_Cursor       *evas_object_textblock2_cursor_new(Evas_Object *obj);
   
   EAPI void                         evas_textblock2_cursor_free(Evas_Textblock_Cursor *cur);
   
   EAPI void                         evas_textblock2_cursor_node_first(Evas_Textblock_Cursor *cur);
   EAPI void                         evas_textblock2_cursor_node_last(Evas_Textblock_Cursor *cur);
   EAPI Evas_Bool                    evas_textblock2_cursor_node_next(Evas_Textblock_Cursor *cur);
   EAPI Evas_Bool                    evas_textblock2_cursor_node_prev(Evas_Textblock_Cursor *cur);

   EAPI void                         evas_textblock2_cursor_text_append(Evas_Textblock_Cursor *cur, const char *text);
   EAPI const char                  *evas_textblock2_cursor_node_text_get(Evas_Textblock_Cursor *cur);

   EAPI void                         evas_textblock2_cursor_format_push(Evas_Textblock_Cursor *cur, const char *format);
   EAPI void                         evas_textblock2_cursor_format_pop(Evas_Textblock_Cursor *cur);
   EAPI const char                  *evas_textblock2_cursor_format_get(Evas_Textblock_Cursor *cur);
       
   EAPI void                         evas_object_textblock2_clear(Evas_Object *obj);
   EAPI void                         evas_object_textblock2_size_requested_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);

   
   
   
   
   EAPI void              evas_object_del                   (Evas_Object *obj);

   EAPI const char       *evas_object_type_get              (Evas_Object *obj);

   EAPI void              evas_object_layer_set             (Evas_Object *obj, int l);
   EAPI int               evas_object_layer_get             (Evas_Object *obj);

   EAPI void              evas_object_raise                 (Evas_Object *obj);
   EAPI void              evas_object_lower                 (Evas_Object *obj);
   EAPI void              evas_object_stack_above           (Evas_Object *obj, Evas_Object *above);
   EAPI void              evas_object_stack_below           (Evas_Object *obj, Evas_Object *below);
   EAPI Evas_Object      *evas_object_above_get             (Evas_Object *obj);
   EAPI Evas_Object      *evas_object_below_get             (Evas_Object *obj);
   EAPI Evas_Object      *evas_object_bottom_get            (Evas *e);
   EAPI Evas_Object      *evas_object_top_get               (Evas *e);

   EAPI void              evas_object_move                  (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
   EAPI void              evas_object_resize                (Evas_Object *obj, Evas_Coord w, Evas_Coord h);
   EAPI void              evas_object_geometry_get          (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

   EAPI void              evas_object_show                  (Evas_Object *obj);
   EAPI void              evas_object_hide                  (Evas_Object *obj);
   EAPI Evas_Bool         evas_object_visible_get           (Evas_Object *obj);

   EAPI void              evas_object_color_set             (Evas_Object *obj, int r, int g, int b, int a);
   EAPI void              evas_object_color_get             (Evas_Object *obj, int *r, int *g, int *b, int *a);

   EAPI void              evas_object_clip_set              (Evas_Object *obj, Evas_Object *clip);
   EAPI Evas_Object      *evas_object_clip_get              (Evas_Object *obj);
   EAPI void              evas_object_clip_unset            (Evas_Object *obj);
   EAPI const Evas_List  *evas_object_clipees_get           (Evas_Object *obj);

   EAPI void              evas_object_data_set              (Evas_Object *obj, const char *key, const void *data);
   EAPI void             *evas_object_data_get              (Evas_Object *obj, const char *key);
   EAPI void             *evas_object_data_del              (Evas_Object *obj, const char *key);

   EAPI void              evas_object_name_set              (Evas_Object *obj, const char *name);
   EAPI const char       *evas_object_name_get              (Evas_Object *obj);
   EAPI Evas_Object      *evas_object_name_find             (Evas *e, const char *name);

   EAPI Evas             *evas_object_evas_get              (Evas_Object *obj);

   EAPI Evas_Object      *evas_object_top_at_xy_get         (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects);
   EAPI Evas_Object      *evas_object_top_at_pointer_get    (Evas *e);
   EAPI Evas_Object      *evas_object_top_in_rectangle_get  (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects);

   EAPI Evas_List        *evas_objects_at_xy_get            (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects);
   EAPI Evas_List        *evas_objects_in_rectangle_get     (Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Evas_Bool include_pass_events_objects, Evas_Bool include_hidden_objects);

   EAPI Evas_Smart       *evas_smart_new                    (const char *name, void (*func_add) (Evas_Object *obj), void (*func_del) (Evas_Object *obj), void (*func_layer_set) (Evas_Object *obj, int l), void (*func_raise) (Evas_Object *obj), void (*func_lower) (Evas_Object *obj), void (*func_stack_above) (Evas_Object *obj, Evas_Object *above), void (*func_stack_below) (Evas_Object *obj, Evas_Object *below), void (*func_move) (Evas_Object *obj, Evas_Coord x, Evas_Coord y), void (*func_resize) (Evas_Object *obj, Evas_Coord w, Evas_Coord h), void (*func_show) (Evas_Object *obj), void (*func_hide) (Evas_Object *obj), void (*func_color_set) (Evas_Object *obj, int r, int g, int b, int a), void (*func_clip_set) (Evas_Object *obj, Evas_Object *clip), void (*func_clip_unset) (Evas_Object *obj), const void *data);
   EAPI void              evas_smart_free                   (Evas_Smart *s);
   EAPI Evas_Smart       *evas_smart_class_new              (Evas_Smart_Class *sc);
   EAPI Evas_Smart_Class *evas_smart_class_get              (Evas_Smart *s);

   EAPI void             *evas_smart_data_get               (Evas_Smart *s);

   EAPI Evas_Object      *evas_object_smart_add             (Evas *e, Evas_Smart *s);
   EAPI void              evas_object_smart_member_add      (Evas_Object *obj, Evas_Object *smart_obj);
   EAPI void              evas_object_smart_member_del      (Evas_Object *obj);
   EAPI Evas_Smart       *evas_object_smart_smart_get       (Evas_Object *obj);
   EAPI void             *evas_object_smart_data_get        (Evas_Object *obj);
   EAPI void              evas_object_smart_data_set        (Evas_Object *obj, void *data);
   EAPI void              evas_object_smart_callback_add    (Evas_Object *obj, const char *event, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data);
   EAPI void             *evas_object_smart_callback_del    (Evas_Object *obj, const char *event, void (*func) (void *data, Evas_Object *obj, void *event_info));
   EAPI void              evas_object_smart_callback_call   (Evas_Object *obj, const char *event, void *event_info);

   EAPI void              evas_event_freeze                 (Evas *e);
   EAPI void              evas_event_thaw                   (Evas *e);
   EAPI int               evas_event_freeze_get             (Evas *e);
   EAPI void              evas_event_feed_mouse_down        (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_mouse_up          (Evas *e, int b, Evas_Button_Flags flags, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_mouse_move        (Evas *e, int x, int y, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_mouse_in          (Evas *e, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_mouse_out         (Evas *e, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_mouse_wheel       (Evas *e, int direction, int z, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_key_down          (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data);
   EAPI void              evas_event_feed_key_up            (Evas *e, const char *keyname, const char *key, const char *string, const char *compose, unsigned int timestamp, const void *data);

   EAPI void              evas_object_focus_set             (Evas_Object *obj, Evas_Bool focus);
   EAPI Evas_Bool         evas_object_focus_get             (Evas_Object *obj);

   EAPI Evas_Object      *evas_focus_get                    (Evas *e);

   EAPI Evas_Modifier    *evas_key_modifier_get             (Evas *e);
   EAPI Evas_Lock        *evas_key_lock_get                 (Evas *e);

   EAPI Evas_Bool         evas_key_modifier_is_set          (Evas_Modifier *m, const char *keyname);

   EAPI Evas_Bool         evas_key_lock_is_set              (Evas_Lock *l, const char *keyname);

   EAPI void              evas_key_modifier_add             (Evas *e, const char *keyname);
   EAPI void              evas_key_modifier_del             (Evas *e, const char *keyname);
   EAPI void              evas_key_lock_add                 (Evas *e, const char *keyname);
   EAPI void              evas_key_lock_del                 (Evas *e, const char *keyname);

   EAPI void              evas_key_modifier_on              (Evas *e, const char *keyname);
   EAPI void              evas_key_modifier_off             (Evas *e, const char *keyname);
   EAPI void              evas_key_lock_on                  (Evas *e, const char *keyname);
   EAPI void              evas_key_lock_off                 (Evas *e, const char *keyname);

   EAPI Evas_Modifier_Mask evas_key_modifier_mask_get       (Evas *e, const char *keyname);

   EAPI Evas_Bool         evas_object_key_grab        (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers, Evas_Bool exclusive);
   EAPI void              evas_object_key_ungrab            (Evas_Object *obj, const char *keyname, Evas_Modifier_Mask modifiers, Evas_Modifier_Mask not_modifiers);

   EAPI void              evas_object_pass_events_set       (Evas_Object *obj, Evas_Bool pass);
   EAPI Evas_Bool         evas_object_pass_events_get       (Evas_Object *obj);
   EAPI void              evas_object_repeat_events_set     (Evas_Object *obj, Evas_Bool repeat);
   EAPI Evas_Bool         evas_object_repeat_events_get     (Evas_Object *obj);

   EAPI void              evas_object_event_callback_add    (Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info), const void *data);
   EAPI void             *evas_object_event_callback_del    (Evas_Object *obj, Evas_Callback_Type type, void (*func) (void *data, Evas *e, Evas_Object *obj, void *event_info));

   EAPI void              evas_object_intercept_show_callback_add        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data);
   EAPI void             *evas_object_intercept_show_callback_del        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj));
   EAPI void              evas_object_intercept_hide_callback_add        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data);
   EAPI void             *evas_object_intercept_hide_callback_del        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj));
   EAPI void              evas_object_intercept_move_callback_add        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y), const void *data);
   EAPI void             *evas_object_intercept_move_callback_del        (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y));
   EAPI void              evas_object_intercept_resize_callback_add      (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h), const void *data);
   EAPI void             *evas_object_intercept_resize_callback_del      (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Coord w, Evas_Coord h));
   EAPI void              evas_object_intercept_raise_callback_add       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data);
   EAPI void             *evas_object_intercept_raise_callback_del       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj));
   EAPI void              evas_object_intercept_lower_callback_add       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), const void *data);
   EAPI void             *evas_object_intercept_lower_callback_del       (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj));
   EAPI void              evas_object_intercept_stack_above_callback_add (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above), const void *data);
   EAPI void             *evas_object_intercept_stack_above_callback_del (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *above));
   EAPI void              evas_object_intercept_stack_below_callback_add (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below), const void *data);
   EAPI void             *evas_object_intercept_stack_below_callback_del (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, Evas_Object *below));
   EAPI void              evas_object_intercept_layer_set_callback_add   (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l), const void *data);
   EAPI void             *evas_object_intercept_layer_set_callback_del   (Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, int l));

/* Evas imaging api - exports some of the comon gfx engine routines */
/* this is not complete and should be considered experimental. use at your */
/* own risk */

   typedef struct _Evas_Imaging_Image Evas_Imaging_Image;
   typedef struct _Evas_Imaging_Font Evas_Imaging_Font;

   EAPI Evas_Imaging_Image *evas_imaging_image_load      (const char *file, const char *key);
   EAPI void                evas_imaging_image_free      (Evas_Imaging_Image *im);
   EAPI void                evas_imaging_image_size_get  (Evas_Imaging_Image *im, int *w, int *h);
   EAPI Evas_Bool           evas_imaging_image_alpha_get (Evas_Imaging_Image *im);
   EAPI void                evas_imaging_image_cache_set (int bytes);
   EAPI int                 evas_imaging_image_cache_get (void);

   EAPI Evas_Imaging_Font  *evas_imaging_font_load                      (const char *file, const char *key, int size);
   EAPI void                evas_imaging_font_free                      (Evas_Imaging_Font *fn);
   EAPI int                 evas_imaging_font_ascent_get                (Evas_Imaging_Font *fn);
   EAPI int                 evas_imaging_font_descent_get               (Evas_Imaging_Font *fn);
   EAPI int                 evas_imaging_font_max_ascent_get            (Evas_Imaging_Font *fn);
   EAPI int                 evas_imaging_font_max_descent_get           (Evas_Imaging_Font *fn);
   EAPI int                 evas_imaging_font_line_advance_get          (Evas_Imaging_Font *fn);
   EAPI void                evas_imaging_font_string_advance_get        (Evas_Imaging_Font *fn, char *str, int *x, int *y);
   EAPI void                evas_imaging_font_string_size_query         (Evas_Imaging_Font *fn, char *str, int *w, int *h);
   EAPI int                 evas_imaging_font_string_inset_get          (Evas_Imaging_Font *fn, char *str);
   EAPI int                 evas_imaging_font_string_char_coords_get    (Evas_Imaging_Font *fn, char *str, int pos, int *cx, int *cy, int *cw, int *ch);
   EAPI int                 evas_imaging_font_string_char_at_coords_get (Evas_Imaging_Font *fn, char *str, int x, int y, int *cx, int *cy, int *cw, int *ch);

   EAPI void                evas_imaging_font_cache_set  (int bytes);
   EAPI int                 evas_imaging_font_cache_get  (void);

#ifdef __cplusplus
}
#endif

#endif
