/**
 * @ingroup Mapbuf
 *
 * @{
 */
#define ELM_OBJ_MAPBUF_CLASS elm_obj_mapbuf_class_get()

const Eo_Class *elm_obj_mapbuf_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_MAPBUF_BASE_ID;

enum
{
   ELM_OBJ_MAPBUF_SUB_ID_ENABLED_SET,
   ELM_OBJ_MAPBUF_SUB_ID_ENABLED_GET,
   ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_SET,
   ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_GET,
   ELM_OBJ_MAPBUF_SUB_ID_ALPHA_SET,
   ELM_OBJ_MAPBUF_SUB_ID_ALPHA_GET,
   ELM_OBJ_MAPBUF_SUB_ID_AUTO_SET,
   ELM_OBJ_MAPBUF_SUB_ID_AUTO_GET,
   ELM_OBJ_MAPBUF_SUB_ID_LAST
};

#define ELM_OBJ_MAPBUF_ID(sub_id) (ELM_OBJ_MAPBUF_BASE_ID + sub_id)


/**
 * @def elm_obj_mapbuf_enabled_set
 * @since 1.8
 *
 * Enable or disable the map.
 *
 * @param[in] enabled
 *
 * @see elm_mapbuf_enabled_set
 */
#define elm_obj_mapbuf_enabled_set(enabled) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_mapbuf_enabled_get
 * @since 1.8
 *
 * Get a value whether map is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_mapbuf_enabled_get
 */
#define elm_obj_mapbuf_enabled_get(ret) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_mapbuf_smooth_set
 * @since 1.8
 *
 * Enable or disable smooth map rendering.
 *
 * @param[in] smooth
 *
 * @see elm_mapbuf_smooth_set
 */
#define elm_obj_mapbuf_smooth_set(smooth) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_SET), EO_TYPECHECK(Eina_Bool, smooth)

/**
 * @def elm_obj_mapbuf_smooth_get
 * @since 1.8
 *
 * Get a value whether smooth map rendering is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_mapbuf_smooth_get
 */
#define elm_obj_mapbuf_smooth_get(ret) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_SMOOTH_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_mapbuf_alpha_set
 * @since 1.8
 *
 * Set or unset alpha flag for map rendering.
 *
 * @param[in] alpha
 *
 * @see elm_mapbuf_alpha_set
 */
#define elm_obj_mapbuf_alpha_set(alpha) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ALPHA_SET), EO_TYPECHECK(Eina_Bool, alpha)

/**
 * @def elm_obj_mapbuf_alpha_get
 * @since 1.8
 *
 * Get a value whether alpha blending is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_mapbuf_alpha_get
 */
#define elm_obj_mapbuf_alpha_get(ret) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_ALPHA_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_mapbuf_auto_set
 * @since 1.8
 *
 * Set or unset automatic flag for map rendering.
 *
 * @param[in] on
 *
 * @see elm_mapbuf_auto_set
 */
#define elm_obj_mapbuf_auto_set(on) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_AUTO_SET), EO_TYPECHECK(Eina_Bool, on)

/**
 * @def elm_obj_mapbuf_auto_get
 * @since 1.8
 *
 * Get a value automatic map mode is enabled ore not.
 *
 * @param[out] ret
 *
 * @see elm_mapbuf_auto_get
 */
#define elm_obj_mapbuf_auto_get(ret) ELM_OBJ_MAPBUF_ID(ELM_OBJ_MAPBUF_SUB_ID_AUTO_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @}
 */
