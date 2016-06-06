#ifndef _TYPES_OUTPUT_C_
#define _TYPES_OUTPUT_C_

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
} Named;

typedef struct _Another
{
  Named field;
} Another;

typedef struct _Opaque Opaque;


#endif
#define STRUCT_CLASS struct_class_get()

EWAPI const Eo_Class *struct_class_get(void);

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
