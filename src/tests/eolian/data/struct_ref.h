#ifndef _EOLIAN_TYPEDEF_EO_H_
#define _EOLIAN_TYPEDEF_EO_H_

#ifndef _STRUCT_EO_CLASS_TYPE
#define _STRUCT_EO_CLASS_TYPE

typedef Eo Struct;

#endif

#ifndef _STRUCT_EO_TYPES
#define _STRUCT_EO_TYPES

/** Docs for struct Named.
 *
 * @since 1.66
 *
 * @ingroup Named
 */
typedef struct _Named
{
  int *field;
  const char *something;
} Named;

/** Docs for struct Another.
 *
 * @since 1.66
 *
 * @ingroup Another
 */
typedef struct _Another
{
  Named field;
} Another;

/** Docs for struct Opaque.
 *
 * @since 1.66
 *
 * @ingroup Opaque
 */
typedef struct _Opaque Opaque;


#endif
/** Docs for class Struct.
 *
 * @since 1.66
 *
 * @ingroup Struct
 */
#define STRUCT_CLASS struct_class_get()

EAPI EAPI_WEAK const Efl_Class *struct_class_get(void) EINA_CONST;

/**
 * @brief Foo docs. This is @c monospace. This is alone-standing $.
 *
 * @param[in] obj The object.
 * @param[in] idx
 *
 * @since 1.66
 *
 * @ingroup Struct
 */
EAPI EAPI_WEAK char *struct_foo(Eo *obj, int idx) EFL_TRANSFER_OWNERSHIP EINA_WARN_UNUSED_RESULT;

EAPI EAPI_WEAK Named *struct_bar(Eo *obj);

#endif
