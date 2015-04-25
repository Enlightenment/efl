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

#define ASSH_EV_CONST /* write access to event const fields */

#include <assh/assh_userauth_client.h>

#include <assh/assh_service.h>
#include <assh/assh_session.h>
#include <assh/assh_packet.h>
#include <assh/assh_transport.h>
#include <assh/assh_event.h>
#include <assh/assh_key.h>
#include <assh/assh_sign.h>
#include <assh/assh_alloc.h>

#ifdef CONFIG_ASSH_CLIENT

ASSH_EVENT_SIZE_SASSERT(userauth_client);

#include <stdlib.h>

enum assh_userauth_state_e
{
  ASSH_USERAUTH_INIT,
  ASSH_USERAUTH_GET_USERNAME,
  ASSH_USERAUTH_SENT_NONE_RQ,
#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
  ASSH_USERAUTH_SENT_PUB_KEY_RQ,
  ASSH_USERAUTH_SENT_PUB_KEY,
#endif
#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
  ASSH_USERAUTH_SENT_PASSWORD_RQ,
#endif
  ASSH_USERAUTH_GET_AUTHDATA,
};

struct assh_userauth_context_s
{
  enum assh_userauth_state_e state;  
  const struct assh_service_s *srv;
  char username[CONFIG_ASSH_AUTH_USERNAME_LEN];
  size_t username_len;

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
  const struct assh_algo_s *algo;
  uint_fast16_t algo_idx;
  const struct assh_key_s *pub_keys;
#endif
#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
  char password[CONFIG_ASSH_AUTH_PASSWORD_LEN];
  size_t password_len;
#endif
};

static ASSH_SERVICE_INIT_FCN(assh_userauth_client_init)
{
  struct assh_userauth_context_s *pv;
  assh_error_t err;

  ASSH_CHK_RET(s->srv_index >= s->ctx->srvs_count, ASSH_ERR_SERVICE_NA);

  ASSH_ERR_RET(assh_alloc(s->ctx, sizeof(*pv),
                    ASSH_ALLOC_SECUR, (void**)&pv));

  pv->state = ASSH_USERAUTH_INIT;

  s->srv = &assh_service_userauth_client;
  s->srv_pv = pv;

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
  pv->pub_keys = NULL;
#endif

#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
  pv->password_len = 0;
#endif

  /* get next client requested service */
  pv->srv = s->ctx->srvs[s->srv_index];

  return ASSH_OK;
}

static ASSH_SERVICE_CLEANUP_FCN(assh_userauth_client_cleanup)
{
  struct assh_userauth_context_s *pv = s->srv_pv;

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
  assh_key_flush(s->ctx, &pv->pub_keys);
#endif

  assh_free(s->ctx, pv);

  s->srv_pv = NULL;
  s->srv = NULL;
}

/* allocate a packet and append user name, service name and auth method name fields. */
static assh_error_t assh_userauth_client_pck_head(struct assh_session_s *s,
                                                  struct assh_packet_s **pout,
                                                  const char *method,
                                                  size_t extra_len)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  size_t srvname_len = strlen(pv->srv->name);
  size_t method_len = strlen(method);

  ASSH_ERR_RET(assh_packet_alloc(s->ctx, SSH_MSG_USERAUTH_REQUEST,
                 4 + pv->username_len + 4 + srvname_len +
                 4 + method_len + extra_len, pout) | ASSH_ERRSV_DISCONNECT);
  uint8_t *str;
  ASSH_ASSERT(assh_packet_add_string(*pout, pv->username_len, &str));
  memcpy(str, pv->username, pv->username_len);
  ASSH_ASSERT(assh_packet_add_string(*pout, srvname_len, &str));
  memcpy(str, pv->srv->name, srvname_len);
  ASSH_ASSERT(assh_packet_add_string(*pout, method_len, &str));
  memcpy(str, method, method_len);

  return ASSH_OK;
}

/******************************************************************* password */

#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
/* send a password authentication request */
static assh_error_t assh_userauth_client_req_password(struct assh_session_s *s)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  uint8_t *bool_, *str;

  struct assh_packet_s *pout;
  ASSH_ERR_RET(assh_userauth_client_pck_head(s, &pout, "password",
		   1 + 4 + pv->password_len) | ASSH_ERRSV_DISCONNECT);
  ASSH_ASSERT(assh_packet_add_array(pout, 1, &bool_));
  *bool_ = 0; // FALSE

  ASSH_ASSERT(assh_packet_add_string(pout, pv->password_len, &str));
  memcpy(str, pv->password, pv->password_len);
  assh_transport_push(s, pout);

  pv->state = ASSH_USERAUTH_SENT_PASSWORD_RQ;

  return ASSH_OK;
}
#endif

