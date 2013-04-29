/**
 * @ingroup Naviframe
 *
 * @{
 */
#define ELM_OBJ_NAVIFRAME_CLASS elm_obj_naviframe_class_get()

const Eo_Class *elm_obj_naviframe_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_NAVIFRAME_BASE_ID;

enum
{
   ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_PUSH,
   ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_BEFORE,
   ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_AFTER,
   ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_POP,
   ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_SIMPLE_PROMOTE,
   ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_SET,
   ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_GET,
   ELM_OBJ_NAVIFRAME_SUB_ID_TOP_ITEM_GET,
   ELM_OBJ_NAVIFRAME_SUB_ID_BOTTOM_ITEM_GET,
   ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_SET,
   ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_GET,
   ELM_OBJ_NAVIFRAME_SUB_ID_ITEMS_GET,
   ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_SET,
   ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_GET,
   ELM_OBJ_NAVIFRAME_SUB_ID_LAST
};

#define ELM_OBJ_NAVIFRAME_ID(sub_id) (ELM_OBJ_NAVIFRAME_BASE_ID + sub_id)


/**
 * @def elm_obj_naviframe_item_push
 * @since 1.8
 *
 * @brief Push a new item to the top of the naviframe stack (and show it).
 *
 * @param[in] title_label
 * @param[in] prev_btn
 * @param[in] next_btn
 * @param[in] content
 * @param[in] item_style
 * @param[out] ret
 *
 * @see elm_naviframe_item_push
 */
#define elm_obj_naviframe_item_push(title_label, prev_btn, next_btn, content, item_style, ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_PUSH), EO_TYPECHECK(const char *, title_label), EO_TYPECHECK(Evas_Object *, prev_btn), EO_TYPECHECK(Evas_Object *, next_btn), EO_TYPECHECK(Evas_Object *, content), EO_TYPECHECK(const char *, item_style), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_naviframe_item_insert_before
 * @since 1.8
 *
 * @brief Insert a new item into the naviframe before item before.
 *
 * @param[in] before
 * @param[in] title_label
 * @param[in] prev_btn
 * @param[in] next_btn
 * @param[in] content
 * @param[in] item_style
 * @param[out] ret
 *
 * @see elm_naviframe_item_insert_before
 */
#define elm_obj_naviframe_item_insert_before(before, title_label, prev_btn, next_btn, content, item_style, ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_BEFORE), EO_TYPECHECK(Elm_Object_Item *, before), EO_TYPECHECK(const char *, title_label), EO_TYPECHECK(Evas_Object *, prev_btn), EO_TYPECHECK(Evas_Object *, next_btn), EO_TYPECHECK(Evas_Object *, content), EO_TYPECHECK(const char *, item_style), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_naviframe_item_insert_after
 * @since 1.8
 *
 * @brief Insert a new item into the naviframe after item after.
 *
 * @param[in] after
 * @param[in] title_label
 * @param[in] prev_btn
 * @param[in] next_btn
 * @param[in] content
 * @param[in] item_style
 * @param[out] ret
 *
 * @see elm_naviframe_item_insert_after
 */
#define elm_obj_naviframe_item_insert_after(after, title_label, prev_btn, next_btn, content, item_style, ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_INSERT_AFTER), EO_TYPECHECK(Elm_Object_Item *, after), EO_TYPECHECK(const char *, title_label), EO_TYPECHECK(Evas_Object *, prev_btn), EO_TYPECHECK(Evas_Object *, next_btn), EO_TYPECHECK(Evas_Object *, content), EO_TYPECHECK(const char *, item_style), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_naviframe_item_pop
 * @since 1.8
 *
 * @brief Pop an item that is on top of the stack
 *
 * @param[out] ret
 *
 * @see elm_naviframe_item_pop
 */
#define elm_obj_naviframe_item_pop(ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_POP), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_naviframe_item_simple_promote
 * @since 1.8
 *
 * @brief Simple version of item_promote.
 *
 * @param[in] content
 *
 * @see elm_naviframe_item_simple_promote
 */
#define elm_obj_naviframe_item_simple_promote(content) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEM_SIMPLE_PROMOTE), EO_TYPECHECK(Evas_Object *, content)

/**
 * @def elm_obj_naviframe_content_preserve_on_pop_set
 * @since 1.8
 *
 * @brief preserve the content objects when items are popped.
 *
 * @param[in] preserve
 *
 * @see elm_naviframe_content_preserve_on_pop_set
 */
#define elm_obj_naviframe_content_preserve_on_pop_set(preserve) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_SET), EO_TYPECHECK(Eina_Bool, preserve)

/**
 * @def elm_obj_naviframe_content_preserve_on_pop_get
 * @since 1.8
 *
 * @brief Get a value whether preserve mode is enabled or not.
 *
 * @param[out] ret
 *
 * @see elm_naviframe_content_preserve_on_pop_get
 */
#define elm_obj_naviframe_content_preserve_on_pop_get(ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_CONTENT_PRESERVE_ON_POP_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_naviframe_top_item_get
 * @since 1.8
 *
 * @brief Get a top item on the naviframe stack
 *
 * @param[out] ret
 *
 * @see elm_naviframe_top_item_get
 */
#define elm_obj_naviframe_top_item_get(ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_TOP_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_naviframe_bottom_item_get
 * @since 1.8
 *
 * @brief Get a bottom item on the naviframe stack
 *
 * @param[out] ret
 *
 * @see elm_naviframe_bottom_item_get
 */
#define elm_obj_naviframe_bottom_item_get(ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_BOTTOM_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_naviframe_prev_btn_auto_pushed_set
 * @since 1.8
 *
 * @brief Set creating prev button automatically or not
 *
 * @param[in] auto_pushed
 *
 * @see elm_naviframe_prev_btn_auto_pushed_set
 */
#define elm_obj_naviframe_prev_btn_auto_pushed_set(auto_pushed) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_SET), EO_TYPECHECK(Eina_Bool, auto_pushed)

/**
 * @def elm_obj_naviframe_prev_btn_auto_pushed_get
 * @since 1.8
 *
 * @brief Get a value whether prev button(back button) will be auto pushed or
 *
 * @param[out] ret
 *
 * @see elm_naviframe_prev_btn_auto_pushed_get
 */
#define elm_obj_naviframe_prev_btn_auto_pushed_get(ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_PREV_BTN_AUTO_PUSHED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_naviframe_items_get
 * @since 1.8
 *
 * @brief Get a list of all the naviframe items.
 *
 * @param[out] ret
 *
 * @see elm_naviframe_items_get
 */
#define elm_obj_naviframe_items_get(ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_ITEMS_GET), EO_TYPECHECK(Eina_List **, ret)

/**
 * @def elm_obj_naviframe_event_enabled_set
 * @since 1.8
 *
 * @brief Set the event enabled when pushing/popping items
 *
 * @param[in] enabled
 *
 * @see elm_naviframe_event_enabled_set
 */
#define elm_obj_naviframe_event_enabled_set(enabled) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_naviframe_event_enabled_get
 * @since 1.8
 *
 * @brief Get the value of event enabled status.
 *
 * @param[out] ret
 *
 * @see elm_naviframe_event_enabled_get
 */
#define elm_obj_naviframe_event_enabled_get(ret) ELM_OBJ_NAVIFRAME_ID(ELM_OBJ_NAVIFRAME_SUB_ID_EVENT_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @}
 */
