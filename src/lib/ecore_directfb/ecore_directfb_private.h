#ifndef _ECORE_DIRECTFB_PRIVATE_H
#define _ECORE_DIRECTFB_PRIVATE_H
/* eina_log related things */

extern int _ecore_directfb_log_dom;

#ifdef ECORE_DIRECTFB_DEFAULT_LOG_COLOR
#undef ECORE_DIRECTFB_DEFAULT_LOG_COLOR
#endif /* ifdef ECORE_DIRECTFB_DEFAULT_LOG_COLOR */
#define ECORE_DIRECTFB_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_directfb_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_directfb_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...) EINA_LOG_DOM_INFO(_ecore_directfb_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_directfb_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif /* ifdef CRIT */
#define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_directfb_log_dom, __VA_ARGS__)

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x ...)\
   {\
      _err = x;\
      if (_err != DFB_OK) {\
           CRIT("%s <%d>:\n\t", __FILE__, __LINE__ );\
           DirectFBErrorFatal( # x, _err );\
        }\
   }

struct keymap
{
   char *name;
   char *string;
};
#endif /* ifndef _ECORE_DIRECTFB_PRIVATE_H */
