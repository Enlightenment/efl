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
 *
 * @ingroup Plug
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
 *
 * @ingroup Plug
 */
#define elm_obj_plug_connect(svcname, svcnum, svcsys, ret) ELM_OBJ_PLUG_ID(ELM_OBJ_PLUG_SUB_ID_CONNECT), EO_TYPECHECK(const char *, svcname), EO_TYPECHECK(int, svcnum), EO_TYPECHECK(Eina_Bool, svcsys), EO_TYPECHECK(Eina_Bool *, ret)
