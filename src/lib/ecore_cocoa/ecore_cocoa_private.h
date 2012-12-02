#ifndef _ECORE_COCOA_PRIVATE_H
#define _ECORE_COCOA_PRIVATE_H

struct _Ecore_Cocoa_Window
{
   NSWindow *window;
   unsigned int borderless : 1;
};


#endif
