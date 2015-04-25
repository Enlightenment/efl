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

/*
  This file implements rfc4419
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

#define DH_MAX_GRSIZE 16384

enum assh_kex_dh_gex_state_e
{
#ifdef CONFIG_ASSH_CLIENT
  ASSH_KEX_DH_GEX_CLIENT_SEND_SIZE,
  ASSH_KEX_DH_GEX_CLIENT_WAIT_GROUP,
  ASSH_KEX_DH_GEX_CLIENT_WAIT_F,
  ASSH_KEX_DH_GEX_CLIENT_LOOKUP_HOST_KEY_WAIT,
#endif
#ifdef CONFIG_ASSH_SERVER
  ASSH_KEX_DH_GEX_SERVER_WAIT_SIZE,
  ASSH_KEX_DH_GEX_SERVER_WAIT_E,
#endif
};

#ifdef CONFIG_ASSH_SERVER
/* t[i] = N where 2^(1024 + i * 8) - N is the
   greatest safe prime under a power of 2 */
extern const uint32_t assh_kex_dh_gex_groups[(DH_MAX_GRSIZE-1024) / 8];
#endif

struct assh_kex_dh_gex_private_s
{
  const struct assh_hash_algo_s *hash;

  /* minimum and favorite group sizes */
  size_t algo_min;
  size_t algo_n;

  /* server retained group size */
  size_t server_n;

  /* exponent size */
  size_t exp_n;

  enum assh_kex_dh_gex_state_e state;

  struct assh_bignum_s pn;

  union {
#ifdef CONFIG_ASSH_SERVER
    struct {
      /* client requested group sizes */
      uint32_t client_min;
      uint32_t client_n;
      uint32_t client_max;
      uint8_t request_type;
    };
#endif
#ifdef CONFIG_ASSH_CLIENT
    struct {
      struct assh_bignum_s gn;
      struct assh_bignum_s en;
      struct assh_bignum_s xn;
      const struct assh_key_s *host_key;
      struct assh_packet_s *pck;
    };
#endif
  };
};

#ifdef CONFIG_ASSH_CLIENT
static assh_error_t assh_kex_dh_gex_client_send_size(struct assh_session_s *s)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  struct assh_context_s *c = s->ctx;
  assh_error_t err;

  struct assh_packet_s *p;

#ifndef ASSH_USE_DH_GEX_REQUEST_OLD
  ASSH_ERR_RET(assh_packet_alloc(c, SSH_MSG_KEX_DH_GEX_REQUEST, 3 * 4, &p)
	       | ASSH_ERRSV_DISCONNECT);

  ASSH_ASSERT(assh_packet_add_u32(p, pv->algo_min));
  ASSH_ASSERT(assh_packet_add_u32(p, pv->algo_n));
  ASSH_ASSERT(assh_packet_add_u32(p, DH_MAX_GRSIZE));
#else
  /* for test purpose */
  ASSH_ERR_RET(assh_packet_alloc(c, SSH_MSG_KEX_DH_GEX_REQUEST_OLD, 4, &p)
	       | ASSH_ERRSV_DISCONNECT);

  ASSH_ASSERT(assh_packet_add_u32(p, pv->algo_n));
#endif

  assh_transport_push(s, p);
  pv->state = ASSH_KEX_DH_GEX_CLIENT_WAIT_GROUP;
  return ASSH_OK;
}

static assh_error_t assh_kex_dh_gex_client_wait_group(struct assh_session_s *s,
                                                      struct assh_packet_s *p)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  assh_error_t err;

  ASSH_CHK_RET(p->head.msg != SSH_MSG_KEX_DH_GEX_GROUP, ASSH_ERR_PROTOCOL
	       | ASSH_ERRSV_DISCONNECT);

  uint8_t *p_str = p->head.end;
  uint8_t *g_str;

  ASSH_ERR_RET(assh_packet_check_string(p, p_str, &g_str)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, g_str, NULL)
	       | ASSH_ERRSV_DISCONNECT);

  size_t n;
  ASSH_ERR_RET(assh_bignum_size_of_data(ASSH_BIGNUM_MPINT, p_str, NULL, NULL, &n)
               | ASSH_ERRSV_DISCONNECT);

#ifdef CONFIG_ASSH_DEBUG_KEX
  ASSH_DEBUG("kex_dh_gex server group size: %u\n", n);
#endif

  ASSH_CHK_RET(n < pv->algo_min, ASSH_ERR_WEAK_ALGORITHM | ASSH_ERRSV_DISCONNECT);
  ASSH_CHK_RET(n > DH_MAX_GRSIZE, ASSH_ERR_NOTSUP | ASSH_ERRSV_DISCONNECT);

  pv->server_n = n;

  size_t e_size = assh_bignum_size_of_bits(ASSH_BIGNUM_MPINT, n);

  /* send a packet containing e */
  struct assh_packet_s *pout;
  ASSH_ERR_RET(assh_packet_alloc(s->ctx, SSH_MSG_KEX_DH_GEX_INIT,
                 e_size, &pout)
	       | ASSH_ERRSV_DISCONNECT);

  uint8_t *e_str;
  ASSH_ASSERT(assh_packet_add_array(pout, e_size, &e_str));

  enum bytecode_args_e
  {
    G_mpint, P_mpint, E_mpint,
    G, P, E, X,
    T1, T2, Q, G_n, MT
  };

  static const assh_bignum_op_t bytecode[] = {

    ASSH_BOP_SIZER(     G,      Q,      G_n            	),

    ASSH_BOP_MOVE(      G,      G_mpint                 ),
    ASSH_BOP_MOVE(      P,      P_mpint                 ),

#ifdef CONFIG_ASSH_DEBUG_KEX
    ASSH_BOP_PRINT(     G,      'G'                     ),
    ASSH_BOP_PRINT(     P,      'P'                     ),
#endif

    /* check prime */
    ASSH_BOP_UINT(      T1,     1                       ),
    ASSH_BOP_TESTS(     P,      1,      G_n,    0       ),
#if 0
    ASSH_BOP_ISPRIM(    P,      0                       ),
    ASSH_BOP_SUB(       T2,     P,      T1              ),
    ASSH_BOP_SHR(       Q,      T2,     1, ASSH_BOP_NOREG	),
    ASSH_BOP_ISPRIM(    Q,      0                       ),
#endif

    /* check generator */
    ASSH_BOP_CMPLT(     T1,     G,      0               ), /* g > 1 */
    ASSH_BOP_SUB(       T2,     P,      T1              ),
    ASSH_BOP_CMPLT(     G,      T2,     0               ), /* g < p - 1 */

    /* generate private exponent */
    ASSH_BOP_UINT(      T1,     DH_MAX_GRSIZE           ),
    ASSH_BOP_RAND(      X,      T1,     P,
                        ASSH_PRNG_QUALITY_EPHEMERAL_KEY),

    /* compute dh public key */
    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_MTTO(      T1,     T1,     G,      MT      ),
    ASSH_BOP_EXPM(      E,      T1,     X,      MT      ),
    ASSH_BOP_MTFROM(    E,      E,      E,      MT      ),

    ASSH_BOP_MOVE(      E_mpint,        E               ),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(s->ctx, bytecode, "MMMNNNNTTTsm",
                                    /* M */ g_str, p_str, e_str,
                                    /* N */ &pv->gn, &pv->pn, &pv->en, &pv->xn,
                                    /* S */ n), err_p);

  assh_packet_string_resized(pout, e_str + 4);

  assh_transport_push(s, pout);
  pv->state = ASSH_KEX_DH_GEX_CLIENT_WAIT_F;
  return ASSH_OK;

 err_p:
  assh_packet_release(pout);
  return err;

  return ASSH_OK;
}

