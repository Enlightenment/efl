/**
 * Add a new layout to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_layout_file_set()
 * @see elm_layout_theme_set()
 *
 * @ingroup Elm_Layout
 */
EAPI Evas_Object                 *elm_layout_add(Evas_Object *parent);

/**
 * Get the list of swallow parts of a given container widget
 *
 * @param obj The layout object
 *
 * @return list of swallow parts which should be freed by the user program with elm_list_free()
 *
 * @since 1.9
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_List                   *elm_layout_content_swallow_list_get(const Evas_Object *obj);

/**
 * Set the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @param content The child that will be added in this layout object
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_layout_content_unset() function.
 *
 * @note In an Edje theme, the part used as a content container is called
 * @c SWALLOW. This is why the parameter name is called @p swallow, but it is
 * expected to be a part name just like the second parameter of
 * elm_layout_box_append().
 *
 * @see elm_layout_box_append()
 * @see elm_layout_content_get()
 * @see elm_layout_content_unset()
 *
 * @see @ref secBox
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool                    elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);

/**
 * Get the child object in the given content part.
 *
 * @param obj The layout object
 * @param swallow The SWALLOW part to get its content
 *
 * @return The swallowed object or NULL if none or an error occurred
 *
 * @ingroup Elm_Layout
 */
EAPI Evas_Object                 *elm_layout_content_get(const Evas_Object *obj, const char *swallow);

/**
 * Unset the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this part.
 *
 * @ingroup Elm_Layout
 */
EAPI Evas_Object                 *elm_layout_content_unset(Evas_Object *obj, const char *swallow);

/**
 * Set the file that will be used as layout
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Elm_Layout
 *
 * @param[in] file The path to file (edj) that will be used as layout
 * @param[in] group The group that the layout belongs in edje file
 */
EAPI Eina_Bool elm_layout_file_set(Eo *obj, const char *file, const char *group);

/**
 * Get the loaded file
 *
 *
 * @ingroup Elm_Layout
 *
 * @param file The path to file (edj) used as layout
 * @param group The group that the layout belongs in edje file
 *
 * @since 1.14
 */
EAPI void elm_layout_file_get(Eo *obj, const char **file, const char **group);

/**
 * @brief Append child to layout box part.
 *
 * Once the object is appended, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use @ref elm_layout_box_remove to
 * make this layout forget about the object.
 *
 * @param[in] child The child object to append to box.
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool elm_layout_box_append(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Prepend child to layout box part.
 *
 * Once the object is prepended, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use @ref elm_layout_box_remove to
 * make this layout forget about the object.
 *
 * @param[in] child The child object to prepend to box.
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool elm_layout_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Insert child to layout box part before a reference object.
 *
 * Once the object is inserted, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use @ref elm_layout_box_remove to
 * make this layout forget about the object.
 *
 * @param[in] child The child object to insert into box.
 * @param[in] reference Another reference object to insert before in box.
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool elm_layout_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference);

/**
 * @brief Insert child to layout box part at a given position.
 *
 * Once the object is inserted, it will become child of the layout. Its
 * lifetime will be bound to the layout, whenever the layout dies the child
 * will be deleted automatically. One should use @ref elm_layout_box_remove to
 * make this layout forget about the object.
 *
 * @param[in] child The child object to insert into box.
 * @param[in] pos The numeric position >=0 to insert the child.
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool elm_layout_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos);

/**
 * @brief Remove a child of the given part box.
 *
 * The object will be removed from the box part and its lifetime will not be
 * handled by the layout anymore. This is equivalent to @ref
 * elm_layout_content_unset for box.
 *
 * @param[in] child The object to remove from box.
 *
 * @return The object that was being used, or @c null if not found.
 *
 * @ingroup Elm_Layout
 */
EAPI Evas_Object *elm_layout_box_remove(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Remove all children of the given part box.
 *
 * The objects will be removed from the box part and their lifetime will not be
 * handled by the layout anymore. This is equivalent to
 * @ref elm_layout_box_remove for all box children.
 *
 * @param[in] clear If true, then all objects will be deleted as well,
 * otherwise they will just be removed and will be dangling on the canvas.
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool elm_layout_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear);

/**
 * @brief Insert child to layout table part.
 *
 * Once the object is inserted, it will become child of the table. Its lifetime
 * will be bound to the layout, and whenever the layout dies the child will be
 * deleted automatically. One should use @ref elm_layout_table_unpack to make
 * this layout forget about the object.
 *
 * If @c colspan or @c rowspan are bigger than 1, that object will occupy more
 * space than a single cell.
 *
 * See also @ref elm_layout_table_unpack, @ref elm_layout_table_clear.
 *
 * @param[in] child The child object to pack into table.
 * @param[in] col The column to which the child should be added. (>= 0)
 * @param[in] row The row to which the child should be added. (>= 0)
 * @param[in] colspan How many columns should be used to store this object. (>=
 * 1)
 * @param[in] rowspan How many rows should be used to store this object. (>= 1)
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool elm_layout_table_pack(Evas_Object *obj, const char *part, Evas_Object *child, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan);

/**
 * @brief Unpack (remove) a child of the given part table.
 *
 * The object will be unpacked from the table part and its lifetime will not be
 * handled by the layout anymore. This is equivalent to @ref
 * elm_layout_content_unset for table.
 *
 * @param[in] child The object to remove from table.
 *
 * @return The object that was being used, or @c null if not found.
 *
 * @ingroup Elm_Layout
 */
EAPI Evas_Object *elm_layout_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child);

/**
 * @brief Remove all the child objects of the given part table.
 *
 * The objects will be removed from the table part and their lifetime will not
 * be handled by the layout anymore. This is equivalent to
 * @ref elm_layout_table_unpack for all table children.
 *
 * @param[in] clear If true, then all objects will be deleted as well,
 * otherwise they will just be removed and will be dangling on the canvas.
 *
 * @ingroup Elm_Layout
 */
EAPI Eina_Bool elm_layout_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear);

#include "elm_layout.eo.legacy.h"
