#ifndef _EFLAT_XML_MODEL_PRIVATE_H
#define _EFLAT_XML_MODEL_PRIVATE_H

#include "Eflat_Xml_Model.h"

#include <stdbool.h>

/* logging support */
extern int _eflat_xml_model_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_eflat_xml_model_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_eflat_xml_model_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_eflat_xml_model_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_eflat_xml_model_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_eflat_xml_model_log_dom, __VA_ARGS__)

typedef struct _Eflat_Xml_Model_Data Eflat_Xml_Model_Data;

/**
 * eflat_xml_model
 */
struct _Eflat_Xml_Model_Data
{
   Eo                        *obj;
   Efl_Model_Load             load;
   Eina_Array                *properties_array;
   Eina_Hash                 *properties_hash;
   Eina_Simple_XML_Node_Root *root;
   char                      *xml;
   Eina_Value                 xml_value;
};

#endif

