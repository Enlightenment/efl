   /**
    * Get the widget object's handle which contains a given item
    *
    * @param item The Elementary object item
    * @return The widget object
    *
    * @note This returns the widget object itself that an item belongs to.
    *
    * @ingroup General
    */
   EAPI Evas_Object *elm_object_item_object_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

   /**
    * Set a content of an object item
    *
    * @param it The Elementary object item
    * @param part The content part name to set (NULL for the default content)
    * @param content The new content of the object item
    *
    * @note Elementary object items may have many contents
    * @deprecated Use elm_object_item_part_content_set instead.
    * @ingroup General
    */
   EINA_DEPRECATED EAPI void elm_object_item_content_part_set(Elm_Object_Item *it, const char *part, Evas_Object *content);

   /**
    * Set a content of an object item
    *
    * @param it The Elementary object item
    * @param part The content part name to set (NULL for the default content)
    * @param content The new content of the object item
    *
    * @note Elementary object items may have many contents
    *
    * @ingroup General
    */
   EAPI void elm_object_item_part_content_set(Elm_Object_Item *it, const char *part, Evas_Object *content);

#define elm_object_item_content_set(it, content) elm_object_item_part_content_set((it), NULL, (content))

   /**
    * Get a content of an object item
    *
    * @param it The Elementary object item
    * @param part The content part name to unset (NULL for the default content)
    * @return content of the object item or NULL for any error
    *
    * @note Elementary object items may have many contents
    * @deprecated Use elm_object_item_part_content_get instead.
    * @ingroup General
    */
   EAPI Evas_Object *elm_object_item_content_part_get(const Elm_Object_Item *it, const char *part);

   /**
    * Get a content of an object item
    *
    * @param it The Elementary object item
    * @param part The content part name to unset (NULL for the default content)
    * @return content of the object item or NULL for any error
    *
    * @note Elementary object items may have many contents
    *
    * @ingroup General
    */
   EAPI Evas_Object *elm_object_item_part_content_get(const Elm_Object_Item *it, const char *part);

#define elm_object_item_content_get(it) elm_object_item_part_content_get((it), NULL)

   /**
    * Unset a content of an object item
    *
    * @param it The Elementary object item
    * @param part The content part name to unset (NULL for the default content)
    *
    * @note Elementary object items may have many contents
    * @deprecated Use elm_object_item_part_content_unset instead.
    * @ingroup General
    */
   EINA_DEPRECATED EAPI Evas_Object *elm_object_item_content_part_unset(Elm_Object_Item *it, const char *part);

   /**
    * Unset a content of an object item
    *
    * @param it The Elementary object item
    * @param part The content part name to unset (NULL for the default content)
    *
    * @note Elementary object items may have many contents
    *
    * @ingroup General
    */
   EAPI Evas_Object *elm_object_item_part_content_unset(Elm_Object_Item *it, const char *part);

#define elm_object_item_content_unset(it) elm_object_item_part_content_unset((it), NULL)

   /**
    * Set a label of an object item
    *
    * @param it The Elementary object item
    * @param part The text part name to set (NULL for the default label)
    * @param label The new text of the label
    *
    * @note Elementary object items may have many labels
    * @deprecated Use elm_object_item_part_text_set instead.
    * @ingroup General
    */
   EINA_DEPRECATED EAPI void elm_object_item_text_part_set(Elm_Object_Item *it, const char *part, const char *label);

   /**
    * Set a label of an object item
    *
    * @param it The Elementary object item
    * @param part The text part name to set (NULL for the default label)
    * @param label The new text of the label
    *
    * @note Elementary object items may have many labels
    *
    * @ingroup General
    */
   EAPI void elm_object_item_part_text_set(Elm_Object_Item *it, const char *part, const char *label);

#define elm_object_item_text_set(it, label) elm_object_item_part_text_set((it), NULL, (label))

   /**
    * Get a label of an object item
    *
    * @param it The Elementary object item
    * @param part The text part name to get (NULL for the default label)
    * @return text of the label or NULL for any error
    *
    * @note Elementary object items may have many labels
    * @deprecated Use elm_object_item_part_text_get instead.
    * @ingroup General
    */
   EINA_DEPRECATED EAPI const char *elm_object_item_text_part_get(const Elm_Object_Item *it, const char *part);
   /**
    * Get a label of an object item
    *
    * @param it The Elementary object item
    * @param part The text part name to get (NULL for the default label)
    * @return text of the label or NULL for any error
    *
    * @note Elementary object items may have many labels
    *
    * @ingroup General
    */
   EAPI const char *elm_object_item_part_text_get(const Elm_Object_Item *it, const char *part);

#define elm_object_item_text_get(it) elm_object_item_part_text_get((it), NULL)

   /**
    * Set the text to read out when in accessibility mode
    *
    * @param it The object item which is to be described
    * @param txt The text that describes the widget to people with poor or no vision
    *
    * @ingroup General
    */
   EAPI void elm_object_item_access_info_set(Elm_Object_Item *it, const char *txt);

   /**
    * Get the data associated with an object item
    * @param it The Elementary object item
    * @return The data associated with @p it
    *
    * @ingroup General
    */
   EAPI void *elm_object_item_data_get(const Elm_Object_Item *it);

   /**
    * Set the data associated with an object item
    * @param it The Elementary object item
    * @param data The data to be associated with @p it
    *
    * @ingroup General
    */
   EAPI void elm_object_item_data_set(Elm_Object_Item *it, void *data);

   /**
    * Send a signal to the edje object of the widget item.
    *
    * This function sends a signal to the edje object of the obj item. An
    * edje program can respond to a signal by specifying matching
    * 'signal' and 'source' fields.
    *
    * @param it The Elementary object item
    * @param emission The signal's name.
    * @param source The signal's source.
    * @ingroup General
    */
   EAPI void elm_object_item_signal_emit(Elm_Object_Item *it, const char *emission, const char *source) EINA_ARG_NONNULL(1);

   /**
    * Set the disabled state of an widget item.
    *
    * @param obj The Elementary object item
    * @param disabled The state to put in in: @c EINA_TRUE for
    *        disabled, @c EINA_FALSE for enabled
    *
    * Elementary object item can be @b disabled, in which state they won't
    * receive input and, in general, will be themed differently from
    * their normal state, usually greyed out. Useful for contexts
    * where you don't want your users to interact with some of the
    * parts of you interface.
    *
    * This sets the state for the widget item, either disabling it or
    * enabling it back.
    *
    * @ingroup Styles
    */
   EAPI void elm_object_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled) EINA_ARG_NONNULL(1);

   /**
    * Get the disabled state of an widget item.
    *
    * @param obj The Elementary object
    * @return @c EINA_TRUE, if the widget item is disabled, @c EINA_FALSE
    *            if it's enabled (or on errors)
    *
    * This gets the state of the widget, which might be enabled or disabled.
    *
    * @ingroup Styles
    */
   EAPI Eina_Bool    elm_object_item_disabled_get(const Elm_Object_Item *it) EINA_ARG_NONNULL(1);

