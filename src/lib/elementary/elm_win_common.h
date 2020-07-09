/**
 * @addtogroup Elm_Win_Group
 *
 * @{
 */

/**
 * @typedef Elm_Win_Trap
 *
 * Trap can be set with elm_win_trap_set() and will intercept the
 * calls to internal ecore_evas with the same name and parameters. If
 * there is a trap and it returns @c EINA_TRUE then the call will be
 * allowed, otherwise it will be ignored.
 *
 * @since 1.7
 */
typedef struct _Elm_Win_Trap Elm_Win_Trap;
struct _Elm_Win_Trap
{
#define ELM_WIN_TRAP_VERSION (1UL)
   unsigned long version;
   void *(*add)(Evas_Object *o); /**< object was just added. The returned pointer will be handled to every other trap call. */
   void (*del)(void *data, Evas_Object *o); /**< object will be deleted. */
   Eina_Bool (*hide)(void *data, Evas_Object *o);
   Eina_Bool (*show)(void *data, Evas_Object *o);
   Eina_Bool (*move)(void *data, Evas_Object *o, int x, int y);
   Eina_Bool (*resize)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*center)(void *data, Evas_Object *o, Eina_Bool h, Eina_Bool v); /* not in ecore_evas, but nice to trap */
   Eina_Bool (*lower)(void *data, Evas_Object *o);
   Eina_Bool (*raise)(void *data, Evas_Object *o);
   Eina_Bool (*activate)(void *data, Evas_Object *o);
   Eina_Bool (*alpha_set)(void *data, Evas_Object *o, Eina_Bool alpha);
   Eina_Bool (*aspect_set)(void *data, Evas_Object *o, double aspect);
   Eina_Bool (*avoid_damage_set)(void *data, Evas_Object *o, Ecore_Evas_Avoid_Damage_Type on);
   Eina_Bool (*borderless_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*demand_attention_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*focus_skip_set)(void *data, Evas_Object *o, Eina_Bool skip);
   Eina_Bool (*fullscreen_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*iconified_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*layer_set)(void *data, Evas_Object *o, int layer);
   Eina_Bool (*manual_render_set)(void *data, Evas_Object *o, Eina_Bool manual_render);
   Eina_Bool (*maximized_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*modal_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*name_class_set)(void *data, Evas_Object *o, const char *n, const char *c);
   Eina_Bool (*object_cursor_set)(void *data, Evas_Object *o, Evas_Object *obj, int layer, int hot_x, int hot_y);
   Eina_Bool (*override_set)(void *data, Evas_Object *o, Eina_Bool on);
   Eina_Bool (*rotation_set)(void *data, Evas_Object *o, int rot);
   Eina_Bool (*rotation_with_resize_set)(void *data, Evas_Object *o, int rot);
   Eina_Bool (*shaped_set)(void *data, Evas_Object *o, Eina_Bool shaped);
   Eina_Bool (*size_base_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*size_step_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*size_min_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*size_max_set)(void *data, Evas_Object *o, int w, int h);
   Eina_Bool (*sticky_set)(void *data, Evas_Object *o, Eina_Bool sticky);
   Eina_Bool (*title_set)(void *data, Evas_Object *o, const char *t);
   Eina_Bool (*urgent_set)(void *data, Evas_Object *o, Eina_Bool urgent);
   Eina_Bool (*withdrawn_set)(void *data, Evas_Object *o, Eina_Bool withdrawn);
};

/**
 * Sets the trap to be used for internal @c Ecore_Evas management.
 *
 * @param trap the trap to be used or @c NULL to remove traps. Pointer
 *        is not modified or copied, keep it alive.
 * @return @c EINA_TRUE on success, @c EINA_FALSE if there was a
 *         problem, such as invalid version number.
 *
 * @warning this is an advanced feature that you should avoid using.
 *
 * @since 1.7
 */
EAPI Eina_Bool elm_win_trap_set(const Elm_Win_Trap *trap);

/**
 * @}
 */
