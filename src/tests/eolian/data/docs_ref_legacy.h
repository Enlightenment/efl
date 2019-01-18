#ifndef _EOLIAN_DOCS_EO_LEGACY_H_
#define _EOLIAN_DOCS_EO_LEGACY_H_

#ifndef _DOCS_EO_CLASS_TYPE
#define _DOCS_EO_CLASS_TYPE

typedef Eo Docs;

#endif

#ifndef _DOCS_EO_TYPES
#define _DOCS_EO_TYPES

/**
 * @brief This is struct Foo. It does stuff.
 *
 * @note This is a note.
 *
 * This is a longer description for struct Foo.
 *
 * @warning This is a warning. You can only use Warning: and Note: at the
 * beginning of a paragraph.
 *
 * This is another paragraph.
 *
 * @since 1.66
 *
 * @ingroup Foo
 */
typedef struct _Foo
{
  int field1; /**< Field documentation. */
  float field2;
  short field3; /**< Another field documentation. */
} Foo;

/** Docs for enum Bar.
 *
 * @ingroup Bar
 */
typedef enum
{
  BAR_BLAH = 0,
  BAR_FOO = 1, /**< Docs for foo. */
  BAR_BAR = 2 /**< Docs for bar. */
} Bar;

/**
 * @brief Docs for typedef.
 *
 * More docs for typedef. See @ref Bar.
 *
 * @since 2.0
 *
 * @ingroup Alias
 */
typedef Bar Alias;

/** Docs for var.
 *
 * @ingroup pants
 */
EWAPI extern int PANTS;

/** Opaque struct docs. See @ref Foo for another struct.
 *
 * @ingroup Opaque
 */
typedef struct _Opaque Opaque;


#endif

/**
 * @brief Method documentation.
 *
 * @param[in] obj The object.
 * @param[in] a Param documentation.
 * @param[out] b
 * @param[out] c Another param documentation.
 *
 * @return Return documentation.
 *
 * @since 1.18
 *
 * @ingroup Docs_Group
 */
EAPI int docs_meth(Docs *obj, int a, float *b, long *c);

/**
 * @brief Property common documentation.
 *
 * Set documentation.
 *
 * @param[in] obj The object.
 * @param[in] val Value documentation.
 *
 * @since 1.18
 *
 * @ingroup Docs_Group
 */
EAPI void docs_prop_set(Docs *obj, int val);

/**
 * @brief Property common documentation.
 *
 * Get documentation.
 *
 * @param[in] obj The object.
 *
 * @return Value documentation.
 *
 * @since 1.18
 *
 * @ingroup Docs_Group
 */
EAPI int docs_prop_get(const Docs *obj);

#endif
