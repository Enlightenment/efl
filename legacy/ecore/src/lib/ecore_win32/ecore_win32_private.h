/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef __ECORE_WIN32_PRIVATE_H__
#define __ECORE_WIN32_PRIVATE_H__


#include <windows.h>
#ifdef HAVE_DIRECTDRAW
# include <ddraw.h>
#endif /* HAVE_DIRECTDRAW */
#ifdef HAVE_DIRECT3D
# include <d3d9.h>
# include <d3dx9.h>
#endif /* HAVE_DIRECT3D */

#include <Ecore_Data.h>

#include "Ecore_Win32.h"


#define ECORE_WIN32_WINDOW_CLASS "Ecore_Win32_Window_Class"


typedef struct _Ecore_Win32_Callback_Data Ecore_Win32_Callback_Data;

struct _Ecore_Win32_Callback_Data
{
   RECT         update;
   HWND         window;
   unsigned int message;
   WPARAM       window_param;
   LPARAM       data_param;
   long         time;
   int          x;
   int          y;
};

struct _Ecore_Win32_Window
{
   HWND                window;

#ifdef HAVE_DIRECTDRAW
   struct {
      LPDIRECTDRAW        object;
      LPDIRECTDRAWSURFACE surface_primary;
      LPDIRECTDRAWSURFACE surface_back;
      LPDIRECTDRAWCLIPPER clipper;
      int                 depth;
   } ddraw;
#endif /* HAVE_DIRECTDRAW */

#ifdef HAVE_DIRECT3D
   struct {
      LPDIRECT3D9        object;
      LPDIRECT3DDEVICE9  device;
      LPD3DXSPRITE       sprite;
      LPDIRECT3DTEXTURE9 texture;
      int                depth;
   } d3d;
#endif /* HAVE_DIRECT3D */

   DWORD               style;          /* used to go fullscreen to normal */
   RECT                rect;           /* used to go fullscreen to normal */

   unsigned int        min_width;
   unsigned int        min_height;
   unsigned int        max_width;
   unsigned int        max_height;
   unsigned int        base_width;
   unsigned int        base_height;
   unsigned int        step_width;
   unsigned int        step_height;

   unsigned int        pointer_is_in : 1;
   unsigned int        borderless    : 1;
   unsigned int        iconified     : 1;
   unsigned int        fullscreen    : 1;
};


extern Ecore_List         *_ecore_win32_windows_list;
extern HINSTANCE           _ecore_win32_instance;
extern double              _ecore_win32_double_click_time;
extern double              _ecore_win32_event_last_time;
extern Ecore_Win32_Window *_ecore_win32_event_last_window;


char *_ecore_win32_hwnd_str_get(HWND window);

void  _ecore_win32_event_handle_key_press(Ecore_Win32_Callback_Data *msg, int is_keystroke);
void  _ecore_win32_event_handle_key_release(Ecore_Win32_Callback_Data *msg, int is_keystroke);
void  _ecore_win32_event_handle_button_press(Ecore_Win32_Callback_Data *msg, int button);
void  _ecore_win32_event_handle_button_release(Ecore_Win32_Callback_Data *msg, int button);
void  _ecore_win32_event_handle_motion_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_enter_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_leave_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_focus_in(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_focus_out(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_expose(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_create_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_destroy_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_map_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_unmap_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_configure_notify(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_resize(Ecore_Win32_Callback_Data *msg);
void  _ecore_win32_event_handle_delete_request(Ecore_Win32_Callback_Data *msg);


#endif /* __ECORE_WIN32_PRIVATE_H__ */
