/*

  libassh - asynchronous ssh2 client/server library.

  Copyright (C) 2013 Alexandre Becoulet <alexandre.becoulet@free.fr>

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301 USA

*/

#include <assh/assh_kex.h>
#include <assh/assh_session.h>
#include <assh/assh_packet.h>
#include <assh/assh_transport.h>
#include <assh/assh_bignum.h>
#include <assh/assh_sign.h>
#include <assh/assh_prng.h>
#include <assh/assh_event.h>
#include <assh/assh_alloc.h>
#include <assh/assh_hash.h>
#include <assh/assh_cipher.h>

#include <string.h>
#include <stdlib.h>

#define DH_MAX_GRSIZE 8192

struct assh_kex_dh_group_s
{
  size_t size;
  const uint8_t *generator;
  const uint8_t *prime;
};

enum assh_kex_dh_state_e
{
#ifdef CONFIG_ASSH_CLIENT
  ASSH_KEX_DH_CLIENT_WAIT_F,
  ASSH_KEX_DH_CLIENT_INIT,
  ASSH_KEX_DH_CLIENT_LOOKUP_HOST_KEY_WAIT,
#endif
#ifdef CONFIG_ASSH_SERVER
  ASSH_KEX_DH_SERVER_WAIT_E,
#endif
};

struct assh_kex_dh_private_s
{
  const struct assh_kex_dh_group_s *group;
  enum assh_kex_dh_state_e state;

  union {
#ifdef CONFIG_ASSH_SERVER
    struct {
      size_t exp_n;
    };
#endif
#ifdef CONFIG_ASSH_CLIENT
    struct {
      struct assh_bignum_s en;
      struct assh_bignum_s xn;
      const struct assh_key_s *host_key;
      struct assh_packet_s *pck;
    };
#endif
  };
};

#ifdef CONFIG_ASSH_CLIENT
static assh_error_t assh_kex_dh_client_send_expmod(struct assh_session_s *s)
{
  struct assh_kex_dh_private_s *pv = s->kex_pv;
  const struct assh_kex_dh_group_s *gr = pv->group;
  struct assh_context_s *c = s->ctx;
  assh_error_t err;

  struct assh_packet_s *p;
  size_t e_size = assh_bignum_size_of_bits(ASSH_BIGNUM_MPINT, gr->size);

  ASSH_ERR_RET(assh_packet_alloc(c, SSH_MSG_KEX_DH_REQUEST, e_size, &p)
	       | ASSH_ERRSV_DISCONNECT);

  uint8_t *e_str;
  ASSH_ASSERT(assh_packet_add_array(p, e_size, &e_str));

  enum bytecode_args_e
  {
    G_mpint, P_mpint, E_mpint,
    E, X, G_n,
    G, P, MT
  };

  static const assh_bignum_op_t bytecode[] = {
    ASSH_BOP_SIZE(      G,      G_n		        ),
    ASSH_BOP_SIZE(      P,      G_n		        ),

    ASSH_BOP_MOVE(      G,      G_mpint			),
    ASSH_BOP_MOVE(      P,      P_mpint			),

    /* generate private exponent in range [ group bits, p - 2 ] */
    ASSH_BOP_UINT(      E,      DH_MAX_GRSIZE		),
    ASSH_BOP_RAND(      X,      E,      P,
                        ASSH_PRNG_QUALITY_EPHEMERAL_KEY),

    /* compute dh public key */
    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_MTTO(      G,      G,      G,      MT      ),
    ASSH_BOP_EXPM(      E,      G,      X,	MT      ),
    ASSH_BOP_MTFROM(    E,      E,      E,      MT      ),

    ASSH_BOP_MOVE(      E_mpint,        E		),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "MMMNNsTTm",
                   /* M */ gr->generator, gr->prime, e_str,
                   /* N */ &pv->en, &pv->xn, gr->size), err_p);

  assh_packet_string_resized(p, e_str + 4);

  assh_transport_push(s, p);
  pv->state = ASSH_KEX_DH_CLIENT_WAIT_F;
  return ASSH_OK;

 err_p:
  assh_packet_release(p);
  return err;
}

