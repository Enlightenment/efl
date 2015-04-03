#ifndef ECTOR_PRIVATE_H_
#define ECTOR_PRIVATE_H_

/*
 * variable and macros used for the eina_log module
 */
extern int _ector_log_dom_global;

/*
 * Macros that are used everywhere
 *
 * the first four macros are the general macros for the lib
 */
#ifdef ECTOR_DEFAULT_LOG_COLOR
# undef ECTOR_DEFAULT_LOG_COLOR
#endif /* ifdef ECTOR_DEFAULT_LOG_COLOR */
#define ECTOR_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...)  EINA_LOG_DOM_ERR(_ector_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...)  EINA_LOG_DOM_DBG(_ector_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...)  EINA_LOG_DOM_INFO(_ector_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...)  EINA_LOG_DOM_WARN(_ector_log_dom_global, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif /* ifdef CRI */
#define CRI(...) EINA_LOG_DOM_CRIT(_ector_log_dom_global, __VA_ARGS__)

#endif
