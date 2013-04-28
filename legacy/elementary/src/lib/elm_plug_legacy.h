/**
 * Add a new plug image to the parent.
 *
 * @param parent The parent object
 * @return The new plug image object or NULL if it cannot be created
 *
 * @ingroup Plug
 */
EAPI Evas_Object    *elm_plug_add(Evas_Object *parent);

/**
 * Get the basic Evas_Image object from this object (widget).
 *
 * @param obj The image object to get the inlined image from
 * @return The inlined image object, or NULL if none exists
 *
 * This function allows one to get the underlying @c Evas_Object of type
 * Image from this elementary widget. It can be useful to do things like get
 * the pixel data, save the image to a file, etc.
 *
 * @note Be careful to not manipulate it, as it is under control of
 * elementary.
 *
 * @ingroup Plug
 */
EAPI Evas_Object    *elm_plug_image_object_get(const Evas_Object *obj);

/**
 * Connect a plug widget to service provided by socket image.
 *
 * @param obj The Evas_Object where the new image object will live.
 * @param svcname The service name to connect to set up by the socket.
 * @param svcnum The service number to connect to (set up by socket).
 * @param svcsys Boolean to set if the service is a system one or not (set up by socket).
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @ingroup Plug
 */
EAPI Eina_Bool       elm_plug_connect(Evas_Object *obj, const char *svcname, int svcnum, Eina_Bool svcsys);
