#ifndef _ELDBUS_MODEL_PRIVATE_H
#define _ELDBUS_MODEL_PRIVATE_H

#include "eldbus_private.h"

#include <Ecore.h>
#include <Eina.h>
#include <Eo.h>
#include <Efl.h>

#include "ecore_internal.h"

typedef struct _Eldbus_Children_Slice_Promise Eldbus_Children_Slice_Promise;
typedef struct _Eldbus_Model_Data Eldbus_Model_Data;

struct _Eldbus_Children_Slice_Promise
{
   Eina_Promise *p;

   unsigned int start;
   unsigned int count;
};

struct _Eldbus_Model_Data
{
   Eldbus_Connection *connection;
   Eldbus_Connection_Type type;

   Eina_Stringshare *address;
   Eina_Stringshare *unique_name;

   Eina_Bool private : 1;

   Eina_Bool is_listed : 1;
};

#define UNIQUE_NAME_PROPERTY "unique_name"

/* logging support */
extern int eldbus_model_log_dom;

#define ELDBUS_MODEL_ON_ERROR_EXIT_PROMISE_SET(exp, promise, err, v)    \
  do                                                                    \
    {                                                                   \
      if (EINA_UNLIKELY(!(exp)))                                        \
        {                                                               \
            efl_promise_failed_set(promise, err);                       \
            return v;                                                   \
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