static ASSH_EVENT_DONE_FCN(assh_kex_dh_gex_host_key_lookup_done)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  assh_error_t err;

  ASSH_CHK_RET(pv->state != ASSH_KEX_DH_GEX_CLIENT_LOOKUP_HOST_KEY_WAIT,
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
                     assh_bignum_size_of_bits(ASSH_BIGNUM_MPINT, pv->server_n) +
                     pv->hash->ctx_size,
                     ASSH_ERRSV_CONTINUE, err_);

  void *hash_ctx = scratch;
  uint8_t *secret = scratch + pv->hash->ctx_size;

  enum bytecode_args_e
  {
    F_mpint, K_mpint,
    X, G, P,
    F, T, K, MT
  };

  static const assh_bignum_op_t bytecode[] = {

#ifdef CONFIG_ASSH_DEBUG_KEX
    ASSH_BOP_PRINT(     G,      'G'             	),
    ASSH_BOP_PRINT(     P,      'P'             	),
#endif

    ASSH_BOP_SIZER(     F,      K,      P       	),

    ASSH_BOP_MOVE(      F,      F_mpint         	),

#ifdef CONFIG_ASSH_DEBUG_KEX
    ASSH_BOP_PRINT(     F,      'F'             	),
    ASSH_BOP_PRINT(     T,      'T'             	),
    ASSH_BOP_PRINT(     X,      'X'             	),
#endif

    /* check server public exponent */
    ASSH_BOP_UINT(      T,      2               	),
    ASSH_BOP_CMPLTEQ(   T,      F,      0 /* f >= 2 */	), 
    ASSH_BOP_SUB(       T,      P,      T       	),
    ASSH_BOP_CMPLTEQ(   F,      T,      0 /* f <= p-2 */), 

    /* FIXME check that log_g(pub) is not trivial */

    /* compute shared secret */
    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_MTTO(      F,      F,      F,      MT      ),
    ASSH_BOP_EXPM(      T,      F,      X,      MT      ),
    ASSH_BOP_MTFROM(    K,	K,      T,	MT     	),

    /* check shared secret range */
    ASSH_BOP_UINT(      T,      2               	),
    ASSH_BOP_CMPLTEQ(   T,      K,      0 /* k >= 2 */	),
    ASSH_BOP_SUB(       T,      P,      T       	),
    ASSH_BOP_CMPLTEQ(   K,      T,      0 /* k <= p-2 */),

    ASSH_BOP_MOVE(      K_mpint,        K       	),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(s->ctx, bytecode, "MMNNNTTTm",
                 /* M */ f_str, secret,
                 /* N */ &pv->xn, &pv->gn, &pv->pn,
                 /* T */ pv->server_n, pv->server_n, pv->server_n),
               err_scratch);

  ASSH_ERR_GTO(assh_hash_init(s->ctx, hash_ctx, pv->hash), err_scratch);

  ASSH_ERR_GTO(assh_kex_client_hash1(s, hash_ctx, ks_str)
               | ASSH_ERRSV_DISCONNECT, err_hash);

  uint8_t bit_sizes[12];

  assh_store_u32(bit_sizes + 0, pv->algo_min);
  assh_store_u32(bit_sizes + 4, pv->algo_n);
  assh_store_u32(bit_sizes + 8, DH_MAX_GRSIZE);

#ifndef ASSH_USE_DH_GEX_REQUEST_OLD
  assh_hash_update(hash_ctx, bit_sizes, 12);
#else
  assh_hash_update(hash_ctx, bit_sizes + 4, 4);
#endif

  ASSH_ERR_GTO(assh_hash_bignum(s->ctx, hash_ctx, &pv->pn)
	       | ASSH_ERRSV_DISCONNECT, err_hash);
  ASSH_ERR_GTO(assh_hash_bignum(s->ctx, hash_ctx, &pv->gn)
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

static assh_error_t assh_kex_dh_gex_client_wait_f(struct assh_session_s *s,
                                                  struct assh_packet_s *p,
                                                  struct assh_event_s *e)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  assh_error_t err;

  ASSH_CHK_RET(p->head.msg != SSH_MSG_KEX_DH_GEX_REPLY, ASSH_ERR_PROTOCOL
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
                                       &assh_kex_dh_gex_host_key_lookup_done, pv));

  pv->state = ASSH_KEX_DH_GEX_CLIENT_LOOKUP_HOST_KEY_WAIT;
  pv->pck = assh_packet_refinc(p);

  return ASSH_OK;
}
#endif

#ifdef CONFIG_ASSH_SERVER
static assh_error_t assh_kex_dh_gex_server_wait_size(struct assh_session_s *s,
                                                     struct assh_packet_s *p)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  struct assh_context_s *c = s->ctx;
  assh_error_t err;

  uint8_t *next = p->head.end;
  size_t min, max;
  pv->request_type = p->head.msg;

  switch (pv->request_type)
    {
    case SSH_MSG_KEX_DH_GEX_REQUEST:
      ASSH_ERR_RET(assh_packet_check_u32(p, &pv->client_min, next, &next)
                   | ASSH_ERRSV_DISCONNECT);
      ASSH_ERR_RET(assh_packet_check_u32(p, &pv->client_n, next, &next)
                   | ASSH_ERRSV_DISCONNECT);
      ASSH_ERR_RET(assh_packet_check_u32(p, &pv->client_max, next, &next)
                   | ASSH_ERRSV_DISCONNECT);

      /* check group size bounds */
      ASSH_CHK_RET(pv->client_n > pv->client_max ||
                   pv->client_n < pv->client_min || pv->client_n < 1024,
                   ASSH_ERR_BAD_DATA | ASSH_ERRSV_DISCONNECT);

      ASSH_CHK_RET(pv->client_min > DH_MAX_GRSIZE,
                   ASSH_ERR_NOTSUP | ASSH_ERRSV_DISCONNECT);

      /* get group size intervals intersection */
      min = ASSH_MAX(pv->algo_min, pv->client_min);
      max = ASSH_MIN(pv->client_max, DH_MAX_GRSIZE);
      break;

    case SSH_MSG_KEX_DH_GEX_REQUEST_OLD:
      ASSH_ERR_RET(assh_packet_check_u32(p, &pv->client_n, next, &next)
                   | ASSH_ERRSV_DISCONNECT);
      pv->client_min = pv->client_max = 0;
      min = pv->algo_min;
      max = 8192;
      break;

    default:
      ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
    }

  min = ASSH_MAX(min, pv->client_n - pv->client_n / 16);
  max = ASSH_MIN(max, pv->client_n + pv->client_n / 8);

  min = ASSH_ALIGN8(min);
  max -= max % 8;

  ASSH_CHK_RET(max < min, ASSH_ERR_WEAK_ALGORITHM | ASSH_ERRSV_DISCONNECT);

  /* randomize group size */
  uint16_t r;
  ASSH_ERR_RET(assh_prng_get(s->ctx, (void*)&r, 2, ASSH_PRNG_QUALITY_NONCE));
  r = r % (max - min + 1);
  r -= r % 8;
  pv->server_n = min + r;

#ifdef CONFIG_ASSH_DEBUG_KEX
  ASSH_DEBUG("kex_dh_gex client requested group size %u in [%u, %u]\n",
             pv->client_n, pv->client_min, pv->client_max);
  ASSH_DEBUG("kex_dh_gex server selected group size %zu in [%zu, %zu]\n",
             pv->server_n, min, max);
#endif

  size_t p_size = assh_bignum_size_of_bits(ASSH_BIGNUM_MPINT, pv->server_n);

  struct assh_packet_s *pout;
  ASSH_ERR_RET(assh_packet_alloc(s->ctx, SSH_MSG_KEX_DH_GEX_GROUP,
                 /* p */ p_size + /* g */ 5, &pout)
	       | ASSH_ERRSV_DISCONNECT);

  /* Append P */
  uint8_t *p_str;
  ASSH_ASSERT(assh_packet_add_array(pout, p_size, &p_str));

  enum bytecode_args_e
  {
    P_mpint,
    P_n, P_x, P,
    T
  };

  static const assh_bignum_op_t bytecode[] = {

    ASSH_BOP_SIZE (     P,      P_n                     ),
    ASSH_BOP_SIZEM(     T,      P,      1,      1       ),

    /* P = 2^n - P_x */
    ASSH_BOP_UINT(      T,      1                       ),
    ASSH_BOP_SHL(       T,      T,      0,      P       ),
    ASSH_BOP_MOVE(      P,      P_x                     ),
    ASSH_BOP_SUB(       T,      T,      P               ),
    ASSH_BOP_MOVE(      P,      T                       ),
    ASSH_BOP_MOVE(      P_mpint,        P               ),

#ifdef CONFIG_ASSH_DEBUG_KEX
    ASSH_BOP_PRINT(     P,      'P'                     ),
#endif

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "MsiNT", p_str,
    pv->server_n, (uintptr_t)assh_kex_dh_gex_groups[(pv->server_n - 1024) / 8],
    &pv->pn), err_p);

  assh_packet_string_resized(pout, p_str + 4);

  /* Append G = 2 */
  uint8_t *g_str;
  ASSH_ASSERT(assh_packet_add_array(pout, 5, &g_str));
  assh_store_u32(g_str, 1);
  g_str[4] = 2;

  assh_transport_push(s, pout);
  pv->state = ASSH_KEX_DH_GEX_SERVER_WAIT_E;
  return ASSH_OK;

 err_p:
  assh_packet_release(pout);
  return err;
}

