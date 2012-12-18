/**
 * @defgroup Plug Plug
 * @ingroup Elementary
 *
 * @image html plug_inheritance_tree.png
 * @image latex plug_inheritance_tree.eps
 *
 * An object that allows one to show an image which other process created.
 * It can be used anywhere like any other elementary widget.
 *
 * This widget emits the following signals:
 * @li "clicked": the user clicked the image (press/release). The @c
 *     event parameter of the callback will be @c NULL.
 * @li "image,deleted": the server side was deleted. The @c event
 *     parameter of the callback will be @c NULL.
 * @li "image,resized": the server side was resized. The @c event parameter of
 *     the callback will be @c Evas_Coord_Size (two integers).
 *
 * @note the event "image,resized" will be sent whenever the server
 *       resized its image and this @b always happen on the first
 *       time. Then it can be used to track when the server-side image
 *       is fully known (client connected to server, retrieved its
 *       image buffer through shared memory and resized the evas
 *       object).
 *
 */

#define ELM_OBJ_PLUG_CLASS elm_obj_plug_class_get()

const Eo_Class *elm_obj_plug_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PLUG_BASE_ID;

enum
{
   ELM_OBJ_PLUG_SUB_ID_IMAGE_OBJECT_GET,
   ELM_OBJ_PLUG_SUB_ID_CONNECT,
   ELM_OBJ_PLUG_SUB_ID_LAST
};

#define ELM_OBJ_PLUG_ID(sub_id) (ELM_OBJ_PLUG_BASE_ID + sub_id)


/**
 * @def elm_obj_plug_image_object_get
 * @since 1.8
 *
 * Get the basic Evas_Image object from this object (widget).
 *
 * @param[out] ret
 *
 * @see elm_plug_image_object_get
 */
#define elm_obj_plug_image_object_get(ret) ELM_OBJ_PLUG_ID(ELM_OBJ_PLUG_SUB_ID_IMAGE_OBJECT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_plug_connect
 * @since 1.8
 *
 * Connect a plug widget to service provided by socket image.
 *
 * @param[in] svcname
 * @param[in] svcnum
 * @param[in] svcsys
 * @param[out] ret
 *
 * @see elm_plug_connect
 */
#define elm_obj_plug_connect(svcname, svcnum, svcsys, ret) ELM_OBJ_PLUG_ID(ELM_OBJ_PLUG_SUB_ID_CONNECT), EO_TYPECHECK(const char *, svcname), EO_TYPECHECK(int, svcnum), EO_TYPECHECK(Eina_Bool, svcsys), EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @addtogroup Plug
 * @{
 */

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
 * @}
 */
