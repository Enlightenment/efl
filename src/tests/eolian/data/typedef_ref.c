#ifndef _TYPES_OUTPUT_C_
#define _TYPES_OUTPUT_C_

#ifndef _TYPEDEF_EO_CLASS_TYPE
#define _TYPEDEF_EO_CLASS_TYPE

typedef Eo Typedef;

#endif

#ifndef _TYPEDEF_EO_TYPES
#define _TYPEDEF_EO_TYPES

typedef int Evas_Coord;

typedef Eina_List *List_Objects;

typedef enum
{
  BAR_FIRST_ITEM = 0,
  BAR_SECOND_ITEM,
  BAR_LAST_ITEM
} Enum_Bar;

typedef enum
{
  ELM_OBJECT_SELECT_MODE_DEFAULT = 0,
  ELM_OBJECT_SELECT_MODE_ALWAYS,
  ELM_OBJECT_SELECT_MODE_NONE,
  ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY,
  ELM_OBJECT_SELECT_MODE_MAX
} Elm_Object_Select_Mode;


#endif
#define TYPEDEF_CLASS typedef_class_get()

EAPI const Eo_Class *typedef_class_get(void) EINA_CONST;

/**
 *
 * No description supplied.
 *
 * @param[in] idx No description supplied.
 *
 */
EOAPI char * typedef_foo(int idx);


#endif
