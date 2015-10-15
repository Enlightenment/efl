#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */



/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EAPI Eina_Bool
ecore_win32_clipboard_set(const Ecore_Win32_Window *window,
                          const void *data,
                          int size)
{
   HGLOBAL global;
   char *d;

   /*
    * See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms649016%28v=vs.85%29.aspx#_win32_Copying_Information_to_the_Clipboard
    * 1. Open the clipboard
    * 2. Empty the clipboard
    * 3. Set the data
    * 4. Close the clipboard
    */

   INF("setting data to the clipboard");

   if (!window || !data || (size <= 0))
     return EINA_FALSE;

   if (!OpenClipboard(window->window))
     return EINA_FALSE;

   if (!EmptyClipboard())
     goto close_clipboard;

   global = GlobalAlloc(GMEM_MOVEABLE, size + 1);
   if (!global)
     goto close_clipboard;

   d = (char *)GlobalLock(global);
   if (!d)
     goto unlock_global;

   memcpy(d, data, size);
   d[size] = '\0';
   GlobalUnlock(global);
   SetClipboardData(CF_TEXT, global);
   CloseClipboard();

   return EINA_TRUE;

 unlock_global:
   GlobalUnlock(global);
 close_clipboard:
   CloseClipboard();

   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_win32_clipboard_get(const Ecore_Win32_Window *window,
                          void **data,
                          int *size)
{
   HGLOBAL global;
   void *d;
   void *p;

   /*
    * See https://msdn.microsoft.com/en-us/library/windows/desktop/ms649016%28v=vs.85%29.aspx#_win32_Pasting_Information_from_the_Clipboard
    * 1. Open Clipboard
    * 2. Determine format
    * 3. Retrieve data
    * 4. Manage data
    * 5. Close clipboard
    */

   INF("getting data from the clipboard");

   if (!window)
     return EINA_FALSE;

   if (!IsClipboardFormatAvailable(CF_TEXT))
     return EINA_FALSE;

   if (!OpenClipboard(window->window))
     goto set_val;

   /* { */
   /*   UINT fmt = 0; */

   /*   while (1) */
   /*     { */
   /*       fmt = EnumClipboardFormats(fmt); */
   /*       printf(" $ Format : %x\n", fmt); */
   /*       if (!fmt) */
   /*         break; */
   /*     } */
   /* } */

   global = GetClipboardData(CF_TEXT);
   if (!global)
     goto close_clipboard;

   d = GlobalLock(global);
   if (!d)
     goto unlock_global;

   *size = strlen(d);
   p = malloc(*size);
   if (!p)
     goto unlock_global;

   memcpy(p, d, *size);
   *data = p;
   GlobalUnlock(global);
   CloseClipboard();

   return EINA_TRUE;

 unlock_global:
   GlobalUnlock(global);
 close_clipboard:
   CloseClipboard();
 set_val:
   *data = NULL;
   *size = 0;

   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_win32_clipboard_clear(const Ecore_Win32_Window *window)
{
   INF("clearing the clipboard");

   if (!window)
     return EINA_FALSE;

   if (!OpenClipboard(window->window))
     return EINA_FALSE;

   if (!EmptyClipboard())
     goto close_clipboard;

   CloseClipboard();

   return EINA_TRUE;

 close_clipboard:
   CloseClipboard();

   return EINA_FALSE;
}

/**
 * @}
 */
