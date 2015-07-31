#ifndef _ESOAP_MODEL_SUITE_H
#define _ESOAP_MODEL_SUITE_H

#include <check.h>

extern int _test_esoap_model_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_test_esoap_model_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_test_esoap_model_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_test_esoap_model_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_test_esoap_model_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_test_esoap_model_log_dom, __VA_ARGS__)

void esoap_model_test_esoap_model(TCase *tc);

#endif
