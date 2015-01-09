#ifndef _ELDBUS_MODEL_PRIVATE_H
#define _ELDBUS_MODEL_PRIVATE_H

#include "eldbus_private.h"

#include <Eo.h>
#include <Efl.h>

/* logging support */
extern int eldbus_model_log_dom;

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(eldbus_model_log_dom, __VA_ARGS__)

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(eldbus_model_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(eldbus_model_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(eldbus_model_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(eldbus_model_log_dom, __VA_ARGS__)

#endif
