#ifndef __EFREETD_H
#define __EFREETD_H

#ifdef EFREETD_DEFAULT_LOG_COLOR
#undef EFREETD_DEFAULT_LOG_COLOR
#endif
#define EFREETD_DEFAULT_LOG_COLOR "\033[36m"

extern int efreetd_log_dom;

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(efreetd_log_dom, __VA_ARGS__)
#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(efreetd_log_dom, __VA_ARGS__)
#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(efreetd_log_dom, __VA_ARGS__)
#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(efreetd_log_dom, __VA_ARGS__)
#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(efreetd_log_dom, __VA_ARGS__)

void quit(void);

#endif