static ASSH_EVENT_DONE_FCN(assh_kex_dh_host_key_lookup_done)
{
  struct assh_kex_dh_private_s *pv = s->kex_pv;
  const struct assh_kex_dh_group_s *gr = pv->group;
  assh_error_t err;

  ASSH_CHK_RET(pv->state != ASSH_KEX_DH_CLIENT_LOOKUP_HOST_KEY_WAIT,
               ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  if (!e->kex.hostkey_lookup.accept)
    {
      ASSH_ERR_RET(assh_kex_end(s, 0) | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
    }

  struct assh_packet_s *p = pv->pck;

  uint8_t *ks_str = p->head.end;
  uint8_t *f_str, *h_str;

  ASSH_ERR_RET(assh_packet_check_string(p, ks_str, &f_str)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, f_str, &h_str)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, h_str, NULL)
	       | ASSH_ERRSV_DISCONNECT);

  ASSH_SCRATCH_ALLOC(s->ctx, uint8_t, scratch,
                     assh_bignum_size_of_bits(ASSH_BIGNUM_MPINT, gr->size) +
                     assh_hash_sha1.ctx_size,
                     ASSH_ERRSV_CONTINUE, err_);

  void *hash_ctx = scratch;
  uint8_t *secret = scratch + assh_hash_sha1.ctx_size;

  enum bytecode_args_e
  {
    G_mpint, P_mpint, F_mpint, K_mpint,
    X, G_n,
    G, P, F, T, K, MT
  };

  static const assh_bignum_op_t bytecode[] = {
    ASSH_BOP_SIZER(     G,      K,	G_n		),

    ASSH_BOP_MOVE(      F,      F_mpint			),
    ASSH_BOP_MOVE(      G,      G_mpint			),
    ASSH_BOP_MOVE(      P,      P_mpint			),

    /* check server public exponent */
    ASSH_BOP_UINT(      T,      2			),
    ASSH_BOP_CMPLTEQ(   T,      F,      0 /* f >= 2 */	), 
    ASSH_BOP_SUB(       T,      P,      T       	),
    ASSH_BOP_CMPLTEQ(   F,      T,      0 /* f <= p-2 */), 

    /* FIXME check that log_g(pub) is not trivial */

    /* compute shared secret */
    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_MTTO(      F,      F,      F,      MT      ),
    ASSH_BOP_EXPM(      T,      F,      X,      MT	),
    ASSH_BOP_MTFROM(    K,      K,      T,      MT      ),

    /* check shared secret range */
    ASSH_BOP_UINT(      T,      2			),
    ASSH_BOP_CMPLTEQ(   T,      K,      0 /* k >= 2 */	),
    ASSH_BOP_SUB(       T,      P,      T       	),
    ASSH_BOP_CMPLTEQ(   K,      T,      0 /* k <= p-2 */),

    ASSH_BOP_MOVE(      K_mpint,        K		),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(s->ctx, bytecode, "MMMMNsTTTTTm",
                   /* M */ gr->generator, gr->prime, f_str, secret,
                   /* N */ &pv->xn, gr->size), err_scratch);

  ASSH_ERR_GTO(assh_hash_init(s->ctx, hash_ctx, &assh_hash_sha1), err_scratch);

  ASSH_ERR_GTO(assh_kex_client_hash1(s, hash_ctx, ks_str)
               | ASSH_ERRSV_DISCONNECT, err_hash);

  ASSH_ERR_GTO(assh_hash_bignum(s->ctx, hash_ctx, &pv->en)
	       | ASSH_ERRSV_DISCONNECT, err_hash);

  assh_hash_string(hash_ctx, f_str);

  ASSH_ERR_GTO(assh_kex_client_hash2(s, hash_ctx,
                        pv->host_key, secret, h_str)
               | ASSH_ERRSV_DISCONNECT, err_hash);

  ASSH_ERR_GTO(assh_kex_end(s, 1) | ASSH_ERRSV_DISCONNECT, err_hash);

  err = ASSH_OK;

 err_hash:
  assh_hash_cleanup(hash_ctx);
 err_scratch:
  ASSH_SCRATCH_FREE(s->ctx, scratch);
 err_:
  return err;
}

static assh_error_t assh_kex_dh_client_wait_f(struct assh_session_s *s,
					      struct assh_packet_s *p,
                                              struct assh_event_s *e)
{
  struct assh_kex_dh_private_s *pv = s->kex_pv;
  assh_error_t err;

  ASSH_CHK_RET(p->head.msg != SSH_MSG_KEX_DH_REPLY, ASSH_ERR_PROTOCOL
	       | ASSH_ERRSV_DISCONNECT);

  uint8_t *ks_str = p->head.end;
  uint8_t *f_str, *h_str;

