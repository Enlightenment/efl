   /**
    * @defgroup Ctxpopup Ctxpopup
    *
    * @image html img/widget/ctxpopup/preview-00.png
    * @image latex img/widget/ctxpopup/preview-00.eps
    *
    * @brief Context popup widet.
    *
    * A ctxpopup is a widget that, when shown, pops up a list of items.
    * It automatically chooses an area inside its parent object's view
    * (set via elm_ctxpopup_add() and elm_ctxpopup_hover_parent_set()) to
    * optimally fit into it. In the default theme, it will also point an
    * arrow to it's top left position at the time one shows it. Ctxpopup
    * items have a label and/or an icon. It is intended for a small
    * number of items (hence the use of list, not genlist).
    *
    * @note Ctxpopup is a especialization of @ref Hover.
    *
    * Signals that you can add callbacks for are:
    * "dismissed" - the ctxpopup was dismissed
    *
    * Default contents parts of the ctxpopup widget that you can use for are:
    * @li "default" - A content of the ctxpopup
    *
    * Default contents parts of the ctxpopup items that you can use for are:
    * @li "icon" - An icon in the title area
    *
    * Default text parts of the ctxpopup items that you can use for are:
    * @li "default" - Title label in the title area
    *
    * @ref tutorial_ctxpopup shows the usage of a good deal of the API.
    * @{
    */

   /**
    * @addtogroup Ctxpopup
    * @{
    */

   typedef enum _Elm_Ctxpopup_Direction
     {
        ELM_CTXPOPUP_DIRECTION_DOWN, /**< ctxpopup show appear below clicked
                                          area */
        ELM_CTXPOPUP_DIRECTION_RIGHT, /**< ctxpopup show appear to the right of
                                           the clicked area */
        ELM_CTXPOPUP_DIRECTION_LEFT, /**< ctxpopup show appear to the left of
                                          the clicked area */
        ELM_CTXPOPUP_DIRECTION_UP, /**< ctxpopup show appear above the clicked
                                        area */
        ELM_CTXPOPUP_DIRECTION_UNKNOWN, /**< ctxpopup does not determine it's direction yet*/
     } Elm_Ctxpopup_Direction;

   /**
    * @brief Add a new Ctxpopup object to the parent.
    *
    * @param parent Parent object
    * @return New object or @c NULL, if it cannot be created
    *
    * @ingroup Ctxpopup
    */
   EAPI Evas_Object  *elm_ctxpopup_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the Ctxpopup's parent
    *
    * @param obj The ctxpopup object
    * @param area The parent to use
    *
    * Set the parent object.
    *
    * @note elm_ctxpopup_add() will automatically call this function
    * with its @c parent argument.
    *
    * @see elm_ctxpopup_add()
    * @see elm_hover_parent_set()
    *
    * @ingroup Ctxpopup
    */
   EAPI void          elm_ctxpopup_hover_parent_set(Evas_Object *obj, Evas_Object *parent) EINA_ARG_NONNULL(1, 2);

   /**
    * @brief Get the Ctxpopup's parent
    *
    * @param obj The ctxpopup object
    *
    * @see elm_ctxpopup_hover_parent_set() for more information
    *
    * @ingroup Ctxpopup
    */
   EAPI Evas_Object  *elm_ctxpopup_hover_parent_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Clear all items in the given ctxpopup object.
    *
    * @param obj Ctxpopup object
    *
    * @ingroup Ctxpopup
    */
   EAPI void          elm_ctxpopup_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Change the ctxpopup's orientation to horizontal or vertical.
    *
    * @param obj Ctxpopup object
    * @param horizontal @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical
    *
    * @ingroup Ctxpopup
    */
   EAPI void          elm_ctxpopup_horizontal_set(Evas_Object *obj, Eina_Bool horizontal) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the value of current ctxpopup object's orientation.
    *
    * @param obj Ctxpopup object
    * @return @c EINA_TRUE for horizontal mode, @c EINA_FALSE for vertical mode (or errors)
    *
    * @see elm_ctxpopup_horizontal_set()
    *
    * @ingroup Ctxpopup
    */
   EAPI Eina_Bool     elm_ctxpopup_horizontal_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Add a new item to a ctxpopup object.
    *
    * @param obj Ctxpopup object
    * @param icon Icon to be set on new item
    * @param label The Label of the new item
    * @param func Convenience function called when item selected
    * @param data Data passed to @p func
    * @return A handle to the item added or @c NULL, on errors
    *
    * @warning Ctxpopup can't hold both an item list and a content at the same
    * time. When an item is added, any previous content will be removed.
    *
    * @see elm_ctxpopup_content_set()
    *
    * @ingroup Ctxpopup
    */
   Elm_Object_Item *elm_ctxpopup_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);

   /**
    * @brief Delete the given item in a ctxpopup object.
    *
    * @param it Ctxpopup item to be deleted
    *
    * @see elm_ctxpopup_item_append()
    *
    * @ingroup Ctxpopup
    */
   EAPI void          elm_ctxpopup_item_del(Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the ctxpopup item's state as disabled or enabled.
    *
    * @param it Ctxpopup item to be enabled/disabled
    * @param disabled @c EINA_TRUE to disable it, @c EINA_FALSE to enable it
    *
    * When disabled the item is greyed out to indicate it's state.
    * @deprecated use elm_object_item_disabled_set() instead
    *
    * @ingroup Ctxpopup
    */
   EINA_DEPRECATED EAPI void          elm_ctxpopup_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the ctxpopup item's disabled/enabled state.
    *
    * @param it Ctxpopup item to be enabled/disabled
    * @return disabled @c EINA_TRUE, if disabled, @c EINA_FALSE otherwise
    *
    * @see elm_ctxpopup_item_disabled_set()
    * @deprecated use elm_object_item_disabled_get() instead
    *
    * @ingroup Ctxpopup
    */
   EAPI Eina_Bool     elm_ctxpopup_item_disabled_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the icon object for the given ctxpopup item.
    *
    * @param it Ctxpopup item
    * @return icon object or @c NULL, if the item does not have icon or an error
    * occurred
    *
    * @see elm_ctxpopup_item_append()
    * @see elm_ctxpopup_item_icon_set()
    *
    * @deprecated use elm_object_item_part_content_get() instead
    *
    * @ingroup Ctxpopup
    */
   EINA_DEPRECATED EAPI Evas_Object  *elm_ctxpopup_item_icon_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * @brief Sets the side icon associated with the ctxpopup item
    *
    * @param it Ctxpopup item
    * @param icon Icon object to be set
    *
    * Once the icon object is set, a previously set one will be deleted.
    * @warning Setting the same icon for two items will cause the icon to
    * dissapear from the first item.
    *
    * @see elm_ctxpopup_item_append()
    *
    * @deprecated use elm_object_item_part_content_set() instead
    *
    * @ingroup Ctxpopup
    */
   EINA_DEPRECATED EAPI void          elm_ctxpopup_item_icon_set(Elm_Object_Item *it, Evas_Object *icon) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the label for the given ctxpopup item.
    *
    * @param it Ctxpopup item
    * @return label string or @c NULL, if the item does not have label or an
    * error occured
    *
    * @see elm_ctxpopup_item_append()
    * @see elm_ctxpopup_item_label_set()
    *
    * @deprecated use elm_object_item_text_get() instead
    *
    * @ingroup Ctxpopup
    */
   EINA_DEPRECATED EAPI const char   *elm_ctxpopup_item_label_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * @brief (Re)set the label on the given ctxpopup item.
    *
    * @param it Ctxpopup item
    * @param label String to set as label
    *
    * @deprecated use elm_object_item_text_set() instead
    *
    * @ingroup Ctxpopup
    */
   EINA_DEPRECATED EAPI void          elm_ctxpopup_item_label_set(Elm_Object_Item *it, const char *label) EINA_ARG_NONNULL(1);

   /**
    * @brief Set an elm widget as the content of the ctxpopup.
    *
    * @param obj Ctxpopup object
    * @param content Content to be swallowed
    *
    * If the content object is already set, a previous one will bedeleted. If
    * you want to keep that old content object, use the
    * elm_ctxpopup_content_unset() function.
    *
    * @warning Ctxpopup can't hold both a item list and a content at the same
    * time. When a content is set, any previous items will be removed.
    *
    * @deprecated use elm_object_content_set() instead
    *
    * @ingroup Ctxpopup
    */
   EINA_DEPRECATED EAPI void          elm_ctxpopup_content_set(Evas_Object *obj, Evas_Object *content) EINA_ARG_NONNULL(1, 2);

   /**
    * @brief Unset the ctxpopup content
    *
    * @param obj Ctxpopup object
    * @return The content that was being used
    *
    * Unparent and return the content object which was set for this widget.
    *
    * @deprecated use elm_object_content_unset()
    *
    * @see elm_ctxpopup_content_set()
    *
    * @deprecated use elm_object_content_unset() instead
    *
    * @ingroup Ctxpopup
    */
   EINA_DEPRECATED EAPI Evas_Object  *elm_ctxpopup_content_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the direction priority of a ctxpopup.
    *
    * @param obj Ctxpopup object
    * @param first 1st priority of direction
    * @param second 2nd priority of direction
    * @param third 3th priority of direction
    * @param fourth 4th priority of direction
    *
    * This functions gives a chance to user to set the priority of ctxpopup
    * showing direction. This doesn't guarantee the ctxpopup will appear in the
    * requested direction.
    *
    * @see Elm_Ctxpopup_Direction
    *
    * @ingroup Ctxpopup
    */
   EAPI void          elm_ctxpopup_direction_priority_set(Evas_Object *obj, Elm_Ctxpopup_Direction first, Elm_Ctxpopup_Direction second, Elm_Ctxpopup_Direction third, Elm_Ctxpopup_Direction fourth) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the direction priority of a ctxpopup.
    *
    * @param obj Ctxpopup object
    * @param first 1st priority of direction to be returned
    * @param second 2nd priority of direction to be returned
    * @param third 3th priority of direction to be returned
    * @param fourth 4th priority of direction to be returned
    *
    * @see elm_ctxpopup_direction_priority_set() for more information.
    *
    * @ingroup Ctxpopup
    */
   EAPI void          elm_ctxpopup_direction_priority_get(Evas_Object *obj, Elm_Ctxpopup_Direction *first, Elm_Ctxpopup_Direction *second, Elm_Ctxpopup_Direction *third, Elm_Ctxpopup_Direction *fourth) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the current direction of a ctxpopup.
    *
    * @param obj Ctxpopup object
    * @return current direction of a ctxpopup
    *
    * @warning Once the ctxpopup showed up, the direction would be determined
    *
    * @ingroup Ctxpopup
    */
   EAPI Elm_Ctxpopup_Direction elm_ctxpopup_direction_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @}
    */

