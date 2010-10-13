#ifndef EEZE_PRIVATE_H
#define EEZE_PRIVATE_H
#ifndef EEZE_COLOR_DEFAULT
#define EEZE_COLOR_DEFAULT EINA_COLOR_CYAN
#endif
extern int _eeze_log_dom;
#ifdef ERR
#undef ERR
#endif
#ifdef INF
#undef INF
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DBG
#undef DBG
#endif

#define DBG(...)   EINA_LOG_DOM_DBG(_eeze_log_dom, __VA_ARGS__)
#define INFO(...)    EINA_LOG_DOM_INFO(_eeze_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_eeze_log_dom, __VA_ARGS__)
#define ERR(...)   EINA_LOG_DOM_ERR(_eeze_log_dom, __VA_ARGS__)

#endif
