#ifndef _ESOAP_MODEL_PRIVATE_H
#define _ESOAP_MODEL_PRIVATE_H

#include "Esoap_Model.h"

#include <Eflat_Xml_Model.h>

#include <stdbool.h>

/* logging support */
extern int _esoap_model_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_esoap_model_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_esoap_model_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_esoap_model_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_esoap_model_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_esoap_model_log_dom, __VA_ARGS__)

typedef struct _Esoap_Model_Data Esoap_Model_Data;

/**
 * esoap_model
 */
struct _Esoap_Model_Data
{
   Eo              *obj;
   Efl_Model_Load   load;
   Eina_List       *children_list;
   Eflat_Xml_Model *xml;
   char            *url;
   char            *action;
};

#endif

