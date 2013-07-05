#ifndef _ECORE_EVAS_TYPES_H_
#define _ECORE_EVAS_TYPES_H_

#ifdef _WIN32
# include <stddef.h>
#else
# include <inttypes.h>
#endif

#ifndef _ECORE_X_H
#define _ECORE_X_WINDOW_PREDEF
typedef unsigned int Ecore_X_Window;
typedef unsigned int Ecore_X_Pixmap;
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

#ifndef _ECORE_GETOPT_PREDEF
typedef struct _Ecore_Getopt Ecore_Getopt;
#define _ECORE_GETOPT_PREDEF 1
#endif
#ifndef _ECORE_GETOPT_DESC_PREDEF
typedef struct _Ecore_Getopt_Desc Ecore_Getopt_Desc;
#define _ECORE_GETOPT_DESC_PREDEF 1
#endif
#ifndef _ECORE_GETOPT_VALUE_PREDEF
typedef union _Ecore_Getopt_Value Ecore_Getopt_Value;
#define _ECORE_GETOPT_VALUE_PREDEF 1
#endif

#ifndef _ECORE_WINDOW_PREDEF
typedef uintptr_t Ecore_Window;
#define _ECORE_WINDOW_PREDEF 1
#endif

#endif /* _ECORE_EVAS_TYPES_H_ */
