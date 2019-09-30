#ifndef _EOLIAN_TYPEDEF_EO_H_
#define _EOLIAN_TYPEDEF_EO_H_

#ifndef _TYPEDEF_EO_CLASS_TYPE
#define _TYPEDEF_EO_CLASS_TYPE

typedef Eo Typedef;

#endif

#ifndef _TYPEDEF_EO_TYPES
#define _TYPEDEF_EO_TYPES

/** No description supplied.
 *
 * @ingroup Evas
 */
typedef int Evas_Coord;

/** No description supplied.
 *
 * @ingroup List_Objects
 */
typedef Eina_List *List_Objects;

/** No description supplied.
 *
 * @ingroup Evas
 */
typedef Evas_Coord Evas_Coord2;

/** No description supplied.
 *
 * @ingroup Evas
 */
typedef Evas_Coord2 Evas_Coord3;

/** No description supplied.
 *
 * @ingroup Elm_Object
 */
typedef enum
{
  ELM_OBJECT_SELECT_MODE_DEFAULT = 0,
  ELM_OBJECT_SELECT_MODE_ALWAYS,
  ELM_OBJECT_SELECT_MODE_NONE,
  ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY,
  ELM_OBJECT_SELECT_MODE_MAX
} Elm_Object_Select_Mode;


#endif
/** No description supplied.
 *
 * @ingroup Typedef
 */
#define TYPEDEF_CLASS typedef_class_get()

EWAPI const Efl_Class *typedef_class_get(void);

/**
 * @brief No description supplied.
 *
 * @param[in] obj The object.
 * @param[in] idx
 *
 * @ingroup Typedef
 */
EOAPI char *typedef_foo(Eo *obj, int idx) EFL_TRANSFER_OWNERSHIP;

#endif