/******************************************************************* public key */

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
/* allocate a packet and append common fileds for a publickey request */
static assh_error_t assh_userauth_client_pck_pubkey(struct assh_session_s *s,
                                                    struct assh_packet_s **pout,
                                                    assh_bool_t second,
                                                    size_t extra_len)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  const struct assh_key_s *pub_key = pv->pub_keys;
  assh_error_t err;

  size_t algo_name_len = strlen(pv->algo->name);

  size_t blob_len;
  ASSH_ERR_RET(assh_key_output(s->ctx, pub_key,
           NULL, &blob_len, ASSH_KEY_FMT_PUB_RFC4253_6_6) | ASSH_ERRSV_DISCONNECT);

  ASSH_ERR_RET(assh_userauth_client_pck_head(s, pout, "publickey",
                 1 + 4 + algo_name_len + 4 + blob_len + extra_len) | ASSH_ERRSV_DISCONNECT);

  /* add boolean */
  uint8_t *str;
  ASSH_ASSERT(assh_packet_add_array(*pout, 1, &str));
  *str = second;

  /* add signature algorithm name */
  uint8_t *algo_name;
  ASSH_ASSERT(assh_packet_add_string(*pout, algo_name_len, &algo_name));
  memcpy(algo_name, pv->algo->name, algo_name_len);

  /* add public key blob */
  uint8_t *blob;
  ASSH_ASSERT(assh_packet_add_string(*pout, blob_len, &blob));
  ASSH_ERR_GTO(assh_key_output(s->ctx, pub_key, blob, &blob_len,
                 ASSH_KEY_FMT_PUB_RFC4253_6_6) | ASSH_ERRSV_DISCONNECT, err_packet);
  assh_packet_shrink_string(*pout, blob, blob_len);

  return ASSH_OK;

 err_packet:
  assh_packet_release(*pout);
  return err;
}
#endif

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
/* send a public key authentication request with signature */
static assh_error_t assh_userauth_client_req_pubkey_sign(struct assh_session_s *s)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  const struct assh_algo_sign_s *algo = (const void *)pv->algo;
  assh_error_t err;

  size_t sign_len;
  ASSH_ERR_RET(assh_sign_generate(s->ctx, algo, pv->pub_keys, 0,
		NULL, NULL, NULL, &sign_len) | ASSH_ERRSV_DISCONNECT);

  struct assh_packet_s *pout;
  ASSH_ERR_RET(assh_userauth_client_pck_pubkey(s, &pout,
                1, 4 + sign_len) | ASSH_ERRSV_DISCONNECT);

  uint8_t sid_len[4];   /* fake string header for session id */
  assh_store_u32(sid_len, s->session_id_len);

  /* buffers that must be signed by the client */
  const uint8_t *sign_ptrs[3] =
    { sid_len, s->session_id,     &pout->head.msg };
  size_t sign_sizes[3]        =
    { 4,       s->session_id_len, pout->data_size - 5 };

  /* append the signature */
  uint8_t *sign;
  ASSH_ASSERT(assh_packet_add_string(pout, sign_len, &sign));
  ASSH_ERR_GTO(assh_sign_generate(s->ctx, algo, pv->pub_keys,
                 3, sign_ptrs, sign_sizes, sign, &sign_len)
	       | ASSH_ERRSV_DISCONNECT, err_packet);
  assh_packet_shrink_string(pout, sign, sign_len);

  assh_transport_push(s, pout);

  pv->state = ASSH_USERAUTH_SENT_PUB_KEY_RQ;
  return ASSH_OK;

 err_packet:
  assh_packet_release(pout);
  return err;
}

/* send a public key authentication probing request */
static assh_error_t assh_userauth_client_req_pubkey(struct assh_session_s *s)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

#ifdef CONFIG_ASSH_CLIENT_AUTH_USE_PKOK /* send a public key lookup first */
  struct assh_packet_s *pout;
  ASSH_ERR_RET(assh_userauth_client_pck_pubkey(s, &pout,
                 0, 0) | ASSH_ERRSV_DISCONNECT);
  assh_transport_push(s, pout);
  pv->state = ASSH_USERAUTH_SENT_PUB_KEY;
