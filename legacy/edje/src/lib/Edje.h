#ifndef _EDJE_H
#define _EDJE_H

#include <Evas.h>

#define EDJE_DRAG_DIR_NONE 0
#define EDJE_DRAG_DIR_X    1
#define EDJE_DRAG_DIR_Y    2
#define EDJE_DRAG_DIR_XY   3

#define EDJE_LOAD_ERROR_NONE                       0
#define EDJE_LOAD_ERROR_GENERIC                    1
#define EDJE_LOAD_ERROR_DOES_NOT_EXIST             2
#define EDJE_LOAD_ERROR_PERMISSION_DENIED          3
#define EDJE_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED 4
#define EDJE_LOAD_ERROR_CORRUPT_FILE               5
#define EDJE_LOAD_ERROR_UNKNOWN_FORMAT             6

#ifdef __cplusplus
extern "C" {
#endif
   
   void         edje_init                       (void);
   void         edje_shutdown                   (void);
   void         edje_frametime_set              (double t);
   double       edje_frametime_get              (void);
   void         edje_freeze                     (void);
   void         edje_thaw                       (void);
   
   Evas_List   *edje_file_collection_list       (const char *file);
   void         edje_file_collection_list_free  (Evas_List *lst);
   char        *edje_file_data_get              (const char *file, const char *key);

   void         edje_color_class_set(const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);
   void         edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size);
   
   void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);
   void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);
   
   Evas_Object *edje_object_add                 (Evas *evas);
   const char  *edje_object_data_get            (Evas_Object *obj, const char *key);
   int          edje_object_file_set            (Evas_Object *o, const char *file, const char *part);
   void         edje_object_file_get            (Evas_Object *o, const char **file, const char **part);
   int          edje_object_load_error_get      (Evas_Object *o);
   void         edje_object_signal_callback_add (Evas_Object *o, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source), void *data);
   void        *edje_object_signal_callback_del (Evas_Object *o, const char *emission, const char *source, void (*func) (void *data, Evas_Object *o, const char *emission, const char *source));
   void         edje_object_signal_emit         (Evas_Object *o, const char *emission, const char *source);
   void         edje_object_play_set            (Evas_Object *o, int play);
   int          edje_object_play_get            (Evas_Object *o);
   void         edje_object_animation_set       (Evas_Object *o, int on);
   int          edje_object_animation_get       (Evas_Object *o);
   int          edje_object_freeze              (Evas_Object *o);
   int          edje_object_thaw                (Evas_Object *o);
   void         edje_object_color_class_set     (Evas_Object *o, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3);
   void         edje_object_text_class_set      (Evas_Object *o, const char *text_class, const char *font, Evas_Font_Size size);
   void         edje_object_size_min_get        (Evas_Object *o, Evas_Coord *minw, Evas_Coord *minh);
   void         edje_object_size_max_get        (Evas_Object *o, Evas_Coord *maxw, Evas_Coord *maxh);
   void         edje_object_size_min_calc       (Evas_Object *o, Evas_Coord *minw, Evas_Coord *minh);
   int          edje_object_part_exists         (Evas_Object *o, const char *part);
   void         edje_object_part_geometry_get   (Evas_Object *o, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
   void         edje_object_part_text_set       (Evas_Object *o, const char *part, const char *text);
   const char  *edje_object_part_text_get       (Evas_Object *o, const char *part);
   void         edje_object_part_swallow        (Evas_Object *o, const char *part, Evas_Object *o_swallow);
   void         edje_object_part_unswallow      (Evas_Object *o, Evas_Object *o_swallow);
   Evas_Object *edje_object_part_swallow_get    (Evas_Object *o, const char *part);
   const char  *edje_object_part_state_get      (Evas_Object *o, const char *part, double *val_ret);
   int          edje_object_part_drag_dir_get   (Evas_Object *o, const char *part);
   void         edje_object_part_drag_value_set (Evas_Object *o, const char *part, double dx, double dy);
   void         edje_object_part_drag_value_get (Evas_Object *o, const char *part, double *dx, double *dy);
   void         edje_object_part_drag_size_set  (Evas_Object *o, const char *part, double dw, double dh);
   void         edje_object_part_drag_size_get  (Evas_Object *o, const char *part, double *dw, double *dh);
   void         edje_object_part_drag_step_set  (Evas_Object *o, const char *part, double dx, double dy);
   void         edje_object_part_drag_step_get  (Evas_Object *o, const char *part, double *dx, double *dy);
   void         edje_object_part_drag_page_set  (Evas_Object *o, const char *part, double dx, double dy);
   void         edje_object_part_drag_page_get  (Evas_Object *o, const char *part, double *dx, double *dy);
   void         edje_object_part_drag_step      (Evas_Object *o, const char *part, double dx, double dy);
   void         edje_object_part_drag_page      (Evas_Object *o, const char *part, double dx, double dy);
       
#ifdef __cplusplus
}
#endif

#endif
