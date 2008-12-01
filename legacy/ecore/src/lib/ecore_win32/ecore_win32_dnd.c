/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <windows.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"


static int _ecore_win32_dnd_init_count = 0;

static HANDLE DataToHandle(const char *data, int size)
{
   char *ptr;
   ptr = (char *)GlobalAlloc(GMEM_FIXED, size);
   memcpy(ptr, data, size);
   return ptr;
}


int
ecore_win32_dnd_init()
{
   if (_ecore_win32_dnd_init_count > 0)
     {
	_ecore_win32_dnd_init_count++;
	return _ecore_win32_dnd_init_count;
     }

   if (OleInitialize(NULL) != S_OK)
     return 0;

   _ecore_win32_dnd_init_count++;

   return _ecore_win32_dnd_init_count;
}

int ecore_win32_dnd_shutdown()
{
   _ecore_win32_dnd_init_count--;
   if (_ecore_win32_dnd_init_count > 0) return _ecore_win32_dnd_init_count;

   OleUninitialize();

   if (_ecore_win32_dnd_init_count < 0) _ecore_win32_dnd_init_count = 0;

   return _ecore_win32_dnd_init_count;
}

int ecore_win32_dnd_begin(const char *data,
                          int         size)
{
   if (data == NULL)
      return 0;
   if (size < 0)
      size = strlen(data) + 1;

   IDataObject *pDataObject = NULL;
   IDropSource *pDropSource = NULL;

   FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
   STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

   stgmed.hGlobal = DataToHandle(data, size);

   int res = 0;

   // create the data object
   pDataObject = (IDataObject *)_ecore_win32_dnd_data_object_new((void *)&fmtetc,
                                                                 (void *)&stgmed,
                                                                 1);
   pDropSource = (IDropSource *)_ecore_win32_dnd_drop_source_new();

   if (pDataObject && pDropSource)
   {
      DWORD dwResult;
      DWORD dwEffect = DROPEFFECT_COPY;

      // do the drag-drop!
      dwResult = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);

      // finished. Check the return values to see if we need to do anything else
      if (dwResult == DRAGDROP_S_DROP)
      {
         //printf(">>> \"%s\" Dropped <<<\n", str);
         if(dwEffect == DROPEFFECT_MOVE)
         {
            // remove the data we just dropped from active document
         }
      }
      //else if (dwResult == DRAGDROP_S_CANCEL)
      //   printf("DND cancelled\n");
      //else
      //   printf("DND error\n");

      res = 1;
   }

   _ecore_win32_dnd_data_object_free(pDataObject);
   _ecore_win32_dnd_drop_source_free(pDropSource);

   // cleanup
   ReleaseStgMedium(&stgmed);
   return (int)res;
}

int ecore_win32_dnd_register_drop_target(Ecore_Win32_Window                 *window,
                                         Ecore_Win32_Dnd_DropTarget_Callback callback)
{
   if (window == NULL)
      return 0;

   struct _Ecore_Win32_Window *wnd = (struct _Ecore_Win32_Window *)window;
   wnd->dnd_drop_target = _ecore_win32_dnd_register_drop_window(wnd->window,
                                                                callback,
                                                                (void *)wnd);
   return (int)(wnd->dnd_drop_target != NULL);
}

void ecore_win32_dnd_unregister_drop_target(Ecore_Win32_Window *window)
{
   if (window == NULL)
      return;

   struct _Ecore_Win32_Window *wnd = (struct _Ecore_Win32_Window *)window;
   if (wnd->dnd_drop_target != NULL)
      _ecore_win32_dnd_unregister_drop_window(wnd->window, wnd->dnd_drop_target);
}
