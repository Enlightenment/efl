#ifndef __ECORE_EVAS_COCOA_H__
#define __ECORE_EVAS_COCOA_H__

typedef struct _Ecore_Evas_Interface_Cocoa Ecore_Evas_Interface_Cocoa;

struct _Ecore_Evas_Interface_Cocoa
{
   Ecore_Evas_Interface base;

   Ecore_Cocoa_Window *(*window_get)(const Ecore_Evas *ee);
};

#endif /* ! __ECORE_EVAS_COCOA_H__ */

