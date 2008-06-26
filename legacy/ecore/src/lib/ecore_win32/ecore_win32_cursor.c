/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"


/***** API *****/

Ecore_Win32_Cursor *
ecore_win32_cursor_new(const void *pixels_and,
                       const void *pixels_xor,
                       int         width,
                       int         height,
                       int         hot_x,
                       int         hot_y)
{
   Ecore_Win32_Cursor *cursor = NULL;
   int                 cursor_width;
   int                 cursor_height;

   cursor_width = GetSystemMetrics(SM_CXCURSOR);
   cursor_height = GetSystemMetrics(SM_CYCURSOR);

   if ((cursor_width != width) ||
       (cursor_height != height))
     return NULL;

   if (!(cursor = CreateCursor(_ecore_win32_instance,
                               hot_x, hot_y,
                               width, height,
                               pixels_and,
                               pixels_xor)))
     return NULL;

   return cursor;
}

void
ecore_win32_cursor_free(Ecore_Win32_Cursor *cursor)
{
   DestroyCursor(cursor);
}

Ecore_Win32_Cursor *
ecore_win32_cursor_shape_get(Ecore_Win32_Cursor_Shape shape)
{
   Ecore_Win32_Cursor *cursor = NULL;
   const char         *cursor_name;

   switch (shape)
     {
       case ECORE_WIN32_CURSOR_SHAPE_APP_STARTING:
         cursor_name = IDC_APPSTARTING;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_ARROW:
         cursor_name = IDC_ARROW;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_CROSS:
         cursor_name = IDC_CROSS;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_HAND:
         cursor_name = IDC_HAND;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_HELP:
         cursor_name = IDC_HELP;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_I_BEAM:
         cursor_name = IDC_IBEAM;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_NO:
         cursor_name = IDC_NO;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_ALL:
         cursor_name = IDC_SIZEALL;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_NESW:
         cursor_name = IDC_SIZENESW;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_NS:
         cursor_name = IDC_SIZENS;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_NWSE:
         cursor_name = IDC_SIZENWSE;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_SIZE_WE:
         cursor_name = IDC_SIZEWE;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_UP_ARROW:
         cursor_name = IDC_UPARROW;
         break;
       case ECORE_WIN32_CURSOR_SHAPE_WAIT:
         cursor_name = IDC_WAIT;
         break;
     default:
         return NULL;
     }

   if (!(cursor = LoadCursor(NULL, cursor_name)))
     return NULL;

   return cursor;
}

int
ecore_win32_cursor_size_get(void)
{
   int width;
   int height;

   width = GetSystemMetrics(SM_CXCURSOR);
   height = GetSystemMetrics(SM_CYCURSOR);
   return (width > height) ? width : height;
}
