/* This header is not to be installed and should only be used by EFL internals */
#ifndef _EVAS_INTERNAL_H
#define _EVAS_INTERNAL_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
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
#endif

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

EAPI void evas_render_pending_objects_flush(Evas *eo_evas);

EAPI void efl_input_pointer_finalize(Efl_Input_Pointer *obj);

EAPI Eina_Iterator *efl_canvas_iterator_create(Eo *obj, Eina_Iterator *real_iterator, Eina_List *list);

static inline void
evas_object_size_hint_combined_min_get(const Eo *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_hint_size_combined_min_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

static inline void
evas_object_size_hint_combined_max_get(const Eo *obj, int *w, int *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_hint_size_combined_max_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

/* Internal EO APIs */
EAPI Eo *evas_find(const Eo *obj);
EOAPI void efl_canvas_object_legacy_ctor(Eo *obj);
EOAPI void efl_canvas_object_type_set(Eo *obj, const char *type);
EOAPI void efl_canvas_group_add(Eo *obj);
EOAPI void efl_canvas_group_del(Eo *obj);
EOAPI void efl_canvas_group_clipped_set(Eo *obj, Eina_Bool unclipped);
EOAPI void evas_canvas_touch_point_list_nth_xy_get(Eo *obj, unsigned int n, double *x, double *y);
EOAPI void evas_canvas_seat_focus_in(Eo *obj, Efl_Input_Device *seat);
EOAPI void evas_canvas_seat_focus_out(Eo *obj, Efl_Input_Device *seat);
EOAPI Eo* evas_canvas_seat_focus_get(const Eo *obj, Efl_Input_Device *seat);

EOAPI void *efl_input_legacy_info_get(const Eo *obj);

EOAPI Eo *efl_input_focus_instance_get(Efl_Object *owner, void **priv);
EOAPI Eo *efl_input_hold_instance_get(Efl_Object *owner, void **priv);
EOAPI Eo *efl_input_key_instance_get(Efl_Object *owner, void **priv);
EOAPI Eo *efl_input_pointer_instance_get(Efl_Object *owner, void **priv);
/**
 * @brief If @c true the object belongs to the window border decorations.
 *
 * This will be @c false by default, and should be @c false for all objects
 * created by the application, unless swallowed in some very specific parts of
 * the window.
 *
 * It is very unlikely that an application needs to call this manually, as the
 * window will handle this feature automatically.
 *
 * @param[in] obj The object.
 * @param[in] is_frame @c true if the object is a frame, @c false otherwise
 *
 * @since 1.2
 *
 * @ingroup Efl_Canvas_Object
 */
EOAPI void efl_canvas_object_is_frame_object_set(Eo *obj, Eina_Bool is_frame);

/**
 * @brief If @c true the object belongs to the window border decorations.
 *
 * This will be @c false by default, and should be @c false for all objects
 * created by the application, unless swallowed in some very specific parts of
 * the window.
 *
 * It is very unlikely that an application needs to call this manually, as the
 * window will handle this feature automatically.
 *
 * @param[in] obj The object.
 *
 * @return @c true if the object is a frame, @c false otherwise
 *
 * @since 1.2
 *
 * @ingroup Efl_Canvas_Object
 */
EOAPI Eina_Bool efl_canvas_object_is_frame_object_get(const Eo *obj);

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

/* Efl.Animation.Player */
EWAPI extern const Efl_Event_Description _EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED;
#define EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED (&(_EFL_ANIMATION_PLAYER_EVENT_PRE_STARTED))
/* Efl.Animation.Player END */



/*TextBlock Internal function*/
/**
 * Check if cursor is at cluster with one glyph (replace codepoints with new codepoint).
 *
 * @param cur the cursor.
 * @param forward if Eina_True check cluster after cusror position, else before cursor position.
 */
EAPI Eina_Bool evas_textblock_cursor_at_cluster_as_single_glyph(Evas_Textblock_Cursor *cur,Eina_Bool forward);

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

#endif
