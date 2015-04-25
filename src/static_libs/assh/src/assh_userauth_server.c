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

#include <assh/assh_userauth_server.h>

#include <assh/assh_service.h>
#include <assh/assh_session.h>
#include <assh/assh_transport.h>
#include <assh/assh_event.h>
#include <assh/assh_packet.h>
#include <assh/assh_algo.h>
#include <assh/assh_sign.h>
#include <assh/assh_key.h>
#include <assh/assh_alloc.h>

#ifdef CONFIG_ASSH_SERVER

ASSH_EVENT_SIZE_SASSERT(userauth_server);

#ifdef CONFIG_ASSH_SERVER_AUTH_NONE
# warning CONFIG_ASSH_SERVER_AUTH_NONE is defined, server authentication is bypassed
# undef CONFIG_ASSH_SERVER_AUTH_PASSWORD
# undef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY
#endif

#include <stdlib.h>

enum assh_userauth_state_e
{
  ASSH_USERAUTH_WAIT_RQ,   //< intial state
#ifdef CONFIG_ASSH_SERVER_AUTH_PASSWORD
  ASSH_USERAUTH_PASSWORD,    //< the password event handler must check the user password
#endif
#ifdef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY
  ASSH_USERAUTH_PUBKEY_PKOK,   //< the public key event handler may send PK_OK
  ASSH_USERAUTH_PUBKEY_VERIFY , //< the public key event handler may check the signature
#endif
};

#ifdef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY
enum assh_userauth_pubkey_state_e
{
  ASSH_USERAUTH_PUBKEY_NONE,
  ASSH_USERAUTH_PUBKEY_NEW,
  ASSH_USERAUTH_PUBKEY_FOUND,
};
#endif

struct assh_userauth_context_s
{
  enum assh_userauth_state_e state;
  unsigned int retry;

  const struct assh_service_s *srv;
  char method_name[10];
  char username[CONFIG_ASSH_AUTH_USERNAME_LEN + 1];

#ifdef CONFIG_ASSH_SERVER_AUTH_PASSWORD
  char password[CONFIG_ASSH_AUTH_PASSWORD_LEN + 1];
#endif

#ifdef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY
  enum assh_userauth_pubkey_state_e pubkey_state;
  const struct assh_key_s *pub_key;
  struct assh_algo_sign_s *algo;
  struct assh_packet_s *sign_pck;
  uint8_t *sign;
#endif
};

static ASSH_SERVICE_INIT_FCN(assh_userauth_server_init)
{
  assh_error_t err;
  struct assh_userauth_context_s *pv;

  ASSH_ERR_RET(assh_alloc(s->ctx, sizeof(*pv),
                ASSH_ALLOC_SECUR, (void**)&pv));

  s->srv = &assh_service_userauth_server;
  s->srv_pv = pv;

  pv->state = ASSH_USERAUTH_WAIT_RQ;
  pv->retry = 10;
  pv->srv = NULL;

#ifdef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY
  pv->pub_key = NULL;
  pv->pubkey_state = ASSH_USERAUTH_PUBKEY_NONE;
  pv->sign_pck = NULL;  
#endif

  return ASSH_OK;
}

static void assh_userauth_server_flush_state(struct assh_session_s *s)
{
  struct assh_userauth_context_s *pv = s->srv_pv;

  pv->state = ASSH_USERAUTH_WAIT_RQ;
  pv->srv = NULL;

#ifdef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY
  assh_key_flush(s->ctx, &pv->pub_key);
  pv->pubkey_state = ASSH_USERAUTH_PUBKEY_NONE;

  assh_packet_release(pv->sign_pck);
  pv->sign_pck = NULL;  
#endif
}

static ASSH_SERVICE_CLEANUP_FCN(assh_userauth_server_cleanup)
{
  struct assh_userauth_context_s *pv = s->srv_pv;

  assh_userauth_server_flush_state(s);

  assh_free(s->ctx, pv);

  s->srv_pv = NULL;
  s->srv = NULL;
}

