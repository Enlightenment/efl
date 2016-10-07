#ifndef _EOLIAN_TYPEDEF_H_
#define _EOLIAN_TYPEDEF_H_

#ifndef _STRUCT_EO_CLASS_TYPE
#define _STRUCT_EO_CLASS_TYPE

typedef Eo Struct;

#endif

#ifndef _STRUCT_EO_TYPES
#define _STRUCT_EO_TYPES

typedef struct _Named
{
  int *field;
  const char *something;
  int arr[16];
  const char **tarr;
} Named;

typedef struct _Another
{
  Named field;
} Another;

typedef struct _Opaque Opaque;


#endif
#define STRUCT_CLASS struct_class_get()

EWAPI const Efl_Class *struct_class_get(void);

/**
 * @brief Foo docs. This is @c monospace. This is alone-standing $.
 *
 * @param[in] idx
 *
 * @ingroup Struct
 */
EOAPI char *struct_foo(Eo *obj, int idx);

EOAPI Named *struct_bar(Eo *obj);

#endif