  ASSH_ERR_RET(assh_packet_check_string(p, ks_str, &f_str)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, f_str, &h_str)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, h_str, NULL)
	       | ASSH_ERRSV_DISCONNECT);

  ASSH_ERR_RET(assh_kex_client_get_key(s, &pv->host_key, ks_str, e,
                                &assh_kex_dh_host_key_lookup_done, pv));

  pv->state = ASSH_KEX_DH_CLIENT_LOOKUP_HOST_KEY_WAIT;
  pv->pck = assh_packet_refinc(p);

  return ASSH_OK;
}
#endif

#ifdef CONFIG_ASSH_SERVER

static assh_error_t assh_kex_dh_server_wait_e(struct assh_session_s *s,
                                              struct assh_packet_s *p)
{
  struct assh_kex_dh_private_s *pv = s->kex_pv;
  const struct assh_kex_dh_group_s *gr = pv->group;
  struct assh_context_s *c = s->ctx;
  assh_error_t err;

  ASSH_CHK_RET(p->head.msg != SSH_MSG_KEX_DH_REQUEST,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  /* compute DH */
  uint8_t *e_str = p->head.end;

  ASSH_ERR_RET(assh_packet_check_string(p, e_str, NULL)
	       | ASSH_ERRSV_DISCONNECT);

  size_t l = assh_bignum_size_of_bits(ASSH_BIGNUM_MPINT, gr->size);

  ASSH_SCRATCH_ALLOC(s->ctx, uint8_t, scratch,
                     l + assh_hash_sha1.ctx_size,
                     ASSH_ERRSV_CONTINUE, err_);

  void *hash_ctx = scratch;
  uint8_t *secret = scratch + assh_hash_sha1.ctx_size;

  ASSH_ERR_GTO(assh_hash_init(s->ctx, hash_ctx, &assh_hash_sha1), err_scratch);

  struct assh_packet_s *pout;
  const struct assh_key_s *hk;
  size_t slen;

  ASSH_ERR_GTO(assh_kex_server_hash1(s, l, hash_ctx, &pout,
                 &slen, &hk, SSH_MSG_KEX_DH_REPLY), err_hash);

  uint8_t *f_str;
  ASSH_ASSERT(assh_packet_add_array(pout, l, &f_str));

  enum bytecode_args_e
  {
    G_mpint, P_mpint, E_mpint, F_mpint, K_mpint,
    X_n, G_n,
    F, G, P, E, X, K, T, MT
  };

  static const assh_bignum_op_t bytecode[] = {
    ASSH_BOP_SIZER(     F,      T,	G_n		),

    ASSH_BOP_MOVE(      G,      G_mpint         	),
    ASSH_BOP_MOVE(      P,      P_mpint         	),
    ASSH_BOP_MOVE(      E,      E_mpint         	),

    /* check client public key */
    ASSH_BOP_UINT(      T,      2               	),
    ASSH_BOP_CMPLTEQ(   T,      E,      0 /* f >= 2 */  ), 
    ASSH_BOP_SUB(       T,      P,      T               ),
    ASSH_BOP_CMPLTEQ(   E,      T,      0 /* f <= p-2 */), 

    /* FIXME check that log_g(pub) is not trivial */

    /* generate private exponent */
    ASSH_BOP_UINT(      T,      DH_MAX_GRSIZE   	),
    ASSH_BOP_RAND(      X,      T,      P,
                        ASSH_PRNG_QUALITY_EPHEMERAL_KEY),

    /* compute dh public key and shared secret */
    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_MTTO(      G,      G,      G,      MT      ),
    ASSH_BOP_EXPM(      F,      G,      X,	MT      ),
    ASSH_BOP_MTFROM(    F,      F,      F,      MT      ),
    ASSH_BOP_MTTO(      E,      E,      E,      MT      ),
    ASSH_BOP_EXPM(      K,      E,      X,	MT      ),
    ASSH_BOP_MTFROM(    K,      K,      K,      MT      ),

    /* check shared secret range */
    ASSH_BOP_UINT(      T,      2               	),
    ASSH_BOP_CMPLTEQ(   T,      K,      0 /* k >= 2 */	),
    ASSH_BOP_SUB(       T,      P,      T       	),
    ASSH_BOP_CMPLTEQ(   K,      T,      0 /* k <= p-2 */),

    ASSH_BOP_MOVE(      K_mpint,        K		),
    ASSH_BOP_MOVE(      F_mpint,        F		),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "MMMMMssTTTTXTTm",
                   gr->generator, gr->prime, e_str, f_str, secret,
                   pv->exp_n, gr->size), err_p);

  assh_packet_string_resized(pout, f_str + 4);

  /* hash both ephemeral public keys */
  assh_hash_string(hash_ctx, e_str);
  assh_hash_string(hash_ctx, f_str);

