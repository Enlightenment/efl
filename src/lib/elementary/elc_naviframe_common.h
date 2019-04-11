/**
 * @typedef Elm_Naviframe_Item_Pop_Cb
 *
 * Pop callback called when @c it is going to be popped. @c data is user
 * specific data. If it returns the @c EINA_FALSE in the callback, item popping
 * will be cancelled.
 *
 * @see elm_naviframe_item_pop_cb_set()
 *
 * @since 1.8
 */
typedef Eina_Bool (*Elm_Naviframe_Item_Pop_Cb)(void *data, Elm_Object_Item *it);

/**
 * @brief Add a new Naviframe object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Elm_Naviframe
 */
EAPI Evas_Object     *elm_naviframe_add(Evas_Object *parent);

EAPI void             elm_naviframe_item_title_enabled_set(Elm_Object_Item *it, Eina_Bool enabled, Eina_Bool transition);

/**
 * @brief Push a new item to the top of the naviframe stack (and show it).
 *
 * The item pushed becomes one page of the naviframe, this item will be deleted
 * when it is popped.
 *
 * When push transition animation is in progress, pop operation is blocked
 * until push is complete.
 *
 * The following styles are available for this item: "default"
 *
 * @param[in] obj The object.
 * @param[in] title_label The label in the title area. The name of the title
 * label part is "elm.text.title"
 * @param[in] prev_btn The button to go to the previous item. If it is NULL,
 * then naviframe will create a back button automatically. The name of the
 * prev_btn part is "elm.swallow.prev_btn"
 * @param[in] next_btn The button to go to the next item. Or It could be just
 * an extra function button. The name of the next_btn part is
 * "elm.swallow.next_btn"
 * @param[in] content The main content object. The name of content part is
 * "elm.swallow.content"
 * @param[in] item_style The current item style name. @c NULL would be default.
 *
 * @return The created item or @c NULL upon failure.
 *
 * @ingroup Elm_Naviframe_Group
 */
EAPI Elm_Object_Item *elm_naviframe_item_push(Evas_Object *obj, const char *title_label, Evas_Object *prev_btn, Evas_Object *next_btn, Evas_Object *content, const char *item_style);

/**
 * @brief Simple version of item_push.
 *
 * @see elm_naviframe_item_push
 */
static inline Elm_Object_Item *
elm_naviframe_item_simple_push(Evas_Object *obj, Evas_Object *content)
{
   Elm_Object_Item *it;
   it = elm_naviframe_item_push(obj, NULL, NULL, NULL, content, NULL);
   elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
   return it;
}
