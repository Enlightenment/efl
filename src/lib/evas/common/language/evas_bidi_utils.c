#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "evas_common_private.h"
#include "evas_bidi_utils.h"

#include "evas_font_private.h"

#ifdef BIDI_SUPPORT
#include <fribidi.h>
/**
 * @internal
 * @addtogroup Evas_Utils
 *
 * @{
 */
/**
 * @internal
 * @addtogroup Evas_BiDi
 *
 * @{
 */

/**
 * @internal
 * @def _SAFE_FREE(x)
 * checks if x is not NULL, if it's not, it's freed and set to NULL.
 */
#define _SAFE_FREE(x) \
   do {               \
      if (x)          \
        {             \
           free(x);   \
           x = NULL;  \
        }             \
     } while(0)

#if SIZEOF_FRIBIDICHAR != SIZEOF_EINA_UNICODE
# define EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
#endif

#ifdef EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
/* Convert bidichar to eina_unicode assume both are valid pointers */
static Eina_Unicode *
_evas_bidi_fribidichar_to_unicode(Eina_Unicode *dest, const FriBidiChar *src)
{
   Eina_Unicode *ret = dest;

   if ((!src) || (!dest)) return NULL;
   while (*src) *dest++ = *src++;
   *dest = 0;
   return ret;
}

/* Convert eina_unicode to bidi_char assume both are valid pointers */
static FriBidiChar *
_evas_bidi_unicode_to_fribidichar(FriBidiChar *dest, const Eina_Unicode *src)
{
   FriBidiChar *ret = dest;

   if ((!src) || (!dest)) return NULL;
   while (*src) *dest++ = *src++;
   *dest = 0;
   return ret;
}
#endif

/**
 * @internal
 * Checks if the string has RTL characters.
 *
 * @param str The string to be checked
 * @return #EINA_TRUE if true, #EINA_FALSE otherwise.
 */