  ASSH_ERR_GTO(assh_kex_server_hash2(s, hash_ctx, pout,
                 slen, hk, secret), err_p);

  assh_transport_push(s, pout);

  ASSH_ERR_GTO(assh_kex_end(s, 1) | ASSH_ERRSV_DISCONNECT, err_hash);

  err = ASSH_OK;
  goto err_hash;

 err_p:
  assh_packet_release(pout);
 err_hash:
  assh_hash_cleanup(hash_ctx);
 err_scratch:
  ASSH_SCRATCH_FREE(c, scratch);
 err_:
  return err;
}
#endif


static ASSH_KEX_PROCESS_FCN(assh_kex_dh_process)
{
  struct assh_kex_dh_private_s *pv = s->kex_pv;
  assh_error_t err;

  switch (pv->state)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_KEX_DH_CLIENT_INIT:
      assert(p == NULL);
      ASSH_ERR_RET(assh_kex_dh_client_send_expmod(s)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;

    case ASSH_KEX_DH_CLIENT_WAIT_F:
      if (p == NULL)
        return ASSH_OK;
      ASSH_ERR_RET(assh_kex_dh_client_wait_f(s, p, e)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;

    case ASSH_KEX_DH_CLIENT_LOOKUP_HOST_KEY_WAIT:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
#endif

#ifdef CONFIG_ASSH_SERVER
    case ASSH_KEX_DH_SERVER_WAIT_E:
      if (p == NULL)
        return ASSH_OK;
      ASSH_ERR_RET(assh_kex_dh_server_wait_e(s, p)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
#endif
    }

  abort();
}

static assh_error_t assh_kex_dh_init(struct assh_session_s *s,
                                     size_t cipher_key_size,
                                     const struct assh_kex_dh_group_s *group)
{
  assh_error_t err;
  struct assh_kex_dh_private_s *pv;

  size_t exp_n = cipher_key_size * 2;

  /* allocate DH private context */
  ASSH_ERR_RET(assh_alloc(s->ctx, sizeof(*pv), ASSH_ALLOC_INTERNAL, (void**)&pv)
	       | ASSH_ERRSV_DISCONNECT);

  switch (s->ctx->type)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_CLIENT: {
      pv->state = ASSH_KEX_DH_CLIENT_INIT;
      break;
    }
#endif
#ifdef CONFIG_ASSH_SERVER
    case ASSH_SERVER:
      pv->state = ASSH_KEX_DH_SERVER_WAIT_E;
      break;
#endif
    default:
      abort();
    }

  s->kex_pv = pv;
  pv->group = group;

  switch (s->ctx->type)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_CLIENT:
      assh_bignum_init(s->ctx, &pv->en, group->size, 0);
      assh_bignum_init(s->ctx, &pv->xn, exp_n, 1);
      pv->host_key = NULL;
      pv->pck = NULL;
      break;
#endif

#ifdef CONFIG_ASSH_SERVER
    case ASSH_SERVER:
      pv->exp_n = exp_n;
      break;
#endif
    default:;
    }

  return ASSH_OK;
}

static ASSH_KEX_CLEANUP_FCN(assh_kex_dh_cleanup)
{
  struct assh_kex_dh_private_s *pv = s->kex_pv;
  struct assh_context_s *c = s->ctx;
  
  switch (c->type)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_CLIENT:
      assh_bignum_release(c, &pv->en);
      assh_bignum_release(c, &pv->xn);
      assh_key_flush(s->ctx, &pv->host_key);
      assh_packet_release(pv->pck);
      break;
#endif

#ifdef CONFIG_ASSH_SERVER
    case ASSH_SERVER:
      break;
#endif

    default:
      abort();
    }

  assh_free(c, s->kex_pv);
  s->kex_pv = NULL;
}

