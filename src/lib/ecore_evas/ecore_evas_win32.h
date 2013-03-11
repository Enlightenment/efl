#ifndef ECORE_EVAS_WIN32_H
# define ECORE_EVAS_WIN32_H

typedef struct _Ecore_Evas_Interface_Win32 Ecore_Evas_Interface_Win32;

struct _Ecore_Evas_Interface_Win32 {
   Ecore_Evas_Interface base;

   Ecore_Win32_Window* (*window_get)(const Ecore_Evas *ee);
};

#endif
