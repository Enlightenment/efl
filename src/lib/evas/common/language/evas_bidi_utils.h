#ifndef _EVAS_BIDI_UTILS
#define _EVAS_BIDI_UTILS

/**
 * @internal
 * @addtogroup Evas_Utils
 *
 * @{
 */
/**
 * @internal
 * @defgroup Evas_BiDi Evas BiDi utility functions
 *
 * This set of functions and types helps evas handle BiDi strings correctly.
 * @todo Document types, structures and macros.
 *
 * @{
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_FRIBIDI
# define USE_FRIBIDI
# define BIDI_SUPPORT
#endif

#include <Eina.h>

#ifdef USE_FRIBIDI
# include <fribidi.h>
#endif

/* abstract fribidi - we statically define sizes here because otherwise we would
 * have to ifdef everywhere (because function decorations may change with/without
 * bidi support)
 * These types should only be passed as pointers! i.e do not directely use any of
 * these types in function declarations. Defining as void should help ensuring that.
 */

/* Evas_BiDi_Direction is defined in evas.h */

#ifdef USE_FRIBIDI
# define _EVAS_BIDI_TYPEDEF(type) \
   typedef FriBidi ## type EvasBiDi ## type
#else
# define _EVAS_BIDI_TYPEDEF(type) \
   typedef void EvasBiDi ## type
#endif

#if 0 /* We are using Eina_Unicode instead */
_EVAS_BIDI_TYPEDEF(Char);
#endif
_EVAS_BIDI_TYPEDEF(CharType);
_EVAS_BIDI_TYPEDEF(ParType);
_EVAS_BIDI_TYPEDEF(StrIndex);
_EVAS_BIDI_TYPEDEF(Level);
_EVAS_BIDI_TYPEDEF(JoiningType);
#if FRIBIDI_MAJOR_VERSION >= 1
_EVAS_BIDI_TYPEDEF(BracketType);
#endif

typedef struct _Evas_BiDi_Paragraph_Props Evas_BiDi_Paragraph_Props;
typedef struct _Evas_BiDi_Props Evas_BiDi_Props;

/* This structure defines a set of properties of a BiDi string. In case of a
 * non-bidi string, all values should be NULL.
 * To check if a structure describes a bidi string or not, use the macro
 * EVAS_BIDI_IS_BIDI_PROP. RTL-only strings are also treated as bidi ATM.
 */
struct _Evas_BiDi_Paragraph_Props {
   EvasBiDiCharType  *char_types; /* BiDi char types */
   EvasBiDiLevel     *embedding_levels; /* BiDi embedding levels */
   int                refcount; /* The number of references to this object */
#ifdef USE_FRIBIDI
   EvasBiDiParType    direction;
#endif
};

#include "Evas.h"
struct _Evas_BiDi_Props
{
   Evas_BiDi_Direction  dir;
};



#ifdef USE_FRIBIDI

#define EVAS_BIDI_PARAGRAPH_NEUTRAL FRIBIDI_PAR_ON
#define EVAS_BIDI_PARAGRAPH_LTR     FRIBIDI_PAR_LTR
#define EVAS_BIDI_PARAGRAPH_RTL     FRIBIDI_PAR_RTL
#define EVAS_BIDI_PARAGRAPH_WLTR    FRIBIDI_PAR_WLTR
#define EVAS_BIDI_PARAGRAPH_WRTL    FRIBIDI_PAR_WRTL

#define EVAS_BIDI_DIRECTION_IS_RTL(x)       \
   ((((x) == EVAS_BIDI_PARAGRAPH_RTL) ||   \
     ((x) == EVAS_BIDI_PARAGRAPH_WRTL)) ?   \
    EINA_TRUE : EINA_FALSE)

#define EVAS_BIDI_PARAGRAPH_DIRECTION_IS_RTL(x)       \
   ((x) && EVAS_BIDI_DIRECTION_IS_RTL((x)->direction))

#define EVAS_BIDI_PAR_TYPE_TO_DIRECTION(x) \
   _evas_bidi_par_type_to_direction(x)

static inline Evas_BiDi_Direction
_evas_bidi_par_type_to_direction(EvasBiDiParType par_type)
{
   switch (par_type)
     {
      case EVAS_BIDI_PARAGRAPH_LTR:
      case EVAS_BIDI_PARAGRAPH_WLTR:
         return EVAS_BIDI_DIRECTION_LTR;
         break;
      case EVAS_BIDI_PARAGRAPH_RTL:
      case EVAS_BIDI_PARAGRAPH_WRTL:
         return EVAS_BIDI_DIRECTION_RTL;
         break;
      case EVAS_BIDI_PARAGRAPH_NEUTRAL:
      default:
         return EVAS_BIDI_DIRECTION_NEUTRAL;
         break;
     }
}

# define evas_bidi_position_visual_to_logical(list, position) \
                (list) ? list[position] : position;

EvasBiDiStrIndex
evas_bidi_position_logical_to_visual(EvasBiDiStrIndex *v_to_l, int len, EvasBiDiStrIndex position);

EvasBiDiStrIndex
evas_bidi_position_reverse(const Evas_BiDi_Props *props, int len, EvasBiDiStrIndex position);

Eina_Bool
evas_bidi_is_rtl_str(const Eina_Unicode *str);

Eina_Bool
evas_bidi_is_rtl_char(const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, EvasBiDiStrIndex index);

int
evas_bidi_end_of_run_get(const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, int len);

Eina_Bool
evas_bidi_props_reorder_line(Eina_Unicode *eina_ustr, size_t start, size_t len, const Evas_BiDi_Paragraph_Props *props, EvasBiDiStrIndex **_v_to_l);

Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_get(const Eina_Unicode *eina_ustr, size_t len, int *segment_idxs, EvasBiDiParType base_bidi) EINA_ARG_NONNULL(1) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

void
evas_bidi_props_copy_and_ref(const Evas_BiDi_Props *src, Evas_BiDi_Props *dst);

EAPI Eina_Bool
evas_bidi_shape_string(Eina_Unicode *eina_ustr, const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, size_t len);

void
evas_bidi_props_clean(Evas_BiDi_Props *intl_props) EINA_ARG_NONNULL(1);

void
evas_bidi_paragraph_props_clean(Evas_BiDi_Paragraph_Props *bidi_props) EINA_ARG_NONNULL(1);

Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_ref(Evas_BiDi_Paragraph_Props *bidi_props) EINA_ARG_NONNULL(1);

void
evas_bidi_paragraph_props_unref(Evas_BiDi_Paragraph_Props *bidi_props) EINA_ARG_NONNULL(1);

Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_new(void) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

int *
evas_bidi_segment_idxs_get(const Eina_Unicode *str, const char *delim) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

#endif
/**
 * @}
 */
/**
 * @}
 */

#endif

