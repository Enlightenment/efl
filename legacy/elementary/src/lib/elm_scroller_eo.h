/**
 * @ingroup Scroller
 *
 * @{
 */
#define ELM_OBJ_SCROLLER_CLASS elm_obj_scroller_class_get()

const Eo_Class *elm_obj_scroller_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_SCROLLER_BASE_ID;

enum
{
   ELM_OBJ_SCROLLER_SUB_ID_CUSTOM_WIDGET_BASE_THEME_SET,
   ELM_OBJ_SCROLLER_SUB_ID_PAGE_SCROLL_LIMIT_SET,
   ELM_OBJ_SCROLLER_SUB_ID_PAGE_SCROLL_LIMIT_GET,
   ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_SET,
   ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_GET,
   ELM_OBJ_SCROLLER_SUB_ID_LAST
};

#define ELM_OBJ_SCROLLER_ID(sub_id) (ELM_OBJ_SCROLLER_BASE_ID + sub_id)


/**
 * @def elm_obj_scroller_custom_widget_base_theme_set
 * @since 1.8
 *
 * @brief Set custom theme elements for the scroller
 *
 * @param[in] klass
 * @param[in] group
 *
 * @see elm_scroller_custom_widget_base_theme_set
 */
#define elm_obj_scroller_custom_widget_base_theme_set(klass, group) ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_CUSTOM_WIDGET_BASE_THEME_SET), EO_TYPECHECK(const char *, klass), EO_TYPECHECK(const char *, group)

/**
 * @def elm_obj_scroller_page_scroll_limit_set
 * @since 1.8
 *
 * @brief Set the maxium of the movable page at a flicking.
 *
 * @param[in] page_limit_h
 * @param[in] page_limit_v
 *
 * @see elm_scroller_page_scroll_limit_set
 */
#define elm_obj_scroller_page_scroll_limit_set(page_limit_h, page_limit_v) ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_PAGE_SCROLL_LIMIT_SET), EO_TYPECHECK(int, page_limit_h), EO_TYPECHECK(int, page_limit_v)

/**
 * @def elm_obj_scroller_page_scroll_limit_get
 * @since 1.8
 *
 * @brief Get the maxium of the movable page at a flicking.
 *
 * @param[in] page_limit_h
 * @param[in] page_limit_v
 *
 * @see elm_scroller_page_scroll_limit_get
 */
#define elm_obj_scroller_page_scroll_limit_get(page_limit_h, page_limit_v) ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_PAGE_SCROLL_LIMIT_GET), EO_TYPECHECK(int *, page_limit_h), EO_TYPECHECK(int *, page_limit_v)

/**
 * @def elm_obj_scroller_propagate_events_set
 * @since 1.8
 *
 * @brief Set event propagation on a scroller
 *
 * @param[in] propagation
 *
 * @see elm_scroller_propagate_events_set
 */
#define elm_obj_scroller_propagate_events_set(propagation) ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_SET), EO_TYPECHECK(Eina_Bool, propagation)

/**
 * @def elm_obj_scroller_propagate_events_get
 * @since 1.8
 *
 * @brief Get event propagation for a scroller
 *
 * @param[out] ret
 *
 * @see elm_scroller_propagate_events_get
 */
#define elm_obj_scroller_propagate_events_get(ret) ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, ret)


/**
 * @}
 */