static ASSH_KEX_INIT_FCN(assh_kex_dh_group1_sha1_init)
{
  static const struct assh_kex_dh_group_s group =
    {
      .size = 1024,
      .generator = (const uint8_t*)"\x00\x00\x00\x01\x02",
      .prime = (const uint8_t*)"\x00\x00\x00\x81"
      "\x00\xff\xff\xff\xff\xff\xff\xff\xff\xc9\x0f\xda\xa2\x21\x68\xc2\x34"
      "\xc4\xc6\x62\x8b\x80\xdc\x1c\xd1\x29\x02\x4e\x08\x8a\x67\xcc\x74"
      "\x02\x0b\xbe\xa6\x3b\x13\x9b\x22\x51\x4a\x08\x79\x8e\x34\x04\xdd"
      "\xef\x95\x19\xb3\xcd\x3a\x43\x1b\x30\x2b\x0a\x6d\xf2\x5f\x14\x37"
      "\x4f\xe1\x35\x6d\x6d\x51\xc2\x45\xe4\x85\xb5\x76\x62\x5e\x7e\xc6"
      "\xf4\x4c\x42\xe9\xa6\x37\xed\x6b\x0b\xff\x5c\xb6\xf4\x06\xb7\xed"
      "\xee\x38\x6b\xfb\x5a\x89\x9f\xa5\xae\x9f\x24\x11\x7c\x4b\x1f\xe6"
      "\x49\x28\x66\x51\xec\xe6\x53\x81\xff\xff\xff\xff\xff\xff\xff\xff"
    };

  return assh_kex_dh_init(s, cipher_key_size, &group);
}

const struct assh_algo_kex_s assh_kex_dh_group1_sha1 =
{
  .algo = { .name = "diffie-hellman-group1-sha1",
            .class_ = ASSH_ALGO_KEX, .safety = 10, .speed = 40 },
  .f_init = assh_kex_dh_group1_sha1_init,
  .f_cleanup = assh_kex_dh_cleanup,
  .f_process = assh_kex_dh_process,
};

static ASSH_KEX_INIT_FCN(assh_kex_dh_group14_sha1_init)
{
  static const struct assh_kex_dh_group_s group =
    {
      .size = 2048,
      .generator = (const uint8_t*)"\x00\x00\x00\x01\x02",
      .prime = (const uint8_t*)"\x00\x00\x01\x01"
      "\x00\xff\xff\xff\xff\xff\xff\xff\xff\xc9\x0f\xda\xa2\x21\x68\xc2\x34"
      "\xc4\xc6\x62\x8b\x80\xdc\x1c\xd1\x29\x02\x4e\x08\x8a\x67\xcc\x74"
      "\x02\x0b\xbe\xa6\x3b\x13\x9b\x22\x51\x4a\x08\x79\x8e\x34\x04\xdd"
      "\xef\x95\x19\xb3\xcd\x3a\x43\x1b\x30\x2b\x0a\x6d\xf2\x5f\x14\x37"
      "\x4f\xe1\x35\x6d\x6d\x51\xc2\x45\xe4\x85\xb5\x76\x62\x5e\x7e\xc6"
      "\xf4\x4c\x42\xe9\xa6\x37\xed\x6b\x0b\xff\x5c\xb6\xf4\x06\xb7\xed"
      "\xee\x38\x6b\xfb\x5a\x89\x9f\xa5\xae\x9f\x24\x11\x7c\x4b\x1f\xe6"
      "\x49\x28\x66\x51\xec\xe4\x5b\x3d\xc2\x00\x7c\xb8\xa1\x63\xbf\x05"
      "\x98\xda\x48\x36\x1c\x55\xd3\x9a\x69\x16\x3f\xa8\xfd\x24\xcf\x5f"
      "\x83\x65\x5d\x23\xdc\xa3\xad\x96\x1c\x62\xf3\x56\x20\x85\x52\xbb"
      "\x9e\xd5\x29\x07\x70\x96\x96\x6d\x67\x0c\x35\x4e\x4a\xbc\x98\x04"
      "\xf1\x74\x6c\x08\xca\x18\x21\x7c\x32\x90\x5e\x46\x2e\x36\xce\x3b"
      "\xe3\x9e\x77\x2c\x18\x0e\x86\x03\x9b\x27\x83\xa2\xec\x07\xa2\x8f"
      "\xb5\xc5\x5d\xf0\x6f\x4c\x52\xc9\xde\x2b\xcb\xf6\x95\x58\x17\x18"
      "\x39\x95\x49\x7c\xea\x95\x6a\xe5\x15\xd2\x26\x18\x98\xfa\x05\x10"
      "\x15\x72\x8e\x5a\x8a\xac\xaa\x68\xff\xff\xff\xff\xff\xff\xff\xff"
    };

  return assh_kex_dh_init(s, cipher_key_size, &group);
}

const struct assh_algo_kex_s assh_kex_dh_group14_sha1 =
{
  .algo = { .name = "diffie-hellman-group14-sha1",
            .class_ = ASSH_ALGO_KEX, .safety = 20, .speed = 30 },
  .f_init = assh_kex_dh_group14_sha1_init,
  .f_cleanup = assh_kex_dh_cleanup,
  .f_process = assh_kex_dh_process,
};

