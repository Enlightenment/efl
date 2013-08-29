#define ELM_OBJ_BG_CLASS elm_obj_bg_class_get()

const Eo_Class *elm_obj_bg_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_BG_BASE_ID;

enum
{
   ELM_OBJ_BG_SUB_ID_FILE_SET,
   ELM_OBJ_BG_SUB_ID_FILE_GET,
   ELM_OBJ_BG_SUB_ID_OPTION_SET,
   ELM_OBJ_BG_SUB_ID_OPTION_GET,
   ELM_OBJ_BG_SUB_ID_COLOR_SET,
   ELM_OBJ_BG_SUB_ID_COLOR_GET,
   ELM_OBJ_BG_SUB_ID_LOAD_SIZE_SET,
   ELM_OBJ_BG_SUB_ID_LAST
};

#define ELM_OBJ_BG_ID(sub_id) (ELM_OBJ_BG_BASE_ID + sub_id)


/**
 * @def elm_obj_bg_file_set
 * @since 1.8
 *
 * Set the file (image or edje collection) to give life for the
 * background
 *
 * @param[in] file
 * @param[in] group
 * @param[out] ret
 *
 * @see elm_bg_file_set
 *
 * @ingroup Bg
 */
#define elm_obj_bg_file_set(file, group, ret) ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_FILE_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, group), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_bg_file_get
 * @since 1.8
 *
 * Get the file (image or edje collection) set on a given background
 *
 * @param[out] file
 * @param[out] group
 *
 * @see elm_bg_file_get
 *
 * @ingroup Bg
 */
#define elm_obj_bg_file_get(file, group) ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_FILE_GET), EO_TYPECHECK(const char **, file), EO_TYPECHECK(const char **, group)

/**
 * @def elm_obj_bg_option_set
 * @since 1.8
 *
 * Set the mode of display for a given background widget's image
 *
 * @param[in] option
 *
 * @see elm_bg_option_set
 *
 * @ingroup Bg
 */
#define elm_obj_bg_option_set(option) ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_OPTION_SET), EO_TYPECHECK(Elm_Bg_Option, option)

/**
 * @def elm_obj_bg_option_get
 * @since 1.8
 *
 * Get the mode of display for a given background widget's image
 *
 * @param[out] ret
 *
 * @see elm_bg_option_get
 *
 * @ingroup Bg
 */
#define elm_obj_bg_option_get(ret) ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_OPTION_GET), EO_TYPECHECK(Elm_Bg_Option *, ret)

/**
 * @def elm_obj_bg_color_set
 * @since 1.8
 *
 * Set the color on a given background widget
 *
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 *
 * @see elm_bg_color_set
 *
 * @ingroup Bg
 */
#define elm_obj_bg_color_set(r, g, b, a) ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def elm_obj_bg_color_get
 * @since 1.8
 *
 * Get the color set on a given background widget
 *
 * @param[out] r
 * @param[out] g
 * @param[out] b
 * @param[out] a
 *
 * @see elm_bg_color_get
 *
 * @ingroup Bg
 */
#define elm_obj_bg_color_get(r, g, b, a) ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def elm_obj_bg_load_size_set
 * @since 1.8
 *
 * Set the size of the pixmap representation of the image set on a
 * given background widget.
 *
 * @param[in] w
 * @param[in] h
 *
 * @see elm_bg_load_size_set
 *
 * @ingroup Bg
 */
#define elm_obj_bg_load_size_set(w, h) ELM_OBJ_BG_ID(ELM_OBJ_BG_SUB_ID_LOAD_SIZE_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)
