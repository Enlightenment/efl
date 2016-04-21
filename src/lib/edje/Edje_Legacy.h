/**
 * @brief Instantiate a new Edje object
 *
 * @param evas A valid Evas handle, the canvas to place the new object
 * in
 * @return A handle to the new object created or @c NULL, on errors.
 *
 * This function creates a new Edje smart object, returning its @c
 * Evas_Object handle. An Edje object is useless without a (source)
 * file set to it, so you'd most probably call edje_object_file_set()
 * afterwards, like in:
 * @code
 * Evas_Object *edje;
 *
 * edje = edje_object_add(canvas);
 * if (!edje)
 *   {
 *      fprintf(stderr, "could not create edje object!\n");
 *      return NULL;
 *   }
 *
 * if (!edje_object_file_set(edje, "theme.edj", "group_name"))
 *   {
 *      int err = edje_object_load_error_get(edje);
 *      const char *errmsg = edje_load_error_str(err);
 *      fprintf(stderr, "could not load 'group_name' from theme.edj: %s",
 *      	errmsg);
 *
 *      evas_object_del(edje);
 *      return NULL;
 *   }
 *
 * @endcode
 *
 * @note You can get a callback every time edje re-calculates the object
 * (either due to animation or some kind of signal or input). This is called
 * in-line just after the recalculation has occurred. It is a good idea not
 * to go and delete or alter the object inside this callbacks, simply make
 * a note that the recalculation has taken place and then do something about
 * it outside the callback. to register a callback use code like:
 *
 * @code
 *    evas_object_smart_callback_add(edje_obj, "recalc", my_cb, my_cb_data);
 * @endcode
 *
 * @see evas_object_smart_callback_add()
 *
 * @note Before creating the first Edje object in your code, remember
 * to initialize the library, with edje_init(), or unexpected behavior
 * might occur.
 */
EAPI Evas_Object *edje_object_add                 (Evas *evas);

/**
 * @brief Remove a signal-triggered callback from an object.
 *
 * @param obj A valid Evas_Object handle.
 * @param emission The emission string.
 * @param source The source string.
 * @param func The callback function.
 * @return The data pointer
 *
 * This function removes a callback, previously attached to the
 * emission of a signal, from the object @a obj. The parameters @a
 * emission, @a source and @a func must match exactly those passed to
 * a previous call to edje_object_signal_callback_add(). The data
 * pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del_full().
 *
 */
EAPI void        *edje_object_signal_callback_del (Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func);

/**
 * @brief Unregister/delete a callback set for an arriving Edje
 * signal, emitted by a given Edje object.
 *
 * @param obj A handle to an Edje object
 * @param emission The signal's "emission" string
 * @param source The signal's "source" string
 * @param func The callback function passed on the callback's
 * registration
 * @param data The pointer given to be passed as data to @p func
 * @return @p data, on success or @c NULL, on errors (or if @p data
 * had this value)
 *
 * This function removes a callback, previously attached to the
 * emission of a signal, from the object @a obj. The parameters
 * @a emission, @a source, @a func and @a data must match exactly those
 * passed to a previous call to edje_object_signal_callback_add(). The
 * data pointer that was passed to this call will be returned.
 *
 * @see edje_object_signal_callback_add().
 * @see edje_object_signal_callback_del().
 *
 */
EAPI void        *edje_object_signal_callback_del_full(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data);

 /**
 * @brief Set the object minimum size.
 *
 * @param obj A valid Evas_Object handle
 * @param minw The minimum width
 * @param minh The minimum height
 *
 * This sets the minimum size restriction for the object.
 *
 * @deprecated use evas_object_size_hint_min_set() instead.
 */
EINA_DEPRECATED EAPI void         edje_extern_object_min_size_set (Evas_Object *obj, Evas_Coord minw, Evas_Coord minh);

/**
 * @brief Set the object maximum size.
 *
 * @param obj A valid Evas_Object handle
 * @param maxw The maximum width
 * @param maxh The maximum height
 *
 * This sets the maximum size restriction for the object.
 *
 * @deprecated use evas_object_size_hint_max_set() instead.
 */
EAPI void         edje_extern_object_max_size_set (Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh);

/**
 * @brief Set the object aspect size.
 *
 * @param obj A valid Evas_Object handle
 * @param aspect The aspect control axes
 * @param aw The aspect radio width
 * @param ah The aspect ratio height
 *
 * This sets the desired aspect ratio to keep an object that will be
 * swallowed by Edje. The width and height define a preferred size
 * ASPECT and the object may be scaled to be larger or smaller, but
 * retaining the relative scale of both aspect width and height.
 *
 * @deprecated use evas_object_size_hint_aspect_set() instead.
 */
EAPI void         edje_extern_object_aspect_set   (Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah);