/* handle authentication failure */
static assh_error_t assh_userauth_server_failure(struct assh_session_s *s)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  /* flush state */
  assh_userauth_server_flush_state(s);

  /* check auth attempts count */
  ASSH_CHK_RET(pv->retry == 0, ASSH_ERR_NO_AUTH | ASSH_ERRSV_DISCONNECT);

  /* send the authentication failure packet */
  struct assh_packet_s *pout;
#if defined(CONFIG_ASSH_SERVER_AUTH_PUBLICKEY) && defined(CONFIG_ASSH_SERVER_AUTH_PASSWORD)
  const char *list_ = "publickey,password";
#elif defined(CONFIG_ASSH_SERVER_AUTH_PUBLICKEY)
  const char *list_ = "publickey";
#elif defined(CONFIG_ASSH_SERVER_AUTH_PASSWORD)
  const char *list_ = "password";
#else
  const char *list_ = "none";
#endif
  uint8_t *list, *partial_success;
  size_t list_len = strlen(list_);

  ASSH_ERR_RET(assh_packet_alloc(s->ctx, SSH_MSG_USERAUTH_FAILURE,
		 4 + list_len + 1, &pout) | ASSH_ERRSV_DISCONNECT);

  ASSH_ASSERT(assh_packet_add_string(pout, list_len, &list));
  memcpy(list, list_, list_len);
  ASSH_ASSERT(assh_packet_add_array(pout, 1, &partial_success));
  *partial_success = 0;
  assh_transport_push(s, pout);
  pv->retry--;

  return ASSH_OK;
}

/* handle authentication success */
static assh_error_t assh_userauth_server_success(struct assh_session_s *s)
{
  assh_error_t err;
  struct assh_userauth_context_s *pv = s->srv_pv;
  const struct assh_service_s *srv = pv->srv;

  /* cleanup the authentication service */
  assh_userauth_server_cleanup(s);

  /* send the authentication success packet */
  struct assh_packet_s *pout;
  ASSH_ERR_RET(assh_packet_alloc(s->ctx, SSH_MSG_USERAUTH_SUCCESS, 0, &pout)
	       | ASSH_ERRSV_DISCONNECT);
  assh_transport_push(s, pout);

  /* start the next requested service */
  ASSH_ERR_RET(srv->f_init(s) | ASSH_ERRSV_DISCONNECT);

  return ASSH_OK;
}

/******************************************************************* password */

#ifdef CONFIG_ASSH_SERVER_AUTH_PASSWORD

static ASSH_EVENT_DONE_FCN(assh_userauth_server_password_done)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  ASSH_CHK_RET(pv->state != ASSH_USERAUTH_PASSWORD, ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  if (e->userauth_server.password.success)
    ASSH_ERR_RET(assh_userauth_server_success(s) | ASSH_ERRSV_DISCONNECT);
  else
    ASSH_ERR_RET(assh_userauth_server_failure(s) | ASSH_ERRSV_DISCONNECT);

  return ASSH_OK;
}

/* handle password request packet */
static assh_error_t assh_userauth_server_req_password(struct assh_session_s *s,
                                                      struct assh_packet_s *p,
                                                      struct assh_event_s *e,
                                                      uint8_t *auth_data)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  uint8_t *second = auth_data;
  uint8_t *password;

  ASSH_ERR_RET(assh_packet_check_array(p, second, 1, &password) | ASSH_ERRSV_DISCONNECT);
  ASSH_CHK_RET(*second != 0, ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  /* copy password */
  ASSH_ERR_RET(assh_packet_check_string(p, password, NULL) | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_ssh_string_copy(password, pv->password, sizeof(pv->password))
	       | ASSH_ERRSV_DISCONNECT);

  /* return event to check the user password */
  e->id = ASSH_EVENT_USERAUTH_SERVER_PASSWORD;
  e->f_done = assh_userauth_server_password_done;
  e->userauth_server.password.username.str = pv->username;
  e->userauth_server.password.username.len = strlen(pv->username);
  e->userauth_server.password.password.str = pv->password;
  e->userauth_server.password.password.len = strlen(pv->password);
  e->userauth_server.password.success = 0;

  pv->state = ASSH_USERAUTH_PASSWORD;
  return ASSH_OK;
}

