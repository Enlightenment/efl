#ifndef _EVAS_GL_PRIVATE_H
#define _EVAS_GL_PRIVATE_H
#include "evas_gl_common.h"

extern int _evas_engine_GL_common_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_GL_common_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_GL_common_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_GL_common_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_GL_common_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_common_log_dom, __VA_ARGS__)

extern Cutout_Rects *_evas_gl_common_cutout_rects;

#endif
