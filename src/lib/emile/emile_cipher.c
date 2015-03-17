#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <Eina.h>

#include "Emile.h"

#include "emile_private.h"

EAPI Eina_Binbuf *
emile_binbuf_cipher(const Eina_Binbuf *data EINA_UNUSED,
                    const char *key EINA_UNUSED,
                    unsigned int length EINA_UNUSED)
{
   return NULL;
}

EAPI Eina_Binbuf *
emile_binbuf_decipher(const Eina_Binbuf *data EINA_UNUSED,
                      const char *key EINA_UNUSED,
                      unsigned int length EINA_UNUSED)
{
   return NULL;
}
