/* This header is not to be installed and should only be used by EFL internals */
#ifndef _EVAS_INTERNAL_H
#define _EVAS_INTERNAL_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Efl_Canvas_Output Efl_Canvas_Output;

EAPI Efl_Canvas_Output *efl_canvas_output_add(Evas *canvas);
EAPI void efl_canvas_output_del(Efl_Canvas_Output *output);
EAPI void efl_canvas_output_view_set(Efl_Canvas_Output *output,
                                     Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
EAPI void efl_canvas_output_view_get(Efl_Canvas_Output *output,
                                     Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI Eina_Bool efl_canvas_output_engine_info_set(Efl_Canvas_Output *output,
                                                 Evas_Engine_Info *info);
EAPI Evas_Engine_Info *efl_canvas_output_engine_info_get(Efl_Canvas_Output *output);
EAPI Eina_Bool efl_canvas_output_lock(Efl_Canvas_Output *output);
EAPI Eina_Bool efl_canvas_output_unlock(Efl_Canvas_Output *output);

EAPI void efl_input_pointer_finalize(Efl_Input_Pointer *obj);

static inline void
evas_object_size_hint_combined_min_get(const Eo *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_size_hint_combined_min_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

/* Internal EO APIs */
EOAPI void efl_canvas_object_legacy_ctor(Eo *obj);
EOAPI void efl_canvas_object_type_set(Eo *obj, const char *type);
EOAPI void efl_canvas_group_add(Eo *obj);
EOAPI void efl_canvas_group_del(Eo *obj);
EOAPI void efl_canvas_group_clipped_set(Eo *obj, Eina_Bool unclipped);

EOAPI void *efl_input_legacy_info_get(const Eo *obj);
EOAPI Eo *efl_input_instance_get(const Eo *obj, Efl_Object *owner, void **priv);

EWAPI extern const Efl_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE;
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE))

EWAPI extern const Efl_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_POST;
#define EVAS_CANVAS_EVENT_RENDER_FLUSH_POST (&(_EVAS_CANVAS_EVENT_RENDER_FLUSH_POST))

EWAPI extern const Efl_Event_Description _EVAS_CANVAS_EVENT_AXIS_UPDATE;
#define EVAS_CANVAS_EVENT_AXIS_UPDATE (&(_EVAS_CANVAS_EVENT_AXIS_UPDATE))

EWAPI extern const Efl_Event_Description _EVAS_CANVAS_EVENT_VIEWPORT_RESIZE;
#define EVAS_CANVAS_EVENT_VIEWPORT_RESIZE (&(_EVAS_CANVAS_EVENT_VIEWPORT_RESIZE))

#define EFL_CANVAS_GROUP_DEL_OPS(kls) EFL_OBJECT_OP_FUNC(efl_canvas_group_del, _##kls##_efl_canvas_group_group_del)
#define EFL_CANVAS_GROUP_ADD_OPS(kls) EFL_OBJECT_OP_FUNC(efl_canvas_group_add, _##kls##_efl_canvas_group_group_add)
#define EFL_CANVAS_GROUP_ADD_DEL_OPS(kls) EFL_CANVAS_GROUP_ADD_OPS(kls), EFL_CANVAS_GROUP_DEL_OPS(kls)

/* Efl.Animation.Object */
EOAPI void efl_animation_object_target_set(Eo *obj, Efl_Canvas_Object *target);
EOAPI Efl_Canvas_Object *efl_animation_object_target_get(const Eo *obj);

EOAPI void efl_animation_object_final_state_keep_set(Eo *obj, Eina_Bool state_keep);
EOAPI Eina_Bool efl_animation_object_final_state_keep_get(const Eo *obj);

EOAPI void efl_animation_object_duration_set(Eo *obj, double duration);
EOAPI double efl_animation_object_duration_get(const Eo *obj);

EOAPI void efl_animation_object_duration_only_set(Eo *obj, double duration);

EOAPI void efl_animation_object_total_duration_set(Eo *obj, double total_duration);
EOAPI double efl_animation_object_total_duration_get(const Eo *obj);

EOAPI void efl_animation_object_start_delay_set(Eo *obj, double delay_time);
EOAPI double efl_animation_object_start_delay_get(const Eo *obj);

typedef enum
{
  EFL_ANIMATION_OBJECT_REPEAT_MODE_RESTART = 0,
  EFL_ANIMATION_OBJECT_REPEAT_MODE_REVERSE
} Efl_Animation_Object_Repeat_Mode;

EOAPI void efl_animation_object_repeat_mode_set(Eo *obj, Efl_Animation_Object_Repeat_Mode mode);
EOAPI Efl_Animation_Object_Repeat_Mode efl_animation_object_repeat_mode_get(const Eo *obj);

EOAPI void efl_animation_object_repeat_count_set(Eo *obj, int count);
EOAPI int efl_animation_object_repeat_count_get(const Eo *obj);

EWAPI extern const Efl_Event_Description _EFL_ANIMATION_OBJECT_EVENT_PRE_STARTED;
#define EFL_ANIMATION_OBJECT_EVENT_PRE_STARTED (&(_EFL_ANIMATION_OBJECT_EVENT_PRE_STARTED))
/* Efl.Animation.Object END */

/* Efl.Animation.Object.Alpha */
EOAPI void efl_animation_object_alpha_set(Eo *obj, double from_alpha, double to_alpha);
EOAPI void efl_animation_object_alpha_get(const Eo *obj, double *from_alpha, double *to_alpha);
/* Efl.Animation.Object.Alpha END */

/* Efl.Animation.Object.Rotate */
EOAPI void efl_animation_object_rotate_set(Eo *obj, double from_degree, double to_degree, Efl_Canvas_Object *pivot, double cx, double cy);
EOAPI void efl_animation_object_rotate_get(const Eo *obj, double *from_degree, double *to_degree, Efl_Canvas_Object **pivot, double *cx, double *cy);

EOAPI void efl_animation_object_rotate_absolute_set(Eo *obj, double from_degree, double to_degree, int cx, int cy);
EOAPI void efl_animation_object_rotate_absolute_get(const Eo *obj, double *from_degree, double *to_degree, int *cx, int *cy);
/* Efl.Animation.Object.Rotate END */

/* Efl.Animation.Object.Scale */
EOAPI void efl_animation_object_scale_set(Eo *obj, double from_scale_x, double from_scale_y, double to_scale_x, double to_scale_y, Efl_Canvas_Object *pivot, double cx, double cy);
EOAPI void efl_animation_object_scale_get(const Eo *obj, double *from_scale_x, double *from_scale_y, double *to_scale_x, double *to_scale_y, Efl_Canvas_Object **pivot, double *cx, double *cy);

EOAPI void efl_animation_object_scale_absolute_set(Eo *obj, double from_scale_x, double from_scale_y, double to_scale_x, double to_scale_y, int cx, int cy);
EOAPI void efl_animation_object_scale_absolute_get(const Eo *obj, double *from_scale_x, double *from_scale_y, double *to_scale_x, double *to_scale_y, int *cx, int *cy);
/* Efl.Animation.Object.Scale END */

/* Efl.Animation.Object.Translate */
EOAPI void efl_animation_object_translate_set(Eo *obj, int from_x, int from_y, int to_x, int to_y);
EOAPI void efl_animation_object_translate_get(const Eo *obj, int *from_x, int *from_y, int *to_x, int *to_y);

EOAPI void efl_animation_object_translate_absolute_set(Eo *obj, int from_x, int from_y, int to_x, int to_y);
EOAPI void efl_animation_object_translate_absolute_get(const Eo *obj, int *from_x, int *from_y, int *to_x, int *to_y);
/* Efl.Animation.Object.Translate END */

/* Efl.Animation.Object.Group */
EOAPI void efl_animation_object_group_object_add(Eo *obj, Efl_Animation_Object *anim_obj);
EOAPI void efl_animation_object_group_object_del(Eo *obj, Efl_Animation_Object *anim_obj);

EOAPI Eina_List *efl_animation_object_group_objects_get(Eo *obj);
/* Efl.Animation.Object.Group END */

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
