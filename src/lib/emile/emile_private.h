#ifndef EMILE_PRIVATE_H_
# define EMILE_PRIVATE_H_

extern int _emile_log_dom_global;

#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...)  EINA_LOG_DOM_ERR(_emile_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...)  EINA_LOG_DOM_DBG(_emile_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...)  EINA_LOG_DOM_INFO(_emile_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...)  EINA_LOG_DOM_WARN(_emile_log_dom_global, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif /* ifdef CRI */
#define CRI(...) EINA_LOG_DOM_CRIT(_emile_log_dom_global, __VA_ARGS__)

typedef enum
{
  EMILE_SSL_STATE_INIT = 0,
  EMILE_SSL_STATE_HANDSHAKING,
  EMILE_SSL_STATE_DONE,
  EMILE_SSL_STATE_ERROR
} Emile_SSL_State;

Eina_Bool _emile_cipher_init(void);

#endif /* EMILE_PRIVATE_H_ */
