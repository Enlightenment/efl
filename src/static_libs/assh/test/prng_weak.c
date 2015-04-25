
#include <assh/assh_prng.h>
#include <assh/assh_context.h>

#include <string.h>
#include <stdlib.h>

static ASSH_PRNG_INIT_FCN(assh_prng_weak_init)
{
  c->prng_entropy = 0;
  srand(0);
  return ASSH_OK;
}

static ASSH_PRNG_GET_FCN(assh_prng_weak_get)
{
  size_t i;

  switch (quality)
    {
    case ASSH_PRNG_QUALITY_WEAK:
      memset(rdata, 42, rdata_len);
      break;
    default:
      for (i = 0; i < rdata_len; i++)
	rdata[i] = rand();
      break;
    }

  return ASSH_OK;
}

static ASSH_PRNG_FEED_FCN(assh_prng_weak_feed)
{
  return ASSH_OK;
}

static ASSH_PRNG_CLEANUP_FCN(assh_prng_weak_cleanup)
{
}

const struct assh_prng_s assh_prng_weak = 
{
  .f_init = assh_prng_weak_init,
  .f_get = assh_prng_weak_get,
  .f_feed = assh_prng_weak_feed,
  .f_cleanup = assh_prng_weak_cleanup,  
};


