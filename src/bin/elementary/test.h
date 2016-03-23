#ifndef _TEST_H
#define _TEST_H

extern int _log_domain;

#define CRI(...)      EINA_LOG_DOM_CRIT(_log_domain, _VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)

#endif
