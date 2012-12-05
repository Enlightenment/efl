#ifndef _ECORE_EVAS_TYPES_H_
#define _ECORE_EVAS_TYPES_H_

#ifndef _ECORE_X_H
#define _ECORE_X_WINDOW_PREDEF
typedef unsigned int Ecore_X_Window;
#endif

#ifndef _ECORE_DIRECTFB_H
#define _ECORE_DIRECTFB_WINDOW_PREDEF
typedef struct _Ecore_DirectFB_Window Ecore_DirectFB_Window;
#endif

#ifndef __ECORE_WIN32_H__
typedef struct _Ecore_Win32_Window Ecore_Win32_Window;
#endif

#ifndef __ECORE_WINCE_H__
typedef struct _Ecore_WinCE_Window Ecore_WinCE_Window;
#endif

#ifndef __ECORE_COCOA_H__
typedef struct _Ecore_Cocoa_Window Ecore_Cocoa_Window;
#endif

#ifndef _ECORE_EVAS_PRIVATE_H
/* basic data types */
typedef struct _Ecore_Evas Ecore_Evas;
typedef void   (*Ecore_Evas_Event_Cb) (Ecore_Evas *ee); /**< Callback used for several ecore evas events @since 1.2 */
#endif

#ifndef _ECORE_WAYLAND_H_
#define _ECORE_WAYLAND_WINDOW_PREDEF
typedef struct _Ecore_Wl_Window Ecore_Wl_Window;
#endif


#endif /* _ECORE_EVAS_TYPES_H_ */
