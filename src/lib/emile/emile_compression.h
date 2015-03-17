#ifndef EMILE_COMPRESSION_H_
#define EMILE_COMPRESSION_H_

typedef enum
{
  EMILE_ZLIB,
  EMILE_LZ4,
  EMILE_LZ4HC
} Emile_Compressor_Type;

typedef enum
{
  EMILE_DEFAULT = -1,
  EMILE_NO_COMPRESSION = 0,
  EMILE_FAST_COMPRESSION = 1,
  EMILE_BEST_COMPRESSION = 9
} Emile_Compressor_Level;

EAPI Eina_Binbuf *emile_binbuf_compress(const Eina_Binbuf *in,
                                        Emile_Compressor_Type t, int level);
EAPI Eina_Binbuf *emile_binbuf_uncompress(const Eina_Binbuf *in,
                                          Emile_Compressor_Type t,
                                          unsigned int dest_length);
EAPI Eina_Bool emile_binbuf_expand(const Eina_Binbuf *in,
                                   Eina_Binbuf *out,
                                   Emile_Compressor_Type t);

#endif