static assh_error_t assh_kex_dh_gex_server_wait_e(struct assh_session_s *s,
                                                  struct assh_packet_s *p)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  struct assh_context_s *c = s->ctx;
  assh_error_t err;

  ASSH_CHK_RET(p->head.msg != SSH_MSG_KEX_DH_GEX_INIT,
	       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  /* compute DH */
  uint8_t *e_str = p->head.end;

  ASSH_ERR_RET(assh_packet_check_string(p, e_str, NULL)
	       | ASSH_ERRSV_DISCONNECT);

  size_t l = assh_bignum_size_of_num(ASSH_BIGNUM_MPINT, &pv->pn);

  ASSH_SCRATCH_ALLOC(c, uint8_t, scratch,
                     l + pv->hash->ctx_size,
                     ASSH_ERRSV_CONTINUE, err_);

  void *hash_ctx = scratch;
  uint8_t *secret = scratch + pv->hash->ctx_size;

  ASSH_ERR_GTO(assh_hash_init(s->ctx, hash_ctx, pv->hash), err_scratch);

  struct assh_packet_s *pout;
  const struct assh_key_s *hk;
  size_t slen;

  ASSH_ERR_GTO(assh_kex_server_hash1(s, l, hash_ctx, &pout,
                 &slen, &hk, SSH_MSG_KEX_DH_GEX_REPLY), err_hash);

  uint8_t *f_str;
  ASSH_ASSERT(assh_packet_add_array(pout, l, &f_str));

  enum bytecode_args_e
  {
    E_mpint, F_mpint, K_mpint,
    P, X_n,
    X, F, E, K, T, MT
  };

  static const assh_bignum_op_t bytecode[] = {

    ASSH_BOP_SIZE(      X,      X_n                     ),
    ASSH_BOP_SIZER(     F,      T,      P               ),

    ASSH_BOP_MOVE(      E,      E_mpint                 ),

#ifdef CONFIG_ASSH_DEBUG_KEX
    ASSH_BOP_PRINT(     P,      'P'                     ),
#endif

    /* check client public key */
    ASSH_BOP_UINT(      T,      2                       ),
    ASSH_BOP_CMPLTEQ(   T,      E,      0 /* f >= 2 */  ), 
    ASSH_BOP_SUB(       T,      P,      T               ),
    ASSH_BOP_CMPLTEQ(   E,      T,      0 /* f <= p-2 */), 

    /* FIXME check that log_g(pub) is not trivial */

    /* generate private exponent */
    ASSH_BOP_UINT(      T,      DH_MAX_GRSIZE   	),
    ASSH_BOP_RAND(      X,      T,      P,
                        ASSH_PRNG_QUALITY_EPHEMERAL_KEY),

    /* compute dh public key using 2 as generator */
    ASSH_BOP_UINT(      T,      2               	),

    ASSH_BOP_MTINIT(    MT,     P                       ),
    ASSH_BOP_MTTO(      T,      T,      T,      MT      ),
    ASSH_BOP_EXPM(      F,      T,      X,      MT      ),
    ASSH_BOP_MTFROM(    F,      F,      F,      MT      ),

    /* compute shared secret */
    ASSH_BOP_MTTO(      E,      E,      E,      MT      ),
    ASSH_BOP_EXPM(      K,      E,      X,      MT      ),
    ASSH_BOP_MTFROM(    K,      K,      K,      MT      ),

    /* check shared secret range */
    ASSH_BOP_UINT(      T,      2               	),
    ASSH_BOP_CMPLTEQ(   T,      K,      0 /* k >= 2 */	),
    ASSH_BOP_SUB(       T,      P,      T       	),
    ASSH_BOP_CMPLTEQ(   K,      T,      0 /* k <= p-2 */),

    ASSH_BOP_MOVE(      K_mpint,        K               ),
    ASSH_BOP_MOVE(      F_mpint,        F               ),

    ASSH_BOP_END(),
  };

  ASSH_ERR_GTO(assh_bignum_bytecode(c, bytecode, "MMMNsXTTTTm",
                   e_str, f_str, secret, &pv->pn,
                   pv->exp_n), err_p);

  assh_packet_string_resized(pout, f_str + 4);

  /* hash group sizes values */
  uint8_t buf[12];
  assh_store_u32(buf + 4, pv->client_n);

  switch (pv->request_type)
    {
    case SSH_MSG_KEX_DH_GEX_REQUEST:
      assh_store_u32(buf + 0, pv->client_min);
      assh_store_u32(buf + 8, pv->client_max);
      assh_hash_update(hash_ctx, buf, 12);
      break;

    case SSH_MSG_KEX_DH_GEX_REQUEST_OLD:
      assh_hash_update(hash_ctx, buf + 4, 4);
      break;
    }

  /* hash P */
  ASSH_ERR_GTO(assh_hash_bignum(s->ctx, hash_ctx, &pv->pn)
	       | ASSH_ERRSV_DISCONNECT, err_p);

  /* hash G */
  assh_store_u32(buf, 1);  
  buf[4] = 2;
  assh_hash_update(hash_ctx, buf, 5);

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


static ASSH_KEX_PROCESS_FCN(assh_kex_dh_gex_process)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  assh_error_t err;

  switch (pv->state)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_KEX_DH_GEX_CLIENT_SEND_SIZE:
      assert(p == NULL);
      ASSH_ERR_RET(assh_kex_dh_gex_client_send_size(s)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;

    case ASSH_KEX_DH_GEX_CLIENT_WAIT_GROUP:
      if (p == NULL)
        return ASSH_OK;
      ASSH_ERR_RET(assh_kex_dh_gex_client_wait_group(s, p)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;

    case ASSH_KEX_DH_GEX_CLIENT_WAIT_F:
      if (p == NULL)
        return ASSH_OK;
      ASSH_ERR_RET(assh_kex_dh_gex_client_wait_f(s, p, e)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;

    case ASSH_KEX_DH_GEX_CLIENT_LOOKUP_HOST_KEY_WAIT:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
#endif

#ifdef CONFIG_ASSH_SERVER
    case ASSH_KEX_DH_GEX_SERVER_WAIT_SIZE:
      if (p == NULL)
        return ASSH_OK;
      ASSH_ERR_RET(assh_kex_dh_gex_server_wait_size(s, p)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;

    case ASSH_KEX_DH_GEX_SERVER_WAIT_E:
      if (p == NULL)
        return ASSH_OK;
      ASSH_ERR_RET(assh_kex_dh_gex_server_wait_e(s, p)
		   | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
#endif
    }

  abort();
}

static assh_error_t assh_kex_dh_gex_init(struct assh_session_s *s,
                                         const struct assh_hash_algo_s *hash,
                                         size_t cipher_key_size,
                                         uint_fast8_t ldiv, uint_fast8_t hdiv)
{
  assh_error_t err;
  struct assh_kex_dh_gex_private_s *pv;

  size_t exp_n = cipher_key_size * 2;

  /* allocate DH private context */
  ASSH_ERR_RET(assh_alloc(s->ctx, sizeof(*pv), ASSH_ALLOC_INTERNAL, (void**)&pv)
	       | ASSH_ERRSV_DISCONNECT);

  switch (s->ctx->type)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_CLIENT:
      pv->state = ASSH_KEX_DH_GEX_CLIENT_SEND_SIZE;
      break;
#endif
#ifdef CONFIG_ASSH_SERVER
    case ASSH_SERVER:
      pv->state = ASSH_KEX_DH_GEX_SERVER_WAIT_SIZE;
      break;
#endif
    default:
      abort();
    }

  s->kex_pv = pv;
  pv->hash = hash;
  pv->algo_min = ASSH_MIN(ASSH_MAX(cipher_key_size * cipher_key_size / hdiv, 1024), DH_MAX_GRSIZE);
  pv->algo_n = ASSH_MIN(ASSH_MAX(cipher_key_size * cipher_key_size / ldiv, 1024), DH_MAX_GRSIZE);
  pv->exp_n = cipher_key_size * 2;

#ifdef CONFIG_ASSH_DEBUG_KEX
  ASSH_DEBUG("kex_dh_gex request algo_n:%u bits, algo_min:%u bits, exp_n:%u bits\n",
             pv->algo_n, pv->algo_min, pv->exp_n);
#endif

  assh_bignum_init(s->ctx, &pv->pn, 0, 0);

  switch (s->ctx->type)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_CLIENT:
      pv->host_key = NULL;
      pv->pck = NULL;

      assh_bignum_init(s->ctx, &pv->gn, 0, 0);
      assh_bignum_init(s->ctx, &pv->en, 0, 0);
      assh_bignum_init(s->ctx, &pv->xn, exp_n, 1);
      break;
#endif

#ifdef CONFIG_ASSH_SERVER
    case ASSH_SERVER:
      break;
#endif
    default:
      abort();
    }

  return ASSH_OK;
}

static ASSH_KEX_CLEANUP_FCN(assh_kex_dh_gex_cleanup)
{
  struct assh_kex_dh_gex_private_s *pv = s->kex_pv;
  struct assh_context_s *c = s->ctx;

  assh_bignum_release(c, &pv->pn);

  switch (c->type)
    {
#ifdef CONFIG_ASSH_CLIENT
    case ASSH_CLIENT:
      assh_bignum_release(c, &pv->en);
      assh_bignum_release(c, &pv->xn);
      assh_bignum_release(c, &pv->gn);
      assh_key_flush(c, &pv->host_key);
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

static ASSH_KEX_INIT_FCN(assh_kex_dh_gex_sha1_init)
{
  return assh_kex_dh_gex_init(s, &assh_hash_sha1, cipher_key_size, 12, 16);
}

const struct assh_algo_kex_s assh_kex_dh_gex_sha1 =
{
  .algo = { .name = "diffie-hellman-group-exchange-sha1",
            .class_ = ASSH_ALGO_KEX,
            .safety = 20, .speed = 30 },
  .f_init = assh_kex_dh_gex_sha1_init,
  .f_cleanup = assh_kex_dh_gex_cleanup,
  .f_process = assh_kex_dh_gex_process,
};


static ASSH_KEX_INIT_FCN(assh_kex_dh_gex_sha256_12_init)
{
  return assh_kex_dh_gex_init(s, &assh_hash_sha256, cipher_key_size, 12, 16);
}

const struct assh_algo_kex_s assh_kex_dh_gex_sha256_12 =
{
  .algo = { .name = "diffie-hellman-group-exchange-sha256",
            .variant = "n^2/12 bits modulus",
            .class_ = ASSH_ALGO_KEX,
            .priority = 10, .safety = 20, .speed = 30 },
  .f_init = assh_kex_dh_gex_sha256_12_init,
  .f_cleanup = assh_kex_dh_gex_cleanup,
  .f_process = assh_kex_dh_gex_process,
};


static ASSH_KEX_INIT_FCN(assh_kex_dh_gex_sha256_8_init)
{
  return assh_kex_dh_gex_init(s, &assh_hash_sha256, cipher_key_size, 8, 12);
}

const struct assh_algo_kex_s assh_kex_dh_gex_sha256_8 =
{
  .algo = { .name = "diffie-hellman-group-exchange-sha256",
            .variant = "n^2/8 bits modulus",
            .class_ = ASSH_ALGO_KEX,
            .priority = 9, .safety = 25, .speed = 20 },
  .f_init = assh_kex_dh_gex_sha256_8_init,
  .f_cleanup = assh_kex_dh_gex_cleanup,
  .f_process = assh_kex_dh_gex_process,
};


static ASSH_KEX_INIT_FCN(assh_kex_dh_gex_sha256_4_init)
{
  return assh_kex_dh_gex_init(s, &assh_hash_sha256, cipher_key_size, 4, 8);
}

const struct assh_algo_kex_s assh_kex_dh_gex_sha256_4 =
{
  .algo = { .name = "diffie-hellman-group-exchange-sha256",
            .variant = "n^2/4 bits modulus",
            .class_ = ASSH_ALGO_KEX,
            .priority = 8, .safety = 40, .speed = 10 },
  .f_init = assh_kex_dh_gex_sha256_4_init,
  .f_cleanup = assh_kex_dh_gex_cleanup,
  .f_process = assh_kex_dh_gex_process,
};

#ifdef CONFIG_ASSH_SERVER
const uint32_t assh_kex_dh_gex_groups[(DH_MAX_GRSIZE-1024) / 8] =
{
  0x0010aed9, 0x0008ba99, 0x00092ca5, 0x0007b58d, 0x000168c5, 0x00068fa5, 0x0013aae9, 0x0000c9a1,
  0x00129875, 0x003860c9, 0x00162ec9, 0x00054041, 0x000f1601, 0x000579ad, 0x00308819, 0x000a86e9,
  0x0000de89, 0x00024a79, 0x000cef51, 0x001f6001, 0x000dc841, 0x0014d311, 0x002b7efd, 0x0002008d,
  0x00287849, 0x00031fa9, 0x0015c065, 0x000ae149, 0x00056b3d, 0x000c8bb5, 0x00075b51, 0x000cf569,
  0x00080be5, 0x0007bfdd, 0x00015845, 0x00316c19, 0x00037de1, 0x00060521, 0x00224d35, 0x0015ce75,
  0x00062fbd, 0x0060e1e9, 0x0023bcf1, 0x00087d01, 0x0001f535, 0x0000bc51, 0x002a2df5, 0x0038c8c1,
  0x0016702d, 0x0004e545, 0x00146cc9, 0x00058475, 0x001ced49, 0x00086195, 0x007c20e9, 0x0019b215,
  0x0090ddb1, 0x0014ecf1, 0x000e3a11, 0x00055289, 0x00196619, 0x00008d4d, 0x00148d51, 0x002909f9,
  0x0016f055, 0x002c96bd, 0x00086b31, 0x000c8351, 0x00163f49, 0x0020917d, 0x0034cc85, 0x00029add,
  0x001e9acd, 0x000f35c9, 0x00070799, 0x004427e1, 0x0002f89d, 0x001e1ab1, 0x0034987d, 0x000c5ec1,
  0x000352a9, 0x001c8fa1, 0x001d3af5, 0x000293a5, 0x00023171, 0x001800f5, 0x0049bd61, 0x002dd3f1,
  0x00132515, 0x00080519, 0x000a8e75, 0x00283655, 0x005b37c9, 0x000a8f1d, 0x000694cd, 0x003bb559,
  0x002b3bb9, 0x001e6b15, 0x00508c9d, 0x0002725d, 0x0004e94d, 0x000d2ee1, 0x002fec3d, 0x0059cbcd,
  0x001dc585, 0x00019b4d, 0x0047bcc1, 0x001e997d, 0x001a3945, 0x00049055, 0x00029b55, 0x00048101,
  0x005e5c65, 0x001ae331, 0x00dd4905, 0x003aa8fd, 0x003b713d, 0x00520a51, 0x0038bcfd, 0x00088d69,
  0x000c69a1, 0x0004e5a5, 0x0017b4f9, 0x0036b315, 0x003f6b75, 0x00117761, 0x007ab8ad, 0x001c26a1,
  0x001da311, 0x00461b19, 0x000e002d, 0x001a73c5, 0x000daa7d, 0x002b5ec9, 0x0008e8e9, 0x0067bdd9,
  0x000dcbdd, 0x003a2f59, 0x00579959, 0x0029b4c9, 0x0014f045, 0x0077c0b1, 0x005a5819, 0x001bf04d,
  0x00108509, 0x0016be15, 0x00199f0d, 0x00356681, 0x00189581, 0x00a43b79, 0x003041a5, 0x006ba179,
  0x00038459, 0x001ea865, 0x003adfc9, 0x002f2fc1, 0x00046a99, 0x00276a85, 0x002afd49, 0x003e48a5,
  0x000cdaed, 0x00401021, 0x002958ad, 0x00acc019, 0x0000dd75, 0x001c4e61, 0x00071591, 0x003decdd,
  0x00846569, 0x0003232d, 0x00e21579, 0x00375b8d, 0x00663101, 0x00633b15, 0x00182099, 0x00074381,
  0x002e567d, 0x00531acd, 0x008948f9, 0x00086b6d, 0x0051d391, 0x00280e95, 0x004775f9, 0x002ced51,
  0x007b6bf9, 0x00398441, 0x000527bd, 0x005425f1, 0x000261f5, 0x0058ee21, 0x0049f15d, 0x0003ecd5,
  0x0008c2b5, 0x0001cfb5, 0x0072b345, 0x00535505, 0x00183be1, 0x000acf19, 0x0024bdad, 0x005e25d5,
  0x000eda01, 0x001de42d, 0x000bb54d, 0x0003100d, 0x002e1255, 0x003ddc39, 0x00929b0d, 0x00110e0d,
  0x0039a2b9, 0x00b48399, 0x001d8fb5, 0x000cec51, 0x000310e5, 0x019b4ab1, 0x00487109, 0x0074abe1,
  0x0069c5ed, 0x00b78201, 0x0014ffed, 0x005b0205, 0x006819ad, 0x008fa4f1, 0x00a74ab5, 0x00753689,
  0x0151c121, 0x004836dd, 0x0027fd61, 0x003db335, 0x00c936ad, 0x0025a3ed, 0x00b5a501, 0x0096e0f9,
  0x005f194d, 0x00302d7d, 0x000b9f75, 0x0052eca1, 0x001e65d5, 0x000a5215, 0x005d35c9, 0x006bb775,
  0x00538205, 0x0022cba1, 0x0002cf69, 0x00639aa9, 0x003c2309, 0x00624a61, 0x003b1d6d, 0x014e762d,
  0x00949241, 0x00367f61, 0x00858f4d, 0x0049ba91, 0x00496f49, 0x00a55225, 0x012dce41, 0x00e6b709,
  0x0010d765, 0x00a47065, 0x004f0d15, 0x0053eb65, 0x00403e11, 0x0001fe1d, 0x00591491, 0x00e68e4d,
  0x00837335, 0x00297905, 0x00febef5, 0x000e5ae1, 0x0098a25d, 0x00242009, 0x0094a0c9, 0x0005b781,
  0x0147856d, 0x00522e15, 0x0003b9c9, 0x01117229, 0x00012905, 0x00080519, 0x0012931d, 0x00122ecd,
  0x0106264d, 0x005f1809, 0x001e10d9, 0x00cedef9, 0x00256505, 0x00161c75, 0x00841745, 0x011a78dd,
  0x00533ac5, 0x00a394c9, 0x008a2ae9, 0x00334de1, 0x007cadb9, 0x004d76e9, 0x00999c71, 0x002ff645,
  0x001ed21d, 0x0033c79d, 0x009df511, 0x00db1ad5, 0x0075dfa9, 0x002cc3f9, 0x0018f761, 0x005da385,
  0x00bce63d, 0x0041851d, 0x01504cf1, 0x0108f059, 0x0084c725, 0x00f98255, 0x00175595, 0x0073ae91,
  0x0263f3f9, 0x009c91b5, 0x003282ad, 0x007f8701, 0x00a7a659, 0x0131c9ed, 0x00e3f561, 0x015f06a1,
  0x0028ad29, 0x019a41a9, 0x02d2962d, 0x009c73b5, 0x0069dc25, 0x0062cfd5, 0x000794d5, 0x00e55161,
  0x00281651, 0x0047ce91, 0x0075b56d, 0x0038bc49, 0x00355b71, 0x00138845, 0x000076b5, 0x00bb9925,
  0x002f4065, 0x00ea5ecd, 0x010d5e65, 0x0003ebf1, 0x008ac55d, 0x019c4bf1, 0x0111acc1, 0x002197e5,
  0x00537011, 0x0001c655, 0x00392cb1, 0x00bd0389, 0x00054f41, 0x005aea41, 0x003957f5, 0x01cee5b5,
  0x00116ad1, 0x0040ba6d, 0x001c6301, 0x032b0265, 0x00786ef9, 0x00507131, 0x0006409d, 0x00599669,
  0x0067dd59, 0x00754409, 0x003baa55, 0x01364279, 0x002375a5, 0x00377ef1, 0x00acb275, 0x00bcf7d1,
  0x015f6539, 0x01a83781, 0x0073e869, 0x006d2d85, 0x012a3781, 0x003f3341, 0x00659b25, 0x002fc195,
  0x00886361, 0x02117ec1, 0x004ff661, 0x01ebf129, 0x01467f41, 0x00095b31, 0x000ccff5, 0x00341b91,
  0x00a63f49, 0x0022d159, 0x003ddf09, 0x004e03dd, 0x02891bd9, 0x009d6d15, 0x009ad1f5, 0x01ed84d9,
  0x0029aa9d, 0x0060eefd, 0x00324d19, 0x00416f75, 0x00af9019, 0x003a1609, 0x00881c99, 0x009618b9,
  0x0017d1f1, 0x0106756d, 0x010d6ac5, 0x0259fe4d, 0x01f01c09, 0x004c843d, 0x00d5493d, 0x026f56a9,
  0x009e236d, 0x00fb242d, 0x00cd19b1, 0x0055a6c9, 0x001ac801, 0x0025d729, 0x009a942d, 0x00cd81a9,
  0x0195b68d, 0x0269c585, 0x0042d445, 0x00a8cbed, 0x014862e9, 0x0083c579, 0x0011f231, 0x01e353b1,
  0x00c6d769, 0x00f6469d, 0x008cc2fd, 0x00973481, 0x00d6834d, 0x02eabf91, 0x000aa969, 0x0015c479,
  0x011659f1, 0x005162e1, 0x033489b9, 0x00d9f21d, 0x006c6815, 0x003d8479, 0x00015be1, 0x00be9661,
  0x049927f1, 0x05b14881, 0x016d4bf9, 0x00e87d41, 0x0044920d, 0x0241a651, 0x001ab271, 0x001da959,
  0x01b3e441, 0x00426281, 0x001e2291, 0x0096afd9, 0x016d9b31, 0x010b77b1, 0x0030556d, 0x03856aed,
  0x009807f5, 0x001c5cc5, 0x025ec725, 0x0099e7d1, 0x005aa45d, 0x00f6a63d, 0x00a69dc9, 0x0025126d,
  0x00094d39, 0x01b62279, 0x01093abd, 0x013d7ea5, 0x01735ec5, 0x01522abd, 0x005b4f39, 0x01c3e46d,
  0x005f4599, 0x00da79e9, 0x00335d7d, 0x020d3095, 0x007241f9, 0x01ba6505, 0x00237971, 0x01ccce29,
  0x0157a501, 0x0086edc9, 0x002e8995, 0x003dd639, 0x02610185, 0x00f526a9, 0x011f7c4d, 0x00512b31,
  0x0154af39, 0x0066d42d, 0x00eb75b5, 0x01748891, 0x001e4be9, 0x019af3c9, 0x00b04845, 0x005a19d9,
  0x032e4ba9, 0x0178b67d, 0x0067ce95, 0x00ce3bfd, 0x0099f389, 0x0013f5f5, 0x0026aeb1, 0x01375c3d,
  0x00d3e725, 0x00c13f01, 0x0083f4c5, 0x00387569, 0x01fb115d, 0x018d2455, 0x001e8771, 0x0322cb11,
  0x02c6f435, 0x02dba28d, 0x04174ad5, 0x01276529, 0x0088753d, 0x00f06eed, 0x01e109a9, 0x01365251,
  0x000f7b41, 0x02785631, 0x0149164d, 0x00b6878d, 0x010bc7d9, 0x00c5de45, 0x002b9aa5, 0x000e6d29,
  0x00429c05, 0x003fa541, 0x016689ad, 0x003e8e71, 0x01907b79, 0x0041fc75, 0x00235fa9, 0x028ea199,
  0x00502d09, 0x02aedfe9, 0x00d20fd1, 0x00e0c255, 0x00ad10d1, 0x0113e739, 0x037cd495, 0x00de913d,
  0x00e97b99, 0x00df0379, 0x0231ecb1, 0x00a5c04d, 0x0096415d, 0x025f138d, 0x0274c06d, 0x006a66cd,
  0x00578ee5, 0x0084865d, 0x0349fdbd, 0x03f308f9, 0x0381f269, 0x010bf3c5, 0x00472cd9, 0x0011a4fd,
  0x00336fc5, 0x015f36d1, 0x02a05bcd, 0x04f9e0c5, 0x01aad0fd, 0x00550c19, 0x017c49dd, 0x01017035,
  0x00bcc525, 0x02fdc479, 0x0392dbe1, 0x033654ad, 0x000d4411, 0x016f5b15, 0x02650bb9, 0x0013d999,
  0x005ee671, 0x00baa3cd, 0x0016af75, 0x003b1d19, 0x008a6749, 0x00bd9329, 0x00438f89, 0x0030322d,
  0x016cc9a9, 0x022eecf9, 0x005592dd, 0x01cce3dd, 0x03a2bc09, 0x0333c8d5, 0x00a3cd15, 0x01e853fd,
  0x04712ad1, 0x00088cf1, 0x00526c25, 0x0139aa95, 0x0132d1b1, 0x02663eb5, 0x0001b61d, 0x00cb99d5,
  0x017d3cad, 0x00aaf999, 0x00b6e63d, 0x0068bf31, 0x02f5a0ed, 0x001f087d, 0x02ba00a5, 0x0097cb1d,
  0x0118dd65, 0x02c0f7c5, 0x0068296d, 0x0165ee85, 0x00aca08d, 0x0258e099, 0x05a56111, 0x03068831,
  0x00cad9e1, 0x04bc7b11, 0x00939e21, 0x01ecb2a9, 0x002548cd, 0x01781c09, 0x02e94945, 0x018c487d,
  0x0028ba01, 0x013e31b5, 0x020722cd, 0x0050a3a1, 0x00ad4209, 0x01778885, 0x009d9139, 0x01b37055,
  0x0017c5e5, 0x050f2e71, 0x0373ced1, 0x0103e7e5, 0x00e5939d, 0x0273dd15, 0x01d5c049, 0x011e05a1,
  0x026585d5, 0x01119a25, 0x0193471d, 0x01794911, 0x02fe7fc9, 0x0097acbd, 0x002187f5, 0x000ebbc5,
  0x01320f65, 0x00171e69, 0x013ddc89, 0x0028de55, 0x01f0ab25, 0x0216f749, 0x009dcb89, 0x00449465,
  0x0142ee09, 0x04f952ed, 0x02c7d2dd, 0x002e3049, 0x03c7d6b1, 0x03c26c5d, 0x0055da95, 0x0082717d,
  0x0067fd51, 0x0234baad, 0x004405f1, 0x0065a755, 0x00a2c2c9, 0x011fbef5, 0x01e98de9, 0x03110585,
  0x021d2b45, 0x00aff1ed, 0x01bfc611, 0x00b1f34d, 0x0025ff9d, 0x004e8399, 0x0790fa99, 0x01a0e4e1,
  0x00ae2309, 0x029fca05, 0x00c868cd, 0x0213c1a9, 0x01d5a14d, 0x01043fa5, 0x054de6c5, 0x014c5241,
  0x001ada0d, 0x00837c59, 0x01f8c58d, 0x005ae5fd, 0x00540239, 0x004dd809, 0x00e9c0bd, 0x020705bd,
  0x008fc291, 0x02d5d131, 0x0100f871, 0x0312139d, 0x006b3e19, 0x03d24a2d, 0x092e9e7d, 0x04557a01,
  0x019da1cd, 0x03c72e2d, 0x007d455d, 0x02efe8f9, 0x013e4aa5, 0x0099b351, 0x079d8835, 0x020e79b1,
  0x009445b5, 0x01651e7d, 0x02e4d479, 0x02a5aa55, 0x0252bda1, 0x012cfe99, 0x020ceca9, 0x017696d5,
  0x02b42919, 0x0015a571, 0x00721aed, 0x0467fa05, 0x04ad76d9, 0x020b5185, 0x01a17751, 0x02fc4e2d,
  0x021226e5, 0x00227d7d, 0x013ee6f9, 0x03d29dfd, 0x077b58a5, 0x020af94d, 0x050b720d, 0x0044b0c1,
  0x04a78729, 0x00a15b7d, 0x1231423d, 0x013aedb1, 0x003f1a99, 0x003eacb9, 0x0058b1a9, 0x005a2681,
  0x089f29b5, 0x00a4acad, 0x0229c601, 0x0195b3d5, 0x06265f91, 0x01d42009, 0x00c54215, 0x0116083d,
  0x09794681, 0x03612f4d, 0x0ddb4c69, 0x0031fbe9, 0x002a4f49, 0x00c26381, 0x0332d425, 0x00dfa3c9,
  0x08101085, 0x0155a539, 0x006dfa99, 0x04db50a1, 0x0084a0b5, 0x014df0dd, 0x051a11a1, 0x02cee76d,
  0x00a1da55, 0x0219922d, 0x0d64c945, 0x02035499, 0x0273e68d, 0x02a83a41, 0x025ebcd5, 0x01bc574d,
  0x02666b9d, 0x00c103d9, 0x0b9203c1, 0x054be0e5, 0x00f845c9, 0x008c6c8d, 0x019dd359, 0x01d31401,
  0x005f3531, 0x009fb525, 0x015391fd, 0x001f47c5, 0x00e0623d, 0x00734201, 0x004cbbe1, 0x004f27f1,
  0x060ac6a9, 0x01c679ed, 0x002b2db5, 0x08690375, 0x015b2121, 0x0681f095, 0x017ccaa1, 0x03910d5d,
  0x011d6eed, 0x00db700d, 0x02384a7d, 0x004ba7e1, 0x04f04d89, 0x000daf15, 0x0075cfb9, 0x00563225,
  0x02965145, 0x01b25e41, 0x01088255, 0x02c20ce5, 0x065322f5, 0x04a89a5d, 0x005f8121, 0x040c6055,
  0x02a720e9, 0x00cc9695, 0x006cd8f5, 0x00f7fee9, 0x0095f4dd, 0x0077ecfd, 0x00d7ca05, 0x0249a9e5,
  0x01efb0ed, 0x0455f2f1, 0x01eae0ad, 0x009d8275, 0x01436c75, 0x007af195, 0x03603bf9, 0x0437ff45,
  0x0180d5f5, 0x01d75fb1, 0x016b3a0d, 0x019646ed, 0x0533f41d, 0x0284e991, 0x005bb16d, 0x07945ead,
  0x0026c4f5, 0x001b94c1, 0x04e65e55, 0x000bf2cd, 0x026375bd, 0x04968269, 0x02195af5, 0x098ba4a1,
  0x00f28355, 0x01bbb145, 0x02699b61, 0x00b76251, 0x02ab6945, 0x06d61aad, 0x04d5e971, 0x0089278d,
  0x013ce3d1, 0x047ae68d, 0x04133ae9, 0x019bc131, 0x03341369, 0x03792641, 0x002385b9, 0x01857251,
  0x0b7ea34d, 0x00212165, 0x000e7e39, 0x07bafc61, 0x044d7055, 0x0136a3a5, 0x0112ed01, 0x008114a5,
  0x01875029, 0x04da1cd9, 0x00451b11, 0x018ded49, 0x0a9c6679, 0x007fe881, 0x0301ecc5, 0x039ac049,
  0x0121dbb5, 0x01e8d1b5, 0x0057b621, 0x025cfa81, 0x0151d855, 0x022bae59, 0x05043c59, 0x0430f655,
  0x00e32649, 0x06d6f955, 0x01b4294d, 0x0578bf3d, 0x02c39ea9, 0x044958e9, 0x01511d35, 0x01e84b69,
  0x0299f801, 0x0375970d, 0x01ba552d, 0x0050dc11, 0x00cb8de1, 0x035a3cb9, 0x010e8d4d, 0x0312abb9,
  0x020d11a5, 0x06735cb9, 0x13b73d15, 0x079a56f1, 0x00d23f59, 0x05657ed9, 0x01e3e8e5, 0x01da8159,
  0x00e0bf6d, 0x0042a7e1, 0x060dfa09, 0x006009d1, 0x0520057d, 0x0061f72d, 0x056b450d, 0x008a00f5,
  0x01df7afd, 0x004558b5, 0x01296cc5, 0x0336e77d, 0x0803ebad, 0x0192de35, 0x00bc9951, 0x000f2669,
  0x01619bb5, 0x0105889d, 0x03d5c9b9, 0x01c047ad, 0x028b5345, 0x032c4e5d, 0x025f9da5, 0x01b2e0a9,
  0x024c5dad, 0x04de1fe1, 0x035d2ba9, 0x0027a5f5, 0x01689b39, 0x032495e1, 0x0109a9e1, 0x01394d1d,
  0x03199b7d, 0x01046075, 0x00de31d9, 0x03af4e25, 0x04175ec1, 0x00d6f415, 0x05e83ef9, 0x03a68d31,
  0x038d3c05, 0x013d2cfd, 0x0a050da9, 0x0141f4e5, 0x04de855d, 0x05ef6be9, 0x014c16e9, 0x018a1a11,
  0x097d2ec5, 0x01c88bfd, 0x003fd00d, 0x01f792cd, 0x009c28f1, 0x0596c20d, 0x03e4eddd, 0x063ec405,
  0x022aa095, 0x0231883d, 0x03a27c91, 0x0703ae49, 0x08e7c435, 0x0606f7fd, 0x00b6c789, 0x0ed5a4d9,
  0x01496369, 0x00bb57e5, 0x004bea35, 0x01d62499, 0x0e825b31, 0x002ad45d, 0x0115cedd, 0x02bd93a5,
  0x00335cc9, 0x049ebc39, 0x0229e6d1, 0x00c66db5, 0x00067739, 0x0188566d, 0x020c12c9, 0x04a985a1,
  0x0509bed1, 0x02c27be5, 0x021b2ccd, 0x031f89e9, 0x0037dd35, 0x01546dd5, 0x0960fe99, 0x00d28519,
  0x04019b0d, 0x0228fcb9, 0x0c9cc2c9, 0x00f677b1, 0x0102bf5d, 0x0355bdcd, 0x03a59fc5, 0x025cf78d,
  0x003f0be1, 0x01be09b1, 0x01df4839, 0x004ff805, 0x00e849f9, 0x010db15d, 0x053e9a5d, 0x007d5739,
  0x01ff65b9, 0x033cd0c1, 0x01949dad, 0x03f1ff0d, 0x0441af91, 0x03b50e1d, 0x055d8de1, 0x040590b9,
  0x00101411, 0x09633395, 0x01c340bd, 0x04134449, 0x08254a09, 0x03005339, 0x0a718e9d, 0x0024046d,
  0x02ddbb81, 0x0725a7c1, 0x048a6811, 0x04a5d681, 0x04f50b45, 0x005ba075, 0x000a762d, 0x03ee4f21,
  0x06408cad, 0x01c48919, 0x08c4dead, 0x0205c00d, 0x00070d81, 0x049515e9, 0x02b2dd51, 0x0478321d,
  0x02404a45, 0x0d05e1f5, 0x13c5532d, 0x01669f91, 0x01520dad, 0x05252699, 0x08ed4e8d, 0x03298895,
  0x01e7e1d9, 0x04b7de91, 0x037eb435, 0x01f6c5d1, 0x00b93c39, 0x01b27629, 0x0b10106d, 0x05e37db9,
  0x02ac17c9, 0x039ba059, 0x055eccdd, 0x0699bcb1, 0x017bcd69, 0x02ab2b89, 0x0375f46d, 0x05bf9e71,
  0x00888bbd, 0x007d7ed5, 0x04083755, 0x04457e19, 0x00aa97c5, 0x003eb169, 0x00208511, 0x01d4b375,
  0x075d1ee5, 0x0b108765, 0x0134d899, 0x03e15105, 0x008b71b9, 0x067b7331, 0x01c7398d, 0x077b5e21,
  0x014a92b1, 0x00d03c31, 0x049c8acd, 0x091b3f1d, 0x0365e235, 0x01829309, 0x10fbd0d9, 0x04cac1e9,
  0x00d6be45, 0x129c22e5, 0x0157d525, 0x01b8104d, 0x0126295d, 0x004b81e9, 0x0099e6f9, 0x00578f39,
  0x08c19725, 0x04568699, 0x0f068b0d, 0x0c872279, 0x018006e9, 0x03414461, 0x013eeeb5, 0x028dc86d,
  0x022e81dd, 0x0da25419, 0x02ba19dd, 0x000dc301, 0x01300919, 0x02278301, 0x04729f3d, 0x00928889,
  0x01028e31, 0x064df4a1, 0x044fcf81, 0x04e73c49, 0x001af885, 0x06f33d25, 0x01cce91d, 0x0618df55,
  0x0fb43d25, 0x06ce0bf1, 0x007e0dc1, 0x015c286d, 0x07110941, 0x0324799d, 0x010a60c9, 0x04e19ec5,
  0x01aec3c1, 0x02620d45, 0x1127f099, 0x01922b61, 0x01bd91bd, 0x064210b5, 0x010711a9, 0x03398429,
  0x017b4825, 0x08b36bad, 0x049b9029, 0x07627cc1, 0x025220b1, 0x00457529, 0x13477e11, 0x04ab006d,
  0x041589dd, 0x0d5d9739, 0x046326ad, 0x059afb99, 0x013a7449, 0x01512251, 0x081392a5, 0x08777735,
  0x19dbf9b5, 0x00afa009, 0x07f70ee5, 0x01edb941, 0x0364d14d, 0x06331ef5, 0x09cfc9f5, 0x074f800d,
  0x01c4cf5d, 0x0021646d, 0x004bbd1d, 0x02c64df1, 0x01be48c9, 0x03c1c595, 0x026dd1ed, 0x03fa0dd5,
  0x172ab571, 0x0221fda1, 0x12b6df1d, 0x033e2631, 0x0d6c0385, 0x0901d1e9, 0x0acea5e9, 0x02e234e5,
  0x006d8749, 0x07c2c515, 0x05e65c65, 0x0a9a6099, 0x063b5b71, 0x04b7f84d, 0x105f62d5, 0x001e21dd,
  0x016885f1, 0x008363ed, 0x025a5d21, 0x0bafaf25, 0x10cb83ed, 0x0049c661, 0x022dd521, 0x03a13c69,
  0x03681389, 0x03b16cdd, 0x00476561, 0x00bb6565, 0x0450d09d, 0x04b5dd3d, 0x00661abd, 0x011fde2d,
  0x01e50ced, 0x02f16275, 0x0252d6f1, 0x010e487d, 0x0554200d, 0x00673265, 0x0f9e188d, 0x0126fc11,
  0x01c333cd, 0x0153e159, 0x085eadbd, 0x046e08ed, 0x02f89a21, 0x002301c5, 0x02da98b9, 0x0388b629,
  0x065bebc9, 0x07b59c2d, 0x01734ae5, 0x02758fd9, 0x001c0fa9, 0x0f7fd9a5, 0x027acbdd, 0x05b41d55,
  0x0dd1e105, 0x0db66e99, 0x045c80a5, 0x011516d5, 0x05bac285, 0x015b5091, 0x01edbc35, 0x0c4d9405,
  0x072cb3ed, 0x079f11dd, 0x0232c625, 0x0708a24d, 0x018e9b3d, 0x01381a45, 0x02525045, 0x01a7580d,
  0x01ffc7b1, 0x06f6aa2d, 0x01e34ccd, 0x081262a9, 0x01621ccd, 0x033c1bad, 0x01ab14dd, 0x073097c9,
  0x070f9949, 0x0ad71d69, 0x0831f4b5, 0x1b801d85, 0x0060ea95, 0x00f3b351, 0x01039e7d, 0x071169ad,
  0x0288caa9, 0x038aa00d, 0x018d5221, 0x01719b2d, 0x0122f201, 0x046d1035, 0x02bf3fc1, 0x001138f1,
  0x01b13b65, 0x00bc20e5, 0x05974f61, 0x010d45c9, 0x065a9b5d, 0x0bf5c289, 0x03d996e5, 0x0344d221,
  0x006087e9, 0x0ac432b5, 0x0dcf7795, 0x046ff301, 0x017441b1, 0x0095f9a5, 0x0b5ba8c5, 0x15babf51,
  0x06fa4999, 0x1c2ac1c9, 0x10b88379, 0x00aa5d51, 0x04256a65, 0x0c4c1b85, 0x065ecc19, 0x01d1bb01,
  0x04ef895d, 0x0153b405, 0x013718b1, 0x1709467d, 0x00f032f9, 0x158fd425, 0x009a8e21, 0x032e49e1,
  0x043553bd, 0x070b2a89, 0x0ee6bf35, 0x05f2aed9, 0x0c1bae1d, 0x008f137d, 0x03df32dd, 0x0b22ef99,
  0x03169ffd, 0x05101c49, 0x05c759a5, 0x00002ab9, 0x05914af9, 0x02468309, 0x0323e93d, 0x09be6f25,
  0x0211642d, 0x0644d0dd, 0x0262a3c9, 0x0c4336b5, 0x0e06dd3d, 0x0564136d, 0x0941f89d, 0x184a4e75,
  0x08636ed9, 0x04f30145, 0x07d1bd4d, 0x0325bc59, 0x0ec09b79, 0x00cee9e5, 0x0862b131, 0x085f5c35,
  0x01a1ea11, 0x0b122e1d, 0x001893a1, 0x0f44b3f1, 0x041e38c5, 0x057535fd, 0x01c69b95, 0x1327468d,
  0x00d94c81, 0x0710416d, 0x078d5275, 0x02bf79a5, 0x0495c521, 0x090abd55, 0x075705e9, 0x0b47d459,
  0x024cc7f1, 0x052d9819, 0x001d4be1, 0x0b9ff23d, 0x00d7d79d, 0x003e5ee9, 0x06e2cf19, 0x010d4ea5,
  0x0c749345, 0x00015701, 0x037be111, 0x047d68c9, 0x032e2b39, 0x014dbfd5, 0x03887b25, 0x0565e521,
  0x0ffc7319, 0x034d29c1, 0x00452705, 0x06386e25, 0x041233f1, 0x11c29085, 0x03cc73d9, 0x0ef430ad,
  0x0426c611, 0x13340d05, 0x075b38c1, 0x0606f911, 0x00bbc121, 0x011c8655, 0x15342269, 0x101511d1,
  0x044f5c85, 0x03be02e9, 0x00553d99, 0x222499cd, 0x01109d7d, 0x0de25f55, 0x0e746c3d, 0x05c9e919,
  0x00e3d761, 0x0a43b8dd, 0x02514e21, 0x01bff2a5, 0x08305859, 0x032dd0c1, 0x0225c779, 0x0058675d,
  0x01f6ef71, 0x006c05bd, 0x05b78c19, 0x09a2cd01, 0x140c755d, 0x0d7b8ec9, 0x00213a49, 0x040c65a1,
  0x093519dd, 0x00099095, 0x0585643d, 0x0104bd69, 0x012601a9, 0x031309c1, 0x0169357d, 0x1c10e5d1,
  0x04414469, 0x0d1e7a95, 0x01de22c9, 0x059dcec9, 0x0471fbe1, 0x0312cae5, 0x0507de7d, 0x011ff68d,
  0x10bbd485, 0x003ff8b1, 0x0cae14c9, 0x04f89f65, 0x0ab8b175, 0x05c97155, 0x05832ba5, 0x0a037441,
  0x0309955d, 0x07bbd641, 0x123853f1, 0x047e9da5, 0x00fe0ba9, 0x0d824029, 0x0095a34d, 0x02ed83d9,
  0x075f1a69, 0x0039a9f1, 0x0842a4a9, 0x08a1eff5, 0x06fdcc6d, 0x00bfebc5, 0x063bfff9, 0x009ce741,
  0x13765811, 0x0355a2fd, 0x05815bc5, 0x06babe79, 0x070a0c99, 0x05516a11, 0x140ed80d, 0x0774ee49,
  0x133b5009, 0x0678253d, 0x01573571, 0x01f5022d, 0x01434cb9, 0x07c021d9, 0x06b793e9, 0x129125bd,
  0x022bb471, 0x00752eb5, 0x011a6f05, 0x0cf03e21, 0x14ffbf85, 0x0b38b1fd, 0x09d78151, 0x06f158ed,
  0x07f69589, 0x0ca9b371, 0x0e745b21, 0x00f3d7c9, 0x094000a9, 0x03aa4f65, 0x018708d1, 0x007719b9,
  0x09b448ad, 0x096ba1b5, 0x0a0d47a9, 0x075d96d9, 0x0ed1c33d, 0x08568605, 0x09d5b135, 0x00979d09,
  0x11e6a471, 0x08ac3815, 0x0369a12d, 0x008723ed, 0x2df4b40d, 0x002398d9, 0x02c5e209, 0x00f8eae1,
  0x06eed999, 0x04cf60f1, 0x085f0685, 0x04167761, 0x03f29af5, 0x18e0f9ed, 0x075d8401, 0x01b1fe7d,
  0x11b375a9, 0x0933997d, 0x06207089, 0x14277281, 0x0d9d04d1, 0x1548c545, 0x01b1ade9, 0x03297b2d,
  0x00194f81, 0x045404c9, 0x13071245, 0x00dcd159, 0x029418f5, 0x0538b1b5, 0x04142c15, 0x00f7abcd,
  0x0a5c6389, 0x030e3e85, 0x03943739, 0x067297d9, 0x06030db5, 0x083ada2d, 0x00eb65c5, 0x0a859645,
  0x033bd8bd, 0x042d62dd, 0x101744ed, 0x1aafa565, 0x102f2531, 0x0725bfa9, 0x004c5755, 0x03866fc9,
  0x07343771, 0x0b139521, 0x012343b5, 0x00768a0d, 0x152592e9, 0x2db5b67d, 0x050f6075, 0x0f91e07d,
  0x0b933ae9, 0x0308cddd, 0x04cd3825, 0x01eee649, 0x019d6621, 0x013cb359, 0x08562eed, 0x03d6441d,
  0x0f3bfa7d, 0x00884315, 0x0d7a1085, 0x035c6891, 0x29e12d8d, 0x07c0a789, 0x08877059, 0x053403d1,
  0x06cb0531, 0x01bd6265, 0x03123db5, 0x05f3f3d5, 0x09848bfd, 0x06727949, 0x04364b31, 0x00d6c1b1,
  0x04ae00f1, 0x0263356d, 0x140927bd, 0x017bf289, 0x001393c1, 0x074778a1, 0x07d77c0d, 0x0e7ba269,
  0x04e36e15, 0x03b5f559, 0x023dcbed, 0x06bb1f99, 0x0192a7f9, 0x03900f29, 0x058e0629, 0x10d0425d,
  0x143d0701, 0x034d5139, 0x07189085, 0x11a5d815, 0x04143731, 0x024b16f5, 0x0defa9b5, 0x07a741f9,
  0x00a24535, 0x1b4ccaed, 0x072b3849, 0x00ab0881, 0x092e8c11, 0x026da781, 0x0afea9f1, 0x09fe4131,
  0x0371f26d, 0x0cb2d975, 0x00a9b7d9, 0x02bc3e29, 0x06b8bc1d, 0x1cab2eed, 0x14cf9f21, 0x03121169,
  0x044e75e5, 0x053ddb1d, 0x010b4d81, 0x0ad91221, 0x005f8ca9, 0x0233d701, 0x07c47e69, 0x056f3b49,
  0x08b197e9, 0x034f3751, 0x200f7ebd, 0x047721e9, 0x0e51e7ed, 0x05a1ab79, 0x0dd6c7c5, 0x04821fe9,
  0x00af7eb5, 0x02c38061, 0x1428c029, 0x06b87681, 0x11acf19d, 0x014701b5, 0x2632a159, 0x17825f2d,
  0x05f42435, 0x0f89a371, 0x073c3679, 0x00d7b421, 0x076def6d, 0x08d8a2c9, 0x0a8eb60d, 0x075f8981,
  0x10e73031, 0x0e2c3f85, 0x0e5d3891, 0x0f1861f1, 0x02d99905, 0x0468d409, 0x009ebedd, 0x0243b8b5,
  0x013e1631, 0x0287977d, 0x0f002f15, 0x095f9bc1, 0x05d2ef31, 0x01ab891d, 0x07a4fc59, 0x02428439,
  0x03e07149, 0x09604b59, 0x0b792df5, 0x0428410d, 0x05e19675, 0x08212145, 0x149c0525, 0x0c350855,
  0x073b4e35, 0x0b155b29, 0x08e122b9, 0x0b6cdb2d, 0x01b2fbfd, 0x0468e8d9, 0x02cf2b29, 0x00e79cdd,
  0x13e26765, 0x01fb2531, 0x0dfd1041, 0x000942e9, 0x0158c099, 0x081a2f89, 0x0027c5c9, 0x11c0f465,
  0x09bce355, 0x0ab6ae11, 0x00598505, 0x00e4fae5, 0x0e64d7e1, 0x0075c3a1, 0x08d80e19, 0x08d30611,
  0x00c8fae9, 0x068ef691, 0x0954f3ad, 0x0b0298ad, 0x1cac8631, 0x075877a9, 0x0f0a3505, 0x0cbe23a1,
  0x06494be5, 0x11923949, 0x0975325d, 0x0abb7d19, 0x05f21141, 0x0ecf3879, 0x052d59a9, 0x02407a15,
  0x10e6c725, 0x15399971, 0x0ceb87b5, 0x3a80d93d, 0x040043ed, 0x0dd56e71, 0x0876a925, 0x0af7e8ad,
  0x0ee2040d, 0x137e1aa1, 0x050a5ea9, 0x00ca6b1d, 0x02b4bb95, 0x0920dfad, 0x00fce975, 0x0a9a461d,
  0x0b234ced, 0x0adfa329, 0x01954fe5, 0x0de124c1, 0x151d76a1, 0x037deaed, 0x0455965d, 0x058a2685,
  0x1ba188d1, 0x036045b9, 0x05844a01, 0x0f960419, 0x0dab3145, 0x07dbb449, 0x025f0811, 0x03e1a2c5,
  0x01c14aa9, 0x1ea697a5, 0x020eac45, 0x032bbf4d, 0x02741d65, 0x212dfa31, 0x11a94c61, 0x081410bd,
  0x1f657361, 0x091d2421, 0x16ca7c29, 0x0df028e1, 0x02cea0e1, 0x0a63e245, 0x0bdabe2d, 0x007451a5,
  0x0333c971, 0x189431c9, 0x16fa42a5, 0x03700dc9, 0x08821a09, 0x016d8fe5, 0x174f67e5, 0x07bb67f5,
  0x073a33b1, 0x029a60b9, 0x2d74ae75, 0x0fe66f51, 0x050b5605, 0x06cc95a5, 0x059872f1, 0x0ca5535d,
  0x0392443d, 0x016b5aa1, 0x07de35c5, 0x01163411, 0x08f0fd65, 0x18bf947d, 0x14f6c531, 0x09aeaded,
  0x02f7b4d1, 0x095e3da5, 0x0cc0b1dd, 0x03df39cd, 0x004143dd, 0x057fc431, 0x005c6c75, 0x0246502d,
  0x0124711d, 0x0e1208e5, 0x0d134b75, 0x092616f5, 0x03116435, 0x12c13c0d, 0x13180079, 0x036e88d1,
  0x32b276d9, 0x05b39379, 0x0d22f951, 0x26602de1, 0x0ac8dc31, 0x02ae776d, 0x0b599e65, 0x16774769,
  0x02f381d5, 0x00f98efd, 0x05961269, 0x16b8a251, 0x05d1bc89, 0x1176bdf5, 0x0830659d, 0x020d62c9,
  0x02bc3a45, 0x00274dd5, 0x0e4a94ed, 0x00834419, 0x041531d5, 0x0768082d, 0x0c6f1a99, 0x011c845d,
  0x059dbe9d, 0x08d04751, 0x01587f71, 0x030f15ad, 0x1d52a54d, 0x1126baf1, 0x0f790169, 0x046216e5,
  0x03642f71, 0x17b91df5, 0x18b5bd0d, 0x06747e15, 0x011ecc85, 0x1e28c859, 0x06bfe8ad, 0x0ee96fe5,
  0x179dcf41, 0x0461b2d1, 0x02c14235, 0x0f16879d, 0x06a09fc1, 0x02f75915, 0x06d0ad7d, 0x014b5ad9,
  0x14a8e8e9, 0x0525b825, 0x00eb87a9, 0x0d193039, 0x16610519, 0x246acfcd, 0x02a97e05, 0x1af50241,
  0x02d48d85, 0x001dbc19, 0x01777b7d, 0x07b674b1, 0x02bfc991, 0x0451a135, 0x0b3eb851, 0x0001357d,
  0x29470c55, 0x1392970d, 0x1d264d59, 0x0029e90d, 0x03d8efe1, 0x155ae855, 0x01bc1481, 0x21a057e5,
  0x08e9a231, 0x0e657669, 0x043ea9c1, 0x09d4d185, 0x0155e661, 0x0a04e33d, 0x1872d439, 0x048854f9,
  0x235c8ed5, 0x008525f9, 0x166ca3bd, 0x03f67139, 0x093c59e1, 0x264ec44d, 0x01534f31, 0x07b12029,
  0x03adae45, 0x169d965d, 0x03704045, 0x0a7d1571, 0x16716d31, 0x016e1df9, 0x0f9dc39d, 0x01b01cc1,
  0x052dfae9, 0x0070a471, 0x099a1ba9, 0x18946a81, 0x177e0f99, 0x013ea7b1, 0x15699a0d, 0x1a2d6add,
  0x18d72f79, 0x13267b11, 0x0b6e5461, 0x012d5371, 0x04f4e709, 0x03fbe51d, 0x03589919, 0x0a385b25,
  0x0c166f0d, 0x11002dc9, 0x0580a08d, 0x18343319, 0x1084f211, 0x0162c1f1, 0x1c88a0d1, 0x15187c31,
  0x18f6478d, 0x04b68981, 0x003d7441, 0x019906a9, 0x2082bbfd, 0x02936f09, 0x0862bfdd, 0x0ec60b55,
};
#endif

