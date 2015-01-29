#ifndef _ECORE_COCOA_PRIVATE_H
#define _ECORE_COCOA_PRIVATE_H

#include "ecore_cocoa_window.h"

extern int _ecore_cocoa_log_domain;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_cocoa_log_domain, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ecore_cocoa_log_domain, __VA_ARGS__)


struct _Ecore_Cocoa_Window
{
   EcoreCocoaWindow *window;
   unsigned int borderless : 1;
};


#endif
