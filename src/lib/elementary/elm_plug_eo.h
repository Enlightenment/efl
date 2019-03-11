#ifndef _ELM_PLUG_EO_H_
#define _ELM_PLUG_EO_H_

#ifndef _ELM_PLUG_EO_CLASS_TYPE
#define _ELM_PLUG_EO_CLASS_TYPE

typedef Eo Elm_Plug;

#endif

#ifndef _ELM_PLUG_EO_TYPES
#define _ELM_PLUG_EO_TYPES


#endif
/** Elementary plug class
 *
 * @ingroup Elm_Plug
 */
#define ELM_PLUG_CLASS elm_plug_class_get()

EWAPI const Efl_Class *elm_plug_class_get(void);

/**
 * @brief Get the basic Evas_Image object from this object (widget).
 *
 * This function allows one to get the underlying Evas object of type Image
 * from this elementary widget. It can be useful to do things like get the
 * pixel data, save the image to a file, etc.
 *
 * @note Be careful to not manipulate it, as it is under control of elementary.
 *
 * @param[in] obj The object.
 *
 * @return The inlined image object or @c null.
 *
 * @ingroup Elm_Plug
 */
EOAPI Efl_Canvas_Object *elm_obj_plug_image_object_get(const Eo *obj);

/**
 * @brief Connect a plug widget to service provided by socket image.
 *
 * @param[in] obj The object.
 * @param[in] svcname The service name to connect to set up by the socket.
 * @param[in] svcnum The service number to connect to (set up by socket).
 * @param[in] svcsys Boolean to set if the service is a system one or not (set
 * up by socket).
 *
 * @return @c true on success, @c false on error.
 *
 * @ingroup Elm_Plug
 */
EOAPI Eina_Bool elm_obj_plug_connect(Eo *obj, const char *svcname, int svcnum, Eina_Bool svcsys);

EWAPI extern const Efl_Event_Description _ELM_PLUG_EVENT_IMAGE_DELETED;

/** Called when image was deleted
 *
 * @ingroup Elm_Plug
 */
#define ELM_PLUG_EVENT_IMAGE_DELETED (&(_ELM_PLUG_EVENT_IMAGE_DELETED))

EWAPI extern const Efl_Event_Description _ELM_PLUG_EVENT_IMAGE_RESIZED;

/** Called when image was resized
 * @return Eina_Position2D
 *
 * @ingroup Elm_Plug
 */
#define ELM_PLUG_EVENT_IMAGE_RESIZED (&(_ELM_PLUG_EVENT_IMAGE_RESIZED))

#endif
