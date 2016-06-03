#ifndef _EJSON_PRIVATE_H
#define _EJSON_PRIVATE_H

#include <Eo.h>
#include <Efl.h>

/* logging support */
extern int _ejson_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_ejson_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_ejson_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_ejson_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_ejson_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_ejson_log_dom, __VA_ARGS__)

#endif
