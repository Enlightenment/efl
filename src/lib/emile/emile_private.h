#ifndef EMILE_PRIVATE_H_
# define EMILE_PRIVATE_H_

extern int _emile_log_dom_global;

#ifdef ERR
# undef ERR
#endif
#define ERR(...)  EINA_LOG_DOM_ERR(_emile_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif
#define DBG(...)  EINA_LOG_DOM_DBG(_emile_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...)  EINA_LOG_DOM_INFO(_emile_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif
#define WRN(...)  EINA_LOG_DOM_WARN(_emile_log_dom_global, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_emile_log_dom_global, __VA_ARGS__)

typedef enum
{
  EMILE_SSL_STATE_INIT = 0,
  EMILE_SSL_STATE_HANDSHAKING,
  EMILE_SSL_STATE_DONE,
  EMILE_SSL_STATE_ERROR
} Emile_SSL_State;

Eina_Bool _emile_cipher_init(void);

Eina_Bool
emile_pbkdf2_sha1(const char *key,
                  unsigned int key_len,
                  const unsigned char *salt,
                  unsigned int salt_len,
                  unsigned int iter,
                  unsigned char *res,
                  unsigned int res_len);

#endif
