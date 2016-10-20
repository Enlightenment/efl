#ifndef _EOLIAN_DOCS_H_
#define _EOLIAN_DOCS_H_

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
 * @brief Docs for class.
 *
 * More docs for class. Testing references now. @ref Foo @ref Bar @ref Alias
 * @ref pants @ref docs_meth @ref docs_prop_get @ref docs_prop_get
 * @ref docs_prop_set @ref Foo.field1 @ref Bar.BAR_FOO @ref Docs
 *
 * @since 1.18
 *
 * @ingroup Docs
 */
#define DOCS_CLASS docs_class_get()

EWAPI const Efl_Class *docs_class_get(void);

/**
 * @brief Property common documentation.
 *
 * Set documentation.
 *
 * @param[in] val Value documentation.
 *
 * @since 1.18
 *
 * @ingroup Docs
 */
EOAPI void docs_prop_set(Eo *obj, int val);

/**
 * @brief Property common documentation.
 *
 * Get documentation.
 *
 * @return Value documentation.
 *
 * @since 1.18
 *
 * @ingroup Docs
 */
EOAPI int docs_prop_get(const Eo *obj);

/**
 * @brief Method documentation.
 *
 * @param[in] a Param documentation.
 * @param[out] b
 * @param[out] c Another param documentation.
 *
 * @return Return documentation.
 *
 * @since 1.18
 *
 * @ingroup Docs
 */
EOAPI int docs_meth(Eo *obj, int a, float *b, long *c);

EWAPI extern const Efl_Event_Description _DOCS_EVENT_CLICKED;

/** Event docs.
 *
 * @since 1.18
 *
 * @ingroup Docs
 */
#define DOCS_EVENT_CLICKED (&(_DOCS_EVENT_CLICKED))

#endif