#else  /* compute and send the signature directly */
  ASSH_ERR_RET(assh_userauth_client_req_pubkey_sign(s) | ASSH_ERRSV_DISCONNECT);
#endif
  return ASSH_OK;
}
#endif

/********************************************************************/

static ASSH_EVENT_DONE_FCN(assh_userauth_client_username_done)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  struct assh_packet_s *pout;
  ASSH_CHK_RET(pv->state != ASSH_USERAUTH_GET_USERNAME,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  /* keep username */
  size_t ulen = e->userauth_client.user.username.len;
  ASSH_CHK_RET(ulen > sizeof(pv->username),
	       ASSH_ERR_OUTPUT_OVERFLOW | ASSH_ERRSV_DISCONNECT);
  memcpy(pv->username, e->userauth_client.user.username.str,
	 pv->username_len = ulen);

  /* send auth request with the "none" method */
  ASSH_ERR_RET(assh_userauth_client_pck_head(s, &pout, "none", 0)
	       | ASSH_ERRSV_DISCONNECT);
  assh_transport_push(s, pout);

  pv->state = ASSH_USERAUTH_SENT_NONE_RQ;

  return ASSH_OK;
}

static ASSH_EVENT_DONE_FCN(assh_userauth_client_methods_done)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  ASSH_CHK_RET(pv->state != ASSH_USERAUTH_GET_AUTHDATA,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
  const char *password = e->userauth_client.methods.password.str;
  if (password != NULL)
    {
      size_t password_len = e->userauth_client.methods.password.len;
      ASSH_CHK_RET(password_len > sizeof(pv->password),
		   ASSH_ERR_OUTPUT_OVERFLOW | ASSH_ERRSV_DISCONNECT);
      memcpy(pv->password, password, pv->password_len = password_len);
    }
#endif

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
  const struct assh_key_s *k = e->userauth_client.methods.pub_keys;

  while (k != NULL)
    {
      /* check usable keys */
      pv->algo_idx = 0;
      if (assh_algo_by_key(s->ctx, k, &pv->algo_idx, &pv->algo) == ASSH_OK)
        {
          /* insert provided keys in internal list */
          const struct assh_key_s *next = k->next;
          assh_key_insert(&pv->pub_keys, k);
          k = next;
        }
      else
        assh_key_drop(s->ctx, &k);
    }

  if (pv->pub_keys != NULL)
    {
      ASSH_ERR_RET(assh_userauth_client_req_pubkey(s) | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
    }
#endif

#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
  if (pv->password_len != 0)
    {
      ASSH_ERR_RET(assh_userauth_client_req_password(s) | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
    }
#endif

   ASSH_ERR_RET(ASSH_ERR_NO_AUTH | ASSH_ERRSV_DISCONNECT);
   return ASSH_OK;
}

/* cleanup the authentication service and start the next service. */
static assh_error_t assh_userauth_client_success(struct assh_session_s *s)
{
  assh_error_t err;
  struct assh_userauth_context_s *pv = s->srv_pv;
  const struct assh_service_s *srv = pv->srv;

  assh_userauth_client_cleanup(s);

  ASSH_ERR_RET(srv->f_init(s) | ASSH_ERRSV_DISCONNECT);
  s->srv_index++;

  return ASSH_OK;
}

/* extract the list of acceptable authentication methods from a failure packet */
static assh_error_t assh_userauth_client_failure(struct assh_session_s *s,
                                                 struct assh_packet_s *p,
                                                 struct assh_event_s *e)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  switch (pv->state)
    {
    case ASSH_USERAUTH_SENT_PUB_KEY_RQ:
    case ASSH_USERAUTH_SENT_PUB_KEY:
      /* try next algorithm usable with the same key */
      pv->algo_idx++;
      while (pv->pub_keys != NULL &&
             assh_algo_by_key(s->ctx, pv->pub_keys,
                              &pv->algo_idx, &pv->algo) != ASSH_OK)
        {
          /* drop used key */
          assh_key_drop(s->ctx, &pv->pub_keys);
          pv->algo_idx = 0;
        }
      break;
    case ASSH_USERAUTH_SENT_PASSWORD_RQ:
      /* drop used password */
      pv->password_len = 0;
      break;
    default:
      break;
    }

  uint8_t *methods = p->head.end;
  uint8_t *partial_success, *n;
  ASSH_ERR_RET(assh_packet_check_string(p, methods, &partial_success)
	       | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_array(p, partial_success, 1, NULL)
	       | ASSH_ERRSV_DISCONNECT);

#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
  e->userauth_client.methods.use_password = 0;
#endif
#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
  e->userauth_client.methods.use_pub_key = 0;
#endif
  int count = 0;

  for (methods += 4; methods < partial_success; methods = n + 1)
    {
      n = methods;
      while (*n != ',' && n < partial_success)
        n++;

      switch (n - methods)
        {
#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
        case 8:
          if (!strncmp((const char*)methods, "password", 8))
            {
              if (pv->password_len != 0)
                {
                  /* a password string is already available */
                  ASSH_ERR_RET(assh_userauth_client_req_password(s)
			       | ASSH_ERRSV_DISCONNECT);
                  return ASSH_OK;
                }
              e->userauth_client.methods.use_password = 1;
              count++;
            }
          break;
#endif

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
        case 9:
          if (!strncmp((const char*)methods, "publickey", 9))
            {
              if (pv->pub_keys != NULL)
                {
                  /* some user keys are already available */
                  ASSH_ERR_RET(assh_userauth_client_req_pubkey(s)
			       | ASSH_ERRSV_DISCONNECT);
                  return ASSH_OK;
                }

              e->userauth_client.methods.use_pub_key = 1;
              count++;
            }
#endif

        default:
          break;
        }
    }

  ASSH_CHK_RET(count == 0, ASSH_ERR_NO_AUTH | ASSH_ERRSV_DISCONNECT);

#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
  e->userauth_client.methods.password.str = NULL;
  e->userauth_client.methods.password.len = 0;
#endif
#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
  e->userauth_client.methods.pub_keys = NULL;
#endif
  e->id = ASSH_EVENT_USERAUTH_CLIENT_METHODS;
  e->f_done = &assh_userauth_client_methods_done;

  pv->state = ASSH_USERAUTH_GET_AUTHDATA;

  return ASSH_OK;
}

static ASSH_SERVICE_PROCESS_FCN(assh_userauth_client_process)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  if (s->tr_st >= ASSH_TR_FIN)
    return ASSH_OK;

  switch (pv->state)
    {
    case ASSH_USERAUTH_INIT:
      ASSH_CHK_RET(p != NULL, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
      e->id = ASSH_EVENT_USERAUTH_CLIENT_USER;
      e->f_done = &assh_userauth_client_username_done;
      e->userauth_client.user.username.str = NULL;
      e->userauth_client.user.username.len = 0;
      pv->state = ASSH_USERAUTH_GET_USERNAME;
      return ASSH_OK;

    case ASSH_USERAUTH_SENT_NONE_RQ:
#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
    case ASSH_USERAUTH_SENT_PASSWORD_RQ:
#endif
#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
    case ASSH_USERAUTH_SENT_PUB_KEY_RQ:
#endif
      if (p == NULL)
        return ASSH_OK;

      switch (p->head.msg)
        {
        case SSH_MSG_USERAUTH_SUCCESS:
          ASSH_ERR_RET(assh_userauth_client_success(s) | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;

        case SSH_MSG_USERAUTH_PASSWD_CHANGEREQ:
#ifdef CONFIG_ASSH_CLIENT_AUTH_PASSWORD
          ASSH_CHK_RET(pv->state != ASSH_USERAUTH_SENT_PASSWORD_RQ,
		       ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
#endif
        case SSH_MSG_USERAUTH_FAILURE:
          ASSH_ERR_RET(assh_userauth_client_failure(s, p, e) | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;

        default:
          ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
        }

#ifdef CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY
    case ASSH_USERAUTH_SENT_PUB_KEY:
      if (p == NULL)
        return ASSH_OK;

      switch(p->head.msg)
        {
        case SSH_MSG_USERAUTH_PK_OK:
          ASSH_ERR_RET(assh_userauth_client_req_pubkey_sign(s) | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;

        case SSH_MSG_USERAUTH_FAILURE:
          ASSH_ERR_RET(assh_userauth_client_failure(s, p, e) | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;

        default:
          ASSH_ERR_RET(ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);
        }
#endif

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  return ASSH_OK;
}

const struct assh_service_s assh_service_userauth_client =
{
  .name = "ssh-userauth",
  .side = ASSH_CLIENT,
  .f_init = assh_userauth_client_init,
  .f_cleanup = assh_userauth_client_cleanup,
  .f_process = assh_userauth_client_process,
};

#endif

