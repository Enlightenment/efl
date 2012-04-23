#ifndef EVAS_OBJ_H
#define EVAS_OBJ_H

#include "Eobj.h"

extern EAPI Eobj_Op EVAS_OBJ_BASE_ID;

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
 * @def EVAS_OBJ_POSITION_SET(x, y)
 * @brief Set object's position
 * @param[in] x object's X position
 * @param[in] y object's Y position
 */
#define EVAS_OBJ_POSITION_SET(x, y) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_POSITION_SET), EOBJ_TYPECHECK(Evas_Coord, x), EOBJ_TYPECHECK(Evas_Coord, y)

/**
 * @def EVAS_OBJ_SIZE_SET(w, h)
 * @brief Set object's size
 * @param[in] w object's width
 * @param[in] h object's height
 */
#define EVAS_OBJ_SIZE_SET(w, h) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_SIZE_SET), EOBJ_TYPECHECK(Evas_Coord, w), EOBJ_TYPECHECK(Evas_Coord, h)

/**
 * @def EVAS_OBJ_COLOR_SET(r, g, b, a)
 * @brief Set object's color
 * @param[in] r r-value of color
 * @param[in] g g-value of color
 * @param[in] b b-value of color
 * @param[in] a a-value of color
 */
#define EVAS_OBJ_COLOR_SET(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_SET), EOBJ_TYPECHECK(int, r), EOBJ_TYPECHECK(int, g), EOBJ_TYPECHECK(int, b), EOBJ_TYPECHECK(int, a)

/**
 * @def EVAS_OBJ_COLOR_GET(r, g, b, a)
 * @brief Set object's position
 * @param[out] r integer pointer for r-value of color
 * @param[out] g integer pointer for g-value of color
 * @param[out] b integer pointer for b-value of color
 * @param[out] a integer pointer for a-value of color
 */
#define EVAS_OBJ_COLOR_GET(r, g, b, a) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_COLOR_GET), EOBJ_TYPECHECK(int *, r), EOBJ_TYPECHECK(int *, g), EOBJ_TYPECHECK(int *, b), EOBJ_TYPECHECK(int *, a)

/**
 * @def EVAS_OBJ_VISIBILITY_SET(v)
 * @brief Set object's visible property
 * @param[in] v True/False value
 */
#define EVAS_OBJ_VISIBILITY_SET(v) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_VISIBILITY_SET), EOBJ_TYPECHECK(Eina_Bool, v)

/**
 * @def EVAS_OBJ_CHILD_ADD(child)
 * @brief Add child to current object
 * @param[in] pointer to child object
 */
#define EVAS_OBJ_CHILD_ADD(child) EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_CHILD_ADD), EOBJ_TYPECHECK(Eobj *, child)

#define EVAS_OBJ_CLASS evas_object_class_get()
const Eobj_Class *evas_object_class_get(void) EINA_CONST;

#define EVAS_OBJ_STR "Evas_Obj"
/* FIXME: Hack in the meanwhile. */
static inline Evas_Object *
eobj_evas_object_get(Eobj *obj)
{
   void *data;
   eobj_do(obj, EOBJ_BASE_DATA_GET(EVAS_OBJ_STR, &data));
   return data;
}

/* FIXME: Hack in the meanwhile. */
static inline void
eobj_evas_object_set(Eobj *obj, Evas_Object *evas_obj)
{
   eobj_do(obj, EOBJ_BASE_DATA_SET(EVAS_OBJ_STR, evas_obj, NULL));
}

#endif