/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from
 *
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on errors (check
 * edje_object_load_error_get() after this call to get errors causes)
 *
 * Edje expects EDJ files, which are theming objects' descriptions and
 * resources packed together in an EET file, to read Edje object
 * definitions from. They usually are created with the @c .edj
 * extension. EDJ files, in turn, are assembled from @b textual object
 * description files, where one describes Edje objects declaratively
 * -- the EDC files (see @ref edcref "the syntax" for those files).
 *
 * Those description files were designed so that many Edje object
 * definitions -- also called @b groups (or collections) -- could be
 * packed together <b>in the same EDJ file</b>, so that a whole
 * application's theme could be packed in one file only. This is the
 * reason for the @p group argument.
 *
 * Use this function after you instantiate a new Edje object, so that
 * you can "give him life", telling where to get its contents from.
 *
 * @see edje_object_add()
 * @see edje_object_file_get()
 * @see edje_object_mmap_set()
 *
 * @param[in] file The path to the EDJ file to load @p from
 * @param[in] group The name of the group, in @p file, which implements an
Edje object
 */
EAPI Eina_Bool edje_object_file_set(Edje_Object *obj, const char *file, const char *group);

/**
 * @brief Get the file and group name that a given Edje object is bound to
 *
 * This gets the EDJ file's path, with the respective group set for
 * the given Edje object. If @a obj is either not an Edje file, or has
 * not had its file/group set previously, by edje_object_file_set(),
 * then both @p file and @p group will be set to @c NULL, indicating
 * an error.
 *
 * @see edje_object_file_set()
 *
 * @note Use @c NULL pointers on the file/group components you're not
 * interested in: they'll be ignored by the function.
 *
 * @param[out] file The path to the EDJ file to load @p from
 * @param[out] group The name of the group, in @p file, which implements an
Edje object
 */
EAPI void edje_object_file_get(const Edje_Object *obj, const char **file, const char **group);


/**
 * @brief Sets the @b EDJ file (and group within it) to load an Edje
 * object's contents from
 *
 * @return @c EINA_TRUE, on success or @c EINA_FALSE, on errors (check
 * edje_object_load_error_get() after this call to get errors causes)
 *
 * Edje expects EDJ files, which are theming objects' descriptions and
 * resources packed together in an EET file, to read Edje object
 * definitions from. They usually are created with the @c .edj
 * extension. EDJ files, in turn, are assembled from @b textual object
 * description files, where one describes Edje objects declaratively
 * -- the EDC files (see @ref edcref "the syntax" for those files).
 *
 * Those description files were designed so that many Edje object
 * definitions -- also called @b groups (or collections) -- could be
 * packed together <b>in the same EDJ file</b>, so that a whole
 * application's theme could be packed in one file only. This is the
 * reason for the @p group argument.
 *
 * Use this function after you instantiate a new Edje object, so that
 * you can "give him life", telling where to get its contents from.
 *
 * @see edje_object_add()
 * @see edje_object_file_get()
 * @see edje_object_mmap_set()
 * @since 1.8
 *
 * @param[in] file The Eina.File pointing to the EDJ file to load @p from
 * @param[in] group The name of the group, in @p file, which implements an
Edje object
 */
EAPI Eina_Bool edje_object_mmap_set(Edje_Object *obj, const Eina_File *file, const char *group);

/**
 * @brief "Swallows" an object into one of the Edje object @c SWALLOW parts.
 *
 * Swallowing an object into an Edje object is, for a given part of type
 * @c SWALLOW in the EDC group which gave life to  obj, to set an external
 * object to be controlled by  obj, being displayed exactly over that part's
 * region inside the whole Edje object's viewport.
 *
 * From this point on,  obj will have total control over obj_swallow's geometry
 * and visibility. For instance, if  obj is visible, as in @ref
 * evas_object_show(), the swallowed object will be visible too -- if the given
 * @c SWALLOW part it's in is also visible. Other actions on  obj will also
 * reflect on the swallowed object as well (e.g. resizing, moving,
 * raising/lowering, etc.).
 *
 * Finally, all internal changes to  part, specifically, will reflect on the
 * displaying of  obj_swallow, for example state changes leading to different
 * visibility states, geometries, positions, etc.
 *
 * If an object has already been swallowed into this part, then it will first
 * be unswallowed (as in edje_object_part_unswallow()) before the new object is
 * swallowed.
 *
 * @note  obj  won't delete the swallowed object once it is deleted --
 *  obj_swallow will get to an unparented state again.
 *
 * For more details on EDC @c SWALLOW parts, see @ref edcref "syntax
 * reference".
 *
 * @param[in] obj_swallow The object to occupy that part
 *
 * @ingroup Edje_Object
 */
EAPI Eina_Bool edje_object_part_swallow(Edje_Object *obj, const char *part, Evas_Object *obj_swallow);

/**
 * @brief Get the object currently swallowed by a part.
 *
 * @param[in] part The part name
 *
 * @return The swallowed object, or @c null if there is none.
 *
 * @ingroup Edje_Object
 */
EAPI Evas_Object *edje_object_part_swallow_get(const Edje_Object *obj, const char *part);

/**
 * @brief Unswallow an object.
 *
 * Causes the edje to regurgitate a previously swallowed object. :)
 *
 * @note obj_swallow will  not be deleted or hidden. Note: obj_swallow may
 * appear shown on the evas depending on its state when it got unswallowed.
 * Make sure you delete it or hide it if you do not want it to.
 *
 * @param[in] obj_swallow The swallowed object
 *
 * @ingroup Edje_Object
 */
EAPI void edje_object_part_unswallow(Edje_Object *obj, Evas_Object *obj_swallow);


#include "edje_object.eo.legacy.h"
#include "edje_edit.eo.legacy.h"