Eina_Bool
evas_bidi_is_rtl_str(const Eina_Unicode *str)
{
   EvasBiDiCharType type;

   if (!str)
      return EINA_FALSE;

   for ( ; *str ; str++)
     {
        type = fribidi_get_bidi_type((FriBidiChar) *str);
        if (FRIBIDI_IS_RTL(type))
          {
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

/**
 * @internal
 * Shapes the string ustr according to the bidi properties.
 *
 * @param str The string to shape
 * @param bidi_props the bidi props to shaped according.
 * @param start the start of the string to shape (offset in bidi_props)
 * @param len the length of th string.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 */
EAPI Eina_Bool
evas_bidi_shape_string(Eina_Unicode *eina_ustr, const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, size_t len)
{
   FriBidiChar *ustr, *base_ustr = NULL;

   if (!bidi_props)
     return EINA_FALSE;

   /* The size of fribidichar is different than eina_unicode, convert */
#ifdef EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
   base_ustr = ustr = calloc(len + 1, sizeof(FriBidiChar));
   ustr = _evas_bidi_unicode_to_fribidichar(ustr, eina_ustr);
#else
   (void) base_ustr;
   ustr = (FriBidiChar *) eina_ustr;
#endif


   EvasBiDiJoiningType *join_types = NULL;
   join_types = (EvasBiDiJoiningType *) malloc(sizeof(EvasBiDiJoiningType) * len);
   if (!join_types)
     {
#ifdef EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
        if (base_ustr) free(base_ustr);
#endif
        return EINA_FALSE;
     }
   fribidi_get_joining_types(ustr, len, join_types);

   fribidi_join_arabic(bidi_props->char_types + start, len,
         bidi_props->embedding_levels + start, join_types);


   /* Actually modify the string */
   fribidi_shape(FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC,
               bidi_props->embedding_levels + start, len, join_types, ustr);

   if (join_types) free(join_types);

   /* Convert back */
#ifdef EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
   _evas_bidi_fribidichar_to_unicode(eina_ustr, ustr);
   if (base_ustr) free(base_ustr);
#endif
   return EINA_TRUE;
}

/**
 * @internal
 * Return a -1 terminated array of the indexes of the delimiters (passed in
 * delim) found in the string. This result should be used with par_props_get.
 *
 * @param str The string to parse
 * @param delim a list of delimiters to work with.
 * @return returns a -1 terminated array of indexes according to positions of the delimiters found. NULL if there were none.
 */
int *
evas_bidi_segment_idxs_get(const Eina_Unicode *str, const char *delim)
{
   Eina_Unicode *udelim;
   const Eina_Unicode *str_base = str;
   int *ret, *tmp_ret;
   int ret_idx = 0, ret_len = 10; /* arbitrary choice */
   udelim = eina_unicode_utf8_to_unicode(delim, NULL);
   ret = malloc(ret_len * sizeof(int));
   for ( ; *str ; str++)
     {
        const Eina_Unicode *del;
        for (del = udelim ; *del ; del++)
          {
             if (*str == *del)
               {
                  if (ret_idx >= ret_len)
                    {
                       /* arbitrary choice */
                       ret_len += 20;
                       tmp_ret = realloc(ret, ret_len * sizeof(int));
                       if (!tmp_ret)
                         {
                            free(ret);
                            free(udelim);
                            return NULL;
                         }
                       ret = tmp_ret;
                    }
                  ret[ret_idx++] = str - str_base;
                  break;
               }
          }
     }
   free(udelim);

   /* If no indexes were found return NULL */
   if (ret_idx == 0)
     {
        free(ret);
        return NULL;
     }

   ret[ret_idx] = -1;
   tmp_ret = realloc(ret, (ret_idx + 1) * sizeof(int));

   return (tmp_ret) ? tmp_ret : ret;
}

/**
 * @internal
 * Allocates bidi properties according to ustr. First checks to see if the
 * passed has rtl chars, if not, it returns NULL.
 *
 * Assumes all the segment_idxs are either -1 or legal, and > 0 indexes.
 * Also assumes that the characters at the override points are of weak/neutral
 * bidi type, otherwise unexpected results may occur.
 *
 * @param ustr The string to update according to.
 * @param len The length of the string
 * @param segment_idxs A -1 terminated array of points to start a new bidi analysis at (used for section high level bidi overrides). - NULL means none.
 * @param base_bidi The base BiDi direction of paragraph.
 * @return returns allocated paragraph props on success, NULL otherwise.
 */
Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_get(const Eina_Unicode *eina_ustr, size_t len,
      int *segment_idxs, EvasBiDiParType base_bidi)
{
   Evas_BiDi_Paragraph_Props *bidi_props = NULL;
   EvasBiDiCharType *char_types = NULL;
   EvasBiDiLevel *embedding_levels = NULL;
   const FriBidiChar *ustr;
   FriBidiChar *base_ustr = NULL;
   EvasBiDiLevel ret_level = 0;
#if FRIBIDI_MAJOR_VERSION >= 1
   EvasBiDiBracketType *bracket_types = NULL;
#endif

   if (!eina_ustr)
      return NULL;

   /* No need to handle bidi */
   if (!evas_bidi_is_rtl_str(eina_ustr) &&
       (base_bidi != EVAS_BIDI_PARAGRAPH_RTL))
     {
        goto cleanup;
     }

   len = eina_unicode_strlen(eina_ustr);
   /* The size of fribidichar s different than eina_unicode, convert */
#ifdef EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
   base_ustr = calloc(len + 1, sizeof(FriBidiChar));
   base_ustr = _evas_bidi_unicode_to_fribidichar(base_ustr, eina_ustr);
   ustr = base_ustr;
#else
   ustr = (const FriBidiChar *) eina_ustr;
#endif

   bidi_props = evas_bidi_paragraph_props_new();
   bidi_props->direction = base_bidi;

   /* Prep work for reordering */
   char_types = (EvasBiDiCharType *) malloc(sizeof(EvasBiDiCharType) * len);
   if (!char_types)
      {
         goto cleanup;
      }
   fribidi_get_bidi_types(ustr, len, char_types);

#if FRIBIDI_MAJOR_VERSION >= 1
   bracket_types = (EvasBiDiBracketType *) malloc(sizeof(EvasBiDiBracketType) * len);
   if (!bracket_types)
      {
         goto cleanup;
      }
   fribidi_get_bracket_types(ustr, len, char_types, bracket_types);
#endif

   embedding_levels = (EvasBiDiLevel *)malloc(sizeof(EvasBiDiLevel) * len);
   if (!embedding_levels)
     {
        goto cleanup;
     }

   if (segment_idxs)
     {
        size_t pos = 0;
        int *itr;
        EvasBiDiLevel base_level = 0;
        EvasBiDiParType direction;

        for (itr = segment_idxs ; *itr > 0 ; itr++)
          {
             direction = base_bidi;
#if FRIBIDI_MAJOR_VERSION >= 1
             ret_level = fribidi_get_par_embedding_levels_ex(char_types + pos,
                                                             bracket_types,
                                                             *itr - pos,
                                                             &direction,
                                                             embedding_levels + pos);
#else
             ret_level = fribidi_get_par_embedding_levels(char_types + pos,
                                                          *itr - pos,
                                                          &direction,
                                                          embedding_levels + pos);
#endif
             if (!ret_level)
               {
                  goto cleanup;
               }

             /* Only on the first run */
             if (itr == segment_idxs)
               {
                  bidi_props->direction = direction;
                  /* adjust base_level to be 1 for rtl paragraphs, and 0 for
                   * ltr paragraphs. */
                  base_level =
                     EVAS_BIDI_PARAGRAPH_DIRECTION_IS_RTL(bidi_props) ? 1 : 0;
               }

             /* We want those chars at the override points to be on the base
              * level and we also remove -2 cause we later increment them,
              * just for simpler code paths */
             embedding_levels[*itr] = base_level - 2;
             pos = *itr + 1;
          }

        direction = base_bidi;
#if FRIBIDI_MAJOR_VERSION >= 1
        ret_level = fribidi_get_par_embedding_levels_ex(char_types + pos,
                                                        bracket_types,
                                                        len - pos,
                                                        &direction,
                                                        embedding_levels + pos);
#else
        ret_level = fribidi_get_par_embedding_levels(char_types + pos,
                                                     len - pos,
                                                     &direction,
                                                     embedding_levels + pos);
#endif
        if (!ret_level)
          {
             goto cleanup;
          }

        /* Increment all levels by 2 to emulate embedding. */
          {
             EvasBiDiLevel *bitr = embedding_levels, *end;
             end = bitr + len;
             for ( ; bitr < end ; bitr++)
               {
                  *bitr += 2;
               }
          }
     }
   else
     {
#if FRIBIDI_MAJOR_VERSION >= 1
        ret_level = fribidi_get_par_embedding_levels_ex(char_types,
                                                        bracket_types,
                                                        len,
                                                        &bidi_props->direction,
                                                        embedding_levels);
#else
        ret_level = fribidi_get_par_embedding_levels(char_types,
                                                     len,
                                                     &bidi_props->direction,
                                                     embedding_levels);
#endif
        if (!ret_level)
          {
             goto cleanup;
          }
     }


   /* clean up */
   if (bidi_props->embedding_levels)
     {
        free(bidi_props->embedding_levels);
     }
   bidi_props->embedding_levels = embedding_levels;

   /* clean up */

   if (bidi_props->char_types)
     {
        free(bidi_props->char_types);
     }
   bidi_props->char_types = char_types;

   if (base_ustr) free(base_ustr);
#if FRIBIDI_MAJOR_VERSION >= 1
   /* Currently, bracket_types is not reused in other places. */
   if (bracket_types) free(bracket_types);
#endif

   return bidi_props;

/* Cleanup */
cleanup:
   if (char_types) free(char_types);
#if FRIBIDI_MAJOR_VERSION >= 1
   if (bracket_types) free(bracket_types);
#endif
   if (embedding_levels) free(embedding_levels);
   if (base_ustr) free(base_ustr);
   if (bidi_props) evas_bidi_paragraph_props_unref(bidi_props); /* Clean up the bidi props */
   return NULL;
}

/**
 * @internal
 * Copies dst to src and refs (doesn't copy) the paragraph props.
 *
 * @param src the props to copy
 * @param dst the props to copy to.
 */
void
evas_bidi_props_copy_and_ref(const Evas_BiDi_Props *src, Evas_BiDi_Props *dst)
{
   dst->dir = src->dir;
}

/**
 * @internal
 * Reorders ustr according to the bidi props.
 *
 * @param ustr the string to reorder. - Null is ok, will just populate the map.
 * @param start the start of the line
 * @param len the length of the line
 * @param props the paragraph props to reorder according to
 * @param _v_to_l The visual to logical map to populate - if NULL it won't populate it.
 * @return #EINA_FALSE on success, #EINA_TRUE on error.
 */
Eina_Bool
evas_bidi_props_reorder_line(Eina_Unicode *eina_ustr, size_t start, size_t len, const Evas_BiDi_Paragraph_Props *props, EvasBiDiStrIndex **_v_to_l)
{
   EvasBiDiStrIndex *v_to_l = NULL;
   FriBidiChar *ustr = NULL, *base_ustr = NULL;

   if (!props)
     return EINA_FALSE;

   if (eina_ustr)
     {
        /* The size of fribidichar is different than eina_unicode, convert */
#ifdef EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
        base_ustr = ustr = calloc(len + 1, sizeof(FriBidiChar));
        ustr = _evas_bidi_unicode_to_fribidichar(ustr, eina_ustr);
#else
        ustr = (FriBidiChar *) eina_ustr;
#endif
     }


   if (_v_to_l) {
      size_t i;
      v_to_l = *_v_to_l = calloc(len, sizeof(EvasBiDiStrIndex));
      if (!v_to_l)
        {
           goto error;
        }
      /* init the array for fribidi */
      for (i = 0 ; i < len ; i++)
        {
           v_to_l[i] = i;
        }
   }

     {
        EvasBiDiLevel *emb_lvl;
        emb_lvl = malloc((start + len) * sizeof(EvasBiDiLevel));
        memcpy(emb_lvl, props->embedding_levels,
              (start + len) * sizeof(EvasBiDiLevel));
        /* We pass v_to_l - start, because fribidi assumes start is the offset
         * from the start of v_to_l as well, not just the props. */
        if (!fribidi_reorder_line (FRIBIDI_FLAGS_DEFAULT, props->char_types,
                 len, start, props->direction, emb_lvl, ustr, v_to_l - start))
          {
             free(emb_lvl);
             goto error;
          }
        free(emb_lvl);
     }


   /* The size of fribidichar is different than eina_unicode, convert */
#ifdef EVAS_FRIBIDI_EINA_UNICODE_UNEQUAL
   _evas_bidi_fribidichar_to_unicode(eina_ustr, base_ustr);
   free(base_ustr);
#endif
   return EINA_FALSE;
/* ERROR HANDLING */
error:
   if (base_ustr) free(base_ustr);
   _SAFE_FREE(v_to_l);
   return EINA_TRUE;
}

/**
 * @internal
 * Returns the end of the current run of text
 *
 * @param bidi_props the paragraph properties
 * @param start where to start looking from
 * @param len the length of the string
 * @return the position of the end of the run (offset from
 * bidi_props->props->start), 0 when there is no end (i.e all the text)
 */
int
evas_bidi_end_of_run_get(const Evas_BiDi_Paragraph_Props *bidi_props,
      size_t start, int len)
{
   EvasBiDiLevel *i;
   EvasBiDiLevel base;

   if (!bidi_props || (len <= 0))
     return 0;

   i = bidi_props->embedding_levels + start;
   base = *i;
   for ( ; (len > 0) && (base == *i) ; len--, i++)
     ;

   if (len == 0)
     {
        return 0;
     }
   return i - (bidi_props->embedding_levels + start);
}

/**
 * @internal
 * Returns the visual string index from the logical string index.
 *
 * @param v_to_l the visual to logical map
 * @param len the length of the map.
 * @param position the position to convert.
 * @return on success the visual position, on failure the same position.
 */
EvasBiDiStrIndex
evas_bidi_position_logical_to_visual(EvasBiDiStrIndex *v_to_l, int len, EvasBiDiStrIndex position)
{
   int i;
   EvasBiDiStrIndex *ind;
   if (position >= len || !v_to_l)
      return position;

   for (i = 0, ind = v_to_l ; i < len ; i++, ind++)
     {
        if (*ind == position)
          {
             return i;
          }
     }
   return position;
}

/**
 * @internal
 * Returns the reversed pos of the index.
 *
 * @param dir the direction of the string
 * @param len the length of the map.
 * @param position the position to convert.
 * @return on success the visual position, on failure the same position.
 */
EvasBiDiStrIndex
evas_bidi_position_reverse(const Evas_BiDi_Props *props, int len, EvasBiDiStrIndex position)
{
   if (!props || position >= len)
      return position;

   return (props->dir == EVAS_BIDI_DIRECTION_RTL) ? (len - 1) - position : position;
}

/**
 * @internal
 * Checks if the char is rtl oriented. I.e even a neutral char can become rtl
 * if surrounded by rtl chars.
 *
 * @param bidi_props The bidi paragraph properties
 * @param start the base position
 * @param index the offset from the base position.
 * @return #EINA_TRUE if true, #EINA_FALSE otherwise.
 */
Eina_Bool
evas_bidi_is_rtl_char(const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, EvasBiDiStrIndex ind)
{
   if(!bidi_props || ind < 0)
      return EINA_FALSE;
   return (FRIBIDI_IS_RTL(
            bidi_props->embedding_levels[ind + start]))
      ? EINA_TRUE : EINA_FALSE;
}

Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_new(void)
{
   Evas_BiDi_Paragraph_Props *ret;
   ret = calloc(1, sizeof(Evas_BiDi_Paragraph_Props));
   ret->direction = EVAS_BIDI_PARAGRAPH_NEUTRAL;
   ret->refcount = 1;

   return ret;
}

/**
 * @internal
 * Refs the bidi props.
 *
 * @param bidi_props the props to ref.
 */
Evas_BiDi_Paragraph_Props *
evas_bidi_paragraph_props_ref(Evas_BiDi_Paragraph_Props *bidi_props)
{
   if (!bidi_props) return NULL;
   BIDILOCK();

   bidi_props->refcount++;
   BIDIUNLOCK();
   return bidi_props;
}

/**
 * @internal
 * Unrefs and potentially frees the props.
 *
 * @param bidi_props the properties to unref
 */
void
evas_bidi_paragraph_props_unref(Evas_BiDi_Paragraph_Props *bidi_props)
{
   if (!bidi_props) return;
   BIDILOCK();

   if (--bidi_props->refcount == 0)
     {
        evas_bidi_paragraph_props_clean(bidi_props);
        free(bidi_props);
     }
   BIDIUNLOCK();
}


/**
 * @internal
 * Cleans the paragraph properties.
 *
 * @param bidi_props the properties to clean.
 */
void
evas_bidi_paragraph_props_clean(Evas_BiDi_Paragraph_Props *bidi_props)
{
   _SAFE_FREE(bidi_props->embedding_levels);
   _SAFE_FREE(bidi_props->char_types);
}

/**
 * @internal
 * Cleans the bidi properties.
 *
 * @param bidi_props the properties to clean.
 */
void
evas_bidi_props_clean(Evas_BiDi_Props *bidi_props)
{
   if (!bidi_props) return;
   bidi_props->dir = EVAS_BIDI_DIRECTION_NEUTRAL;
}
/**
 * @}
 */
/**
 * @}
 */
#endif

#if 0
/* Good for debugging */
static void
dump_levels(Eina_Unicode *ustr, EvasBiDiLevel *emb)
{
   for ( ; *ustr ; ustr++, emb++)
     {
        printf("%lc %d\n", *ustr, *emb);
     }
}
#endif

