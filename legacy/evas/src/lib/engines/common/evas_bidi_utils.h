#ifndef _EVAS_BIDI_UTILS
#define _EVAS_BIDI_UTILS

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_FRIBIDI
# define USE_FRIBIDI
# define BIDI_SUPPORT
#endif

#include "evas_common.h"

#ifdef USE_FRIBIDI
# include <fribidi/fribidi.h>
#endif

/* abstract fribidi - we statically define sizes here because otherwise we would
 * have to ifdef everywhere (because function decorations may change with/without
 * bidi support)
 * These types should only be passed as pointers! i.e do not directely use any of
 * these types in function declarations. Defining as void should help ensuring that.
 */

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

/* This structure defines a set of properties of a BiDi string. In case of a 
 * non-bidi string, all values should be NULL.
 * To check if a structure describes a bidi string or not, use the macro
 * EVAS_BIDI_IS_BIDI_PROP. RTL-only strings are also treated as bidi ATM.
 */
struct _Evas_BiDi_Props {
   EvasBiDiCharType  *char_types; /* BiDi char types */
   EvasBiDiLevel     *embedding_levels; /* BiDi embedding levels */
#ifdef USE_FRIBIDI
   EvasBiDiParType    direction; /* The paragraph direction, FIXME-tom: should be a 
                                 pointer to the paragraph structure */
#endif
};

typedef struct _Evas_BiDi_Props Evas_BiDi_Props;


#ifdef USE_FRIBIDI

# define EVAS_BIDI_IS_BIDI_PROP(intl_props) ((intl_props) && (intl_props)->char_types)
# define evas_bidi_position_visual_to_logical(list, position) \
                (list) ? list[position] : position;

/* Gets a v_to_l list, it's len and a logical character index, and returns the
 * the visual index of that character.
 */
EvasBiDiStrIndex
evas_bidi_position_logical_to_visual(EvasBiDiStrIndex *v_to_l, int len, EvasBiDiStrIndex position);

/* Returns true if the string has rtl characters, false otherwise */
Eina_Bool
evas_bidi_is_rtl_str(const Eina_Unicode *str);

/* Returns true if the embedding level of the index is rtl, false otherwise */
Eina_Bool
evas_bidi_is_rtl_char(EvasBiDiLevel *embedded_level_list, EvasBiDiStrIndex index);

/* Overallocates a bit, if anyone cares, he should realloc, though usually,
 * the string get freed very fast so there's really no need to care about it 
 * (rellaoc-ing is slower than not)
 */
int
evas_bidi_props_reorder_line(Eina_Unicode *text, const Evas_BiDi_Props *intl_props, EvasBiDiStrIndex **_v_to_l);

/* Updates the international properties according to the text. First checks to see
 * if the text in question has rtl chars, if not, it cleans intl_props and returns.
 * Otherwise, it essentially frees the old fields, allocates new fields, and 
 * populates them.
 * On error, intl_props gets cleaned.
 * Return value: the length of the string.
 */
int
evas_bidi_update_props(Eina_Unicode *text, Evas_BiDi_Props *intl_props) EINA_ARG_NONNULL(1, 2);

/* Cleans and frees the international properties. - Just the content, not the
 * poitner itself. 
 */
void
evas_bidi_props_clean(Evas_BiDi_Props *intl_props) EINA_ARG_NONNULL(1);

#endif

#endif
