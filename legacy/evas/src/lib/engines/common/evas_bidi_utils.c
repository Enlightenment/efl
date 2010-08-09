#include <string.h>
#include <stdlib.h>

#include "evas_common.h"
#include "evas_bidi_utils.h"
#include "evas_encoding.h"

#include "evas_font_private.h"

#ifdef BIDI_SUPPORT
#include <fribidi/fribidi.h>

#define _SAFE_FREE(x) \
   do {               \
      if (x)          \
        {             \
           free(x);   \
           x = NULL;  \
        }             \
     } while(0)

Eina_Bool
evas_bidi_is_rtl_str(const Eina_Unicode *str)
{
   int i = 0;
   int ch;
   EvasBiDiCharType type;
   
   if (!str)
      return EINA_FALSE;

   for ( ; *str ; str++)
     {
        type = fribidi_get_bidi_type(*str);
        if (FRIBIDI_IS_LETTER(type) && FRIBIDI_IS_RTL(type)) 
          {
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

Eina_Bool
evas_bidi_shape_string(Eina_Unicode *ustr, const Evas_BiDi_Props *bidi_props, size_t len)
{
   EvasBiDiJoiningType *join_types = NULL;
   join_types = (EvasBiDiJoiningType *) malloc(sizeof(EvasBiDiJoiningType) * len);
   if (!join_types)
     {
        return -2;
     }
   fribidi_get_joining_types(ustr, len, join_types);

   fribidi_join_arabic(bidi_props->props->char_types, len, bidi_props->props->embedding_levels + bidi_props->start, join_types);


   /* Actually modify the string */
   fribidi_shape(FRIBIDI_FLAGS_DEFAULT | FRIBIDI_FLAGS_ARABIC,
               bidi_props->props->embedding_levels + bidi_props->start, len, join_types, ustr);

   if (join_types) free(join_types);
   return EINA_TRUE;
}

int
evas_bidi_update_props(const Eina_Unicode *ustr, Evas_BiDi_Paragraph_Props *bidi_props)
{
   EvasBiDiCharType *char_types = NULL;
   EvasBiDiLevel *embedding_levels = NULL;
   size_t len;

   if (!ustr)
      return -2;

   if (!evas_bidi_is_rtl_str(ustr)) /* No need to handle bidi */
     {
        len = -1;
        goto cleanup;
     }

   len = eina_unicode_strlen(ustr);

   /* Prep work for reordering */ 
   char_types = (EvasBiDiCharType *) malloc(sizeof(EvasBiDiCharType) * len);
   if (!char_types)
      {
         len = -2;
         goto cleanup;
      }
   fribidi_get_bidi_types(ustr, len, char_types);

   embedding_levels = (EvasBiDiLevel *)malloc(sizeof(EvasBiDiLevel) * len);
   if (!embedding_levels)
     {
        len = -2;
        goto cleanup;
     }
   if (!fribidi_get_par_embedding_levels(char_types, len, &bidi_props->direction, embedding_levels))
     {
        len = -2;
        goto cleanup;
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

  
   return len;

/* Cleanup */
cleanup:
   if (char_types) free(char_types);
   if (embedding_levels) free(embedding_levels);
   evas_bidi_paragraph_props_clean(bidi_props); /*Mark that we don't need bidi handling */
   return len;
}

int
evas_bidi_props_reorder_line(Eina_Unicode *ustr, const Evas_BiDi_Props *intl_props, EvasBiDiStrIndex **_v_to_l)
{
   EvasBiDiStrIndex *v_to_l = NULL;
   size_t len;

   if (!EVAS_BIDI_IS_BIDI_PROP(intl_props->props))
     return 0;

   len = eina_unicode_strlen(ustr);

   if (_v_to_l) {
      int i;
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

   /* Shaping must be done *BEFORE* breaking to lines so there's no choice but
    doing it in textblock. */
   if (!fribidi_reorder_line (FRIBIDI_FLAGS_DEFAULT, intl_props->props->char_types + intl_props->start, 
                              len, 0, intl_props->props->direction, 
                              intl_props->props->embedding_levels + intl_props->start, ustr, v_to_l))
     {
        goto error;
     }

  
   return 0;
/* ERROR HANDLING */
error:
   _SAFE_FREE(v_to_l);
   return 1;
}


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

Eina_Bool
evas_bidi_is_rtl_char(EvasBiDiLevel *embedded_level_list, EvasBiDiStrIndex index)
{
   if(!embedded_level_list || index < 0)
      return EINA_FALSE;
   return (FRIBIDI_IS_RTL(embedded_level_list[index])) ? EINA_TRUE : EINA_FALSE;
}

void
evas_bidi_paragraph_props_clean(Evas_BiDi_Paragraph_Props *bidi_props)
{
   _SAFE_FREE(bidi_props->embedding_levels);
   _SAFE_FREE(bidi_props->char_types);
}
void
evas_bidi_props_clean(Evas_BiDi_Props *bidi_props)
{
   if (!bidi_props) return;
   evas_bidi_paragraph_props_clean(bidi_props->props);
   bidi_props->props = NULL;
}
#endif
