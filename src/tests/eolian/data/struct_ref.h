#ifndef _EOLIAN_TYPEDEF_EO_H_
#define _EOLIAN_TYPEDEF_EO_H_

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

EWAPI const Efl_Class *struct_class_get(void);

/**
 * @brief Foo docs. This is @c monospace. This is alone-standing $.
 *
 * @param[in] obj The object.
 * @param[in] idx
 *
 * @ingroup Struct
 */
EOAPI char *struct_foo(Eo *obj, int idx) EFL_TRANSFER_OWNERSHIP;

EOAPI Named *struct_bar(Eo *obj);

#endif
