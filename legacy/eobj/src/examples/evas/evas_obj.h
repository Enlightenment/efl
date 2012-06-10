#ifndef EVAS_OBJ_H
#define EVAS_OBJ_H

#include "Eo.h"

extern EAPI Eo_Op EVAS_OBJ_BASE_ID;

enum {
     EVAS_OBJ_SUB_ID_POSITION_SET,
     EVAS_OBJ_SUB_ID_SIZE_SET,
     EVAS_OBJ_SUB_ID_COLOR_SET,
     EVAS_OBJ_SUB_ID_COLOR_GET,
     EVAS_OBJ_SUB_ID_VISIBILITY_SET,
     EVAS_OBJ_SUB_ID_CHILD_ADD,
     EVAS_OBJ_SUB_ID_LAST
};

#define EVAS_OBJ_ID(sub_id) (EVAS_OBJ_BASE_ID + sub_id)

/**
 * @def evas_obj_position_set(x, y)
 * @brief Set object's position
 * @param[in] x object's X position
 * @param[in] y object's Y position
 */
#define evas_obj_position_set(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_SET), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def evas_obj_size_set(w, h)
 * @brief Set object's size
 * @param[in] w object's width
 * @param[in] h object's height
 */
#define evas_obj_size_set(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_SET), EO_TYPECHECK(Evas_Coord, w), EO_TYPECHECK(Evas_Coord, h)

/**
 * @def evas_obj_color_set(r, g, b, a)
 * @brief Set object's color
 * @param[in] r r-value of color
 * @param[in] g g-value of color
 * @param[in] b b-value of color
 * @param[in] a a-value of color
 */
#define evas_obj_color_set(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def evas_obj_color_get(r, g, b, a)
 * @brief Set object's position
 * @param[out] r integer pointer for r-value of color
 * @param[out] g integer pointer for g-value of color
 * @param[out] b integer pointer for b-value of color
 * @param[out] a integer pointer for a-value of color
 */
#define evas_obj_color_get(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def evas_obj_visibility_set(v)
 * @brief Set object's visible property
 * @param[in] v True/False value
 */
#define evas_obj_visibility_set(v) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_SET), EO_TYPECHECK(Eina_Bool, v)

/**
 * @def evas_obj_child_add(child)
 * @brief Add child to current object
 * @param[in] pointer to child object
 */
#define evas_obj_child_add(child) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CHILD_ADD), EO_TYPECHECK(Eo *, child)

#define EVAS_OBJ_CLASS evas_object_class_get()
const Eo_Class *evas_object_class_get(void);

#define EVAS_OBJ_STR "Evas_Obj"
/* FIXME: Hack in the meanwhile. */
static inline Evas_Object *
eo_evas_object_get(const Eo *obj)
{
   void *data;
   eo_query(obj, eo_base_data_get(EVAS_OBJ_STR, &data));
   return data;
}

/* FIXME: Hack in the meanwhile. */
static inline void
eo_evas_object_set(Eo *obj, Evas_Object *evas_obj)
{
   eo_do(obj, eo_base_data_set(EVAS_OBJ_STR, evas_obj, NULL));
}

#endif