#endif

/******************************************************************* public key */

#ifdef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY

static assh_error_t assh_userauth_server_pubkey_check(struct assh_session_s *s,
                                                       struct assh_packet_s *p,
                                                       uint8_t *sign)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  uint8_t *end;
  ASSH_ERR_RET(assh_packet_check_string(p, sign, &end) | ASSH_ERRSV_DISCONNECT);

  uint8_t sid_len[4];   /* fake string header for session id */
  assh_store_u32(sid_len, s->session_id_len);

  /* buffers that have been signed by the client */
  const uint8_t *sign_ptrs[3] =
    { sid_len, s->session_id,     &p->head.msg };
  size_t sign_sizes[3]        =
    { 4,       s->session_id_len, sign - &p->head.msg };

  /* check the signature */
  if (assh_sign_check(s->ctx, pv->algo, pv->pub_key, 3,
        sign_ptrs, sign_sizes, sign + 4, end - sign - 4) == ASSH_OK)
    ASSH_ERR_RET(assh_userauth_server_success(s) | ASSH_ERRSV_DISCONNECT);
  else
    ASSH_ERR_RET(assh_userauth_server_failure(s) | ASSH_ERRSV_DISCONNECT);

  return ASSH_OK;
}

static ASSH_EVENT_DONE_FCN(assh_userauth_server_userkey_done)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  switch (pv->state)
    {
    case ASSH_USERAUTH_PUBKEY_PKOK: {      /* may need to send PK_OK */
      pv->state = ASSH_USERAUTH_WAIT_RQ;

      if (!e->userauth_server.userkey.found)
        {
          ASSH_ERR_RET(assh_userauth_server_failure(s) | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;
        }

      /* alloc packet */
      size_t algo_name_len = strlen(pv->algo->algo.name);

      size_t blob_len;
      ASSH_ERR_RET(assh_key_output(s->ctx, pv->pub_key,
                     NULL, &blob_len, ASSH_KEY_FMT_PUB_RFC4253_6_6) | ASSH_ERRSV_DISCONNECT);

      struct assh_packet_s *pout;
      ASSH_ERR_RET(assh_packet_alloc(s->ctx, SSH_MSG_USERAUTH_PK_OK,
                     4 + algo_name_len + 4 + blob_len, &pout) | ASSH_ERRSV_DISCONNECT);

      /* add sign algorithm name */
      uint8_t *algo_name;
      ASSH_ASSERT(assh_packet_add_string(pout, algo_name_len, &algo_name));
      memcpy(algo_name, pv->algo->algo.name, algo_name_len);

      /* add public key blob */
      uint8_t *blob;
      ASSH_ASSERT(assh_packet_add_string(pout, blob_len, &blob));
      ASSH_ERR_GTO(assh_key_output(s->ctx, pv->pub_key,
                     blob, &blob_len, ASSH_KEY_FMT_PUB_RFC4253_6_6)
		   | ASSH_ERRSV_DISCONNECT, err_packet);
      assh_packet_shrink_string(pout, blob, blob_len);

      assh_transport_push(s, pout);
      pv->pubkey_state = ASSH_USERAUTH_PUBKEY_FOUND;

      return ASSH_OK;
     err_packet:
      assh_packet_release(pout);
      return err;
    }

    case ASSH_USERAUTH_PUBKEY_VERIFY: {
      pv->state = ASSH_USERAUTH_WAIT_RQ;

      if (!e->userauth_server.userkey.found)
        ASSH_ERR_RET(assh_userauth_server_failure(s)
		     | ASSH_ERRSV_DISCONNECT);
      else
        ASSH_ERR_RET(assh_userauth_server_pubkey_check(s, pv->sign_pck, pv->sign)
		     | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
    }

    default:
      ASSH_ERR_RET(ASSH_ERR_STATE | ASSH_ERRSV_FATAL);
    }

  return ASSH_OK;
}

/* handle public key request packet */
static assh_error_t assh_userauth_server_req_pubkey(struct assh_session_s *s,
                                                    struct assh_packet_s *p,
                                                    struct assh_event_s *e,
                                                    uint8_t *auth_data)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  uint8_t *second = auth_data;
  uint8_t *algo_name, *pub_blob, *sign;

  ASSH_ERR_RET(assh_packet_check_array(p, second, 1, &algo_name) | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, algo_name, &pub_blob) | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, pub_blob, &sign) | ASSH_ERRSV_DISCONNECT);

  const struct assh_algo_s *algo;

  /* check if we support the requested signature algorithm */
  if (assh_algo_by_name(s->ctx, ASSH_ALGO_SIGN, (char*)algo_name + 4,
			pub_blob - algo_name - 4, &algo) != ASSH_OK)
    {
      ASSH_ERR_RET(assh_userauth_server_failure(s) | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
    }

  const struct assh_key_s *pub_key = NULL;

  /* load the public key from the client provided blob */
  ASSH_ERR_RET(assh_key_load(s->ctx, &pub_key, algo->key, ASSH_ALGO_SIGN,
                 ASSH_KEY_FMT_PUB_RFC4253_6_6, pub_blob + 4,
                 sign - pub_blob - 4) | ASSH_ERRSV_DISCONNECT);

  /* check if the key can be used by the algorithm */
  if (!assh_algo_suitable_key(s->ctx, algo, pub_key))
    {
      assh_key_drop(s->ctx, &pub_key);
      ASSH_ERR_RET(assh_userauth_server_failure(s) | ASSH_ERRSV_DISCONNECT);
      return ASSH_OK;
    }

  /* test if the key has been previously found in the list of authorized user keys. */
  assh_bool_t new_key = (pv->pubkey_state == ASSH_USERAUTH_PUBKEY_NONE ||
                         !assh_key_cmp(s->ctx, pub_key, pv->pub_key, 1));

  if (new_key)
    {
      assh_key_flush(s->ctx, &pv->pub_key);
      pv->pub_key = pub_key;
      pv->algo = (void*)algo;
      pv->pubkey_state = ASSH_USERAUTH_PUBKEY_NEW;
    }
  else
    {
      assh_key_drop(s->ctx, &pub_key);
    }

  /* the packet contains a signature to check */
  if (*second)
    {
      if (pv->pubkey_state == ASSH_USERAUTH_PUBKEY_FOUND)
        {
          ASSH_ERR_RET(assh_userauth_server_pubkey_check(s, p, sign)
		       | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;
        }

      assh_packet_refinc(p);
      pv->sign_pck = p;
      pv->sign = sign;

      pv->state = ASSH_USERAUTH_PUBKEY_VERIFY;
    }
  else
    {
      if (pv->pubkey_state == ASSH_USERAUTH_PUBKEY_FOUND)
        return ASSH_OK;

      pv->state = ASSH_USERAUTH_PUBKEY_PKOK;
    }

  /* return an event to lookup the key in the list of authorized user keys */
  e->id = ASSH_EVENT_USERAUTH_SERVER_USERKEY;
  e->f_done = assh_userauth_server_userkey_done;
  e->userauth_server.userkey.username.str = pv->username;
  e->userauth_server.userkey.username.len = strlen(pv->username);
  e->userauth_server.userkey.pub_key = pv->pub_key;
  e->userauth_server.userkey.found = 0;

  return ASSH_OK;
}
#endif

/********************************************************************/

/* flush the authentication state on new request */
static assh_error_t assh_userauth_server_req_new(struct assh_session_s *s,
                                                 uint8_t *srv_name,
                                                 uint8_t *username,
                                                 uint8_t *method_name)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  if (pv->srv == NULL ||
      !assh_ssh_string_compare(srv_name, pv->srv->name) ||
      !assh_ssh_string_compare(username, pv->username) ||
      !assh_ssh_string_compare(method_name, pv->method_name))
    {
      assh_userauth_server_flush_state(s);

      /* lookup service name */
      if (assh_service_by_name(s->ctx, assh_load_u32(srv_name),
                               (char*)srv_name + 4, &pv->srv))
        {
          ASSH_ERR_RET(assh_userauth_server_failure(s) | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;
        }

      /* keep method name and user name */
      ASSH_ERR_RET(assh_ssh_string_copy(username, pv->username, sizeof(pv->username))
		   | ASSH_ERRSV_DISCONNECT);
      ASSH_ERR_RET(assh_ssh_string_copy(method_name, pv->method_name, sizeof(pv->method_name))
		   | ASSH_ERRSV_DISCONNECT);
    }

  return ASSH_OK;
}

static ASSH_SERVICE_PROCESS_FCN(assh_userauth_server_process)
{
  struct assh_userauth_context_s *pv = s->srv_pv;
  assh_error_t err;

  if (s->tr_st >= ASSH_TR_FIN)
    return ASSH_OK;

  if (p == NULL)
    return ASSH_OK;

  ASSH_CHK_RET(pv->state != ASSH_USERAUTH_WAIT_RQ,
	       ASSH_ERR_STATE | ASSH_ERRSV_FATAL);

  ASSH_CHK_RET(p->head.msg != SSH_MSG_USERAUTH_REQUEST,
               ASSH_ERR_PROTOCOL | ASSH_ERRSV_DISCONNECT);

  uint8_t *username = p->head.end;
  uint8_t *srv_name, *method_name, *auth_data;

  ASSH_ERR_RET(assh_packet_check_string(p, username, &srv_name) | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, srv_name, &method_name) | ASSH_ERRSV_DISCONNECT);
  ASSH_ERR_RET(assh_packet_check_string(p, method_name, &auth_data) | ASSH_ERRSV_DISCONNECT);

  switch (/* method name len */ auth_data - method_name - 4)
    {
#ifdef CONFIG_ASSH_SERVER_AUTH_NONE
    case 4:
      if (!assh_ssh_string_compare(method_name, "none"))
        {
          ASSH_ERR_RET(assh_userauth_server_req_new(s, srv_name, username, method_name)
		       | ASSH_ERRSV_DISCONNECT);
          ASSH_ERR_RET(assh_userauth_server_success(s)
		       | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;
        }
      break;
#endif

#ifdef CONFIG_ASSH_SERVER_AUTH_PASSWORD
    case 8:
      if (!assh_ssh_string_compare(method_name, "password"))
        {
          ASSH_ERR_RET(assh_userauth_server_req_new(s, srv_name, username, method_name)
		       | ASSH_ERRSV_DISCONNECT);
          ASSH_ERR_RET(assh_userauth_server_req_password(s, p, e, auth_data)
		       | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;
        }
      break;
#endif

#ifdef CONFIG_ASSH_SERVER_AUTH_PUBLICKEY
    case 9:
      if (!assh_ssh_string_compare(method_name, "publickey"))
        {
          ASSH_ERR_RET(assh_userauth_server_req_new(s, srv_name, username, method_name)
		       | ASSH_ERRSV_DISCONNECT);
          ASSH_ERR_RET(assh_userauth_server_req_pubkey(s, p, e, auth_data)
		       | ASSH_ERRSV_DISCONNECT);
          return ASSH_OK;
        }
      break;
#endif
    }

  ASSH_ERR_RET(assh_userauth_server_failure(s) | ASSH_ERRSV_DISCONNECT);
  return ASSH_OK;
}

const struct assh_service_s assh_service_userauth_server =
{
  .name = "ssh-userauth",
  .side = ASSH_SERVER,
  .f_init = assh_userauth_server_init,
  .f_cleanup = assh_userauth_server_cleanup,
  .f_process = assh_userauth_server_process,
};

#endif

