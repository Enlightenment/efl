#ifndef _EOLIAN_TYPEDEF_EO_H_
#define _EOLIAN_TYPEDEF_EO_H_

#ifndef _TYPEDEF_EO_CLASS_TYPE
#define _TYPEDEF_EO_CLASS_TYPE

typedef Eo Typedef;

#endif

#ifndef _TYPEDEF_EO_TYPES
#define _TYPEDEF_EO_TYPES

typedef int Evas_Coord;

typedef Eina_Iterator *List_Objects;

typedef Evas_Coord Evas_Coord2;

typedef Evas_Coord2 Evas_Coord3;

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

EWAPI const Efl_Class *typedef_class_get(void);

EOAPI char *typedef_foo(Eo *obj, int idx) EFL_TRANSFER_OWNERSHIP EINA_WARN_UNUSED_RESULT;

#endif
