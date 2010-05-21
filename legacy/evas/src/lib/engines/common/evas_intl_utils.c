#include <string.h>
#include <stdlib.h>

#include "evas_common.h"
#include "evas_intl_utils.h"

#include "evas_font_private.h"

#ifdef INTERNATIONAL_SUPPORT
#include <fribidi/fribidi.h>

#define UTF8_BYTES_PER_CHAR 4

#ifdef BUILD_PTHREAD
static LK(fribidi_lock);
#endif

/* FIXME: fribidi_utf8_to_unicode should use char len and not byte len!*/
char *
evas_intl_utf8_to_visual(const char *text,
			int *ret_len,
			EvasIntlParType *direction,
			EvasIntlStrIndex **position_L_to_V_list,
			EvasIntlStrIndex **position_V_to_L_list,
			EvasIntlLevel **embedding_level_list)
{
   FriBidiChar *unicode_in, *unicode_out;
   EvasIntlStrIndex *tmp_L_to_V_list = NULL;
   EvasIntlStrIndex *tmp_V_to_L_list = NULL;
   EvasIntlLevel *tmp_level_list = NULL;
   char *text_out;
   size_t len;
   size_t byte_len;

   if (!text)
     return NULL;

   len = evas_string_char_len_get(text);

   byte_len = strlen(text); /* we need the actual number of bytes, not number of chars */

   unicode_in = (FriBidiChar *)alloca(sizeof(FriBidiChar) * (len + 1));
   if (!unicode_in)
     {
	len = -1;
        goto error1;
     }

   FBDLOCK();
   len = fribidi_utf8_to_unicode(text, byte_len, unicode_in);
   FBDUNLOCK();
   unicode_in[len] = 0;

   unicode_out = (FriBidiChar *)alloca(sizeof(FriBidiChar) * (len + 1));
   if (!unicode_out)
     {
	len = -1;
	goto error2;
     }

    if (embedding_level_list)
       {
          *embedding_level_list = (EvasIntlLevel *)malloc(sizeof(EvasIntlLevel) * len);
          if (!*embedding_level_list)
            {
 	      len = -1;
 	      goto error3;
            }
 	 tmp_level_list = *embedding_level_list;
       }
 
     if (position_L_to_V_list)
       {
          *position_L_to_V_list = (EvasIntlStrIndex *)malloc(sizeof(EvasIntlStrIndex) * len);
          if (!*position_L_to_V_list)
            {
 	      len = -1;
 	      goto error4;
            }
 	 tmp_L_to_V_list = *position_L_to_V_list;
       }
 
     if (position_V_to_L_list)
       {
          *position_V_to_L_list = (EvasIntlStrIndex *)malloc(sizeof(EvasIntlStrIndex) * len);
          if (!*position_V_to_L_list)
            {
 	      len = -1;
 	      goto error5;
            }
 	 tmp_V_to_L_list = *position_V_to_L_list;
       }

#ifdef ARABIC_SUPPORT
   /* fix arabic context */
   evas_intl_arabic_to_context(unicode_in);
#endif
   LKL(fribidi_lock);
   if (!fribidi_log2vis(unicode_in, len, direction,
         unicode_out, tmp_L_to_V_list, tmp_V_to_L_list, tmp_level_list))
     {
        LKU(fribidi_lock);
 	len = -2;
 	goto error5;
     }
   LKU(fribidi_lock);

   text_out = malloc(UTF8_BYTES_PER_CHAR * len + 1);
   if (!text_out)
     {
 	len = -1;
 	goto error6;
     }

   fribidi_unicode_to_utf8(unicode_out, len, text_out);
   FBDUNLOCK();
   
   *ret_len = len;
   return text_out;

/* ERROR HANDLING */
error6:
   free(unicode_out);
error5:
   free(*position_V_to_L_list);
   *position_V_to_L_list = NULL;
error4:
   free(*position_L_to_V_list);
   *position_L_to_V_list = NULL;
error3:
   free(*embedding_level_list);
   *embedding_level_list = NULL;
error2:
   free(unicode_in);
error1:

   *ret_len = len;
   return NULL;
}

int
evas_intl_is_rtl_char(EvasIntlLevel *embedded_level_list, EvasIntlStrIndex i)
{
   if(embedded_level_list || i < 0)
     return 0;
   return FRIBIDI_IS_RTL(embedded_level_list[i]);
}
#endif
