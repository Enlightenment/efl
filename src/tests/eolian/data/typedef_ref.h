#ifndef _EOLIAN_TYPEDEF_EO_H_
#define _EOLIAN_TYPEDEF_EO_H_

#ifndef _TYPEDEF_EO_CLASS_TYPE
#define _TYPEDEF_EO_CLASS_TYPE

typedef Eo Typedef;

#endif

#ifndef _TYPEDEF_EO_TYPES
#define _TYPEDEF_EO_TYPES

/** Docs for typedef Evas.Coord.
 *
 * @since 1.66
 *
 * @ingroup Evas_Coord
 */
typedef int Evas_Coord;

/** Docs for typedef List_Objects.
 *
 * @since 1.66
 *
 * @ingroup List_Objects
 */
typedef Eina_Iterator *List_Objects;

/** Docs for typedef Evas.Coord2.
 *
 * @since 1.66
 *
 * @ingroup Evas_Coord2
 */
typedef Evas_Coord Evas_Coord2;

/** Docs for typedef Evas.Coord3.
 *
 * @since 1.66
 *
 * @ingroup Evas_Coord3
 */
typedef Evas_Coord2 Evas_Coord3;

/** Docs for enum Elm.Object.Select_Mode.
 *
 * @since 1.66
 *
 * @ingroup Elm_Object_Select_Mode
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
/** Docs for class Typedef.
 *
 * @since 1.66
 *
 * @ingroup Typedef
 */
#define TYPEDEF_CLASS typedef_class_get()

EWAPI const Efl_Class *typedef_class_get(void) EINA_CONST;

EOAPI char *typedef_foo(Eo *obj, int idx) EFL_TRANSFER_OWNERSHIP EINA_WARN_UNUSED_RESULT;

#endif
