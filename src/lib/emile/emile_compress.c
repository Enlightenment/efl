#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <zlib.h>

#ifdef ENABLE_LIBLZ4
#include <lz4.h>
#include <lz4hc.h>
#else
#include "lz4.h"
#include "lz4hc.h"
#endif

#include <Eina.h>

#include "Emile.h"

static int
_emile_compress_buffer_size(const Eina_Binbuf *data, Emile_Compressor_Type t)
{
   switch (t)
     {
      case EMILE_ZLIB:
        return 12 + ((eina_binbuf_length_get(data) * 101) / 100);

      case EMILE_LZ4:
      case EMILE_LZ4HC:
        return LZ4_compressBound(eina_binbuf_length_get(data));

      default:
        return -1;
     }
}

EAPI Eina_Binbuf *
emile_compress(const Eina_Binbuf *data,
               Emile_Compressor_Type t,
               Emile_Compressor_Level l)
{
   void *compact, *temp;
   int length;
   int level = l;
   Eina_Bool ok = EINA_FALSE;

   length = _emile_compress_buffer_size(data, t);

   compact = malloc(length);
   if (!compact)
     return NULL;

   switch (t)
     {
      case EMILE_LZ4:
        length = LZ4_compress_default
          ((const char *)eina_binbuf_string_get(data), compact,
           eina_binbuf_length_get(data), length);
        /* It is going to be smaller and should never fail, if it does you are in deep poo. */
        temp = realloc(compact, length);
        if (temp) compact = temp;

        if (length > 0)
          ok = EINA_TRUE;
        break;

      case EMILE_LZ4HC:
        length = LZ4_compress_HC
          ((const char *)eina_binbuf_string_get(data), compact,
           eina_binbuf_length_get(data), length, 16);
        temp = realloc(compact, length);
        if (temp) compact = temp;

        if (length > 0)
          ok = EINA_TRUE;
        break;

      case EMILE_ZLIB:
      {
         uLongf buflen = (uLongf)length;

         if (compress2((Bytef *)compact, &buflen, (Bytef *)eina_binbuf_string_get(data), (uLong)eina_binbuf_length_get(data), level) == Z_OK)
           ok = EINA_TRUE;
         length = (int)buflen;
      }
     }

   if (!ok)
     {
        free(compact);
        return NULL;
     }

   return eina_binbuf_manage_new(compact, length, EINA_FALSE);
}

EAPI Eina_Bool
emile_expand(const Eina_Binbuf *in, Eina_Binbuf *out, Emile_Compressor_Type t)
{
   if (!in || !out)
     return EINA_FALSE;

   switch (t)
     {
      case EMILE_LZ4:
      case EMILE_LZ4HC:
      {
         int ret;

         ret = LZ4_decompress_fast((const char *)eina_binbuf_string_get(in),
                                   (char *)eina_binbuf_string_get(out),
                                   eina_binbuf_length_get(out));
         if ((unsigned int)ret != eina_binbuf_length_get(in))
           return EINA_FALSE;
         break;
      }

      case EMILE_ZLIB:
      {
         uLongf dlen = eina_binbuf_length_get(out);

         if (uncompress((Bytef *)eina_binbuf_string_get(out), &dlen, eina_binbuf_string_get(in), (uLongf)eina_binbuf_length_get(in)) != Z_OK)
           return EINA_FALSE;
         break;
      }

      default:
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Binbuf *
emile_decompress(const Eina_Binbuf *data,
                 Emile_Compressor_Type t,
                 unsigned int dest_length)
{
   Eina_Binbuf *out;
   void *expanded;

   expanded = malloc(dest_length);
   if (!expanded)
     return NULL;

   out = eina_binbuf_manage_new(expanded, dest_length, EINA_FALSE);
   if (!out)
     goto on_error;

   if (!emile_expand(data, out, t))
     goto on_error;

   return out;

on_error:
   if (!out)
     free(expanded);
   if (out)
     eina_binbuf_free(out);
   return NULL;
}
