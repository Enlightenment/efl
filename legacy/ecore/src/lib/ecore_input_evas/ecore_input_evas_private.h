#ifndef_ECORE_INPUT_PRIVATE_H
#define_ECORE_INPUT_PRIVATE_H

externint _ecore_input_evas_log_dom;

#ifdefECORE_INPUT_EVAS_DEFAULT_LOG_COLOR
#undef ECORE_INPUT_EVAS_DEFAULT_LOG_COLOR 
#endif

#defineECORE_INPUT_EVAS_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdefERR
#undef ERR
#endif
#defineERR(...) EINA_LOG_DOM_ERR(_ecore_input_evas_log_dom, __VA_ARGS__)

#ifdefDBG
#undef DBG
#endif
#defineDBG(...) EINA_LOG_DOM_DBG(_ecore_input_evas_log_dom, __VA_ARGS__)

#ifdefINF
#undef INF
#endif
#defineINF(...) EINA_LOG_DOM_INFO(_ecore_input_evas_log_dom, __VA_ARGS__)

#ifdefWRN
#undef WRN
#endif
#defineWRN(...) EINA_LOG_DOM_WARN(_ecore_input_evas_log_dom, __VA_ARGS__)

#ifdefCRIT
#undef CRIT
#endif
#defineCRIT(...) EINA_LOG_DOM_CRIT(_ecore_input_evas_log_dom, __VA_ARGS__)

#endif
