#ifndef _ELDBUS_MODEL_PRIVATE_H
#define _ELDBUS_MODEL_PRIVATE_H

#include "eldbus_private.h"

#include <Ecore.h>
#include <Eina.h>
#include <Eo.h>
#include <Efl.h>

typedef struct _Eldbus_Children_Slice_Promise _Eldbus_Children_Slice_Promise;
struct _Eldbus_Children_Slice_Promise
{
  unsigned start;
  unsigned count;
  Eina_Promise_Owner* promise;
};

typedef struct _Eldbus_Property_Promise _Eldbus_Property_Promise;
struct _Eldbus_Property_Promise
{
  char *property;
  Eina_Promise_Owner* promise;
};

/* logging support */
extern int eldbus_model_log_dom;

#define ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(exp, promise, err)       \
  do                                                                    \
    {                                                                   \
      if (EINA_UNLIKELY(!(exp)))                                        \
        {                                                               \
            eina_promise_owner_error_set(promise, err);                 \
            return;                                                     \
        }                                                               \
    }                                                                   \
  while(0)

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
