#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <windows.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */


static int _ecore_win32_dnd_init_count = 0;

static HANDLE DataToHandle(const char *data, int size)
{
   char *ptr;
   ptr = (char *)GlobalAlloc(GMEM_FIXED, size);
   memcpy(ptr, data, size);
   return ptr;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Ecore_Win32_Group Ecore_Win32 library
 *
 * @{
 */

/**
 * @brief Initialize the Ecore_Win32 Drag and Drop module.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function initialize the Drag and Drop module. It returns 0 on
 * failure, otherwise it returns the number of times it has already
 * been called.
 *
 * When the Drag and Drop module is not used anymore, call
 * ecore_win32_dnd_shutdown() to shut down the module.
 */
EAPI int
ecore_win32_dnd_init()
{
   HRESULT res;

   if (_ecore_win32_dnd_init_count > 0)
     {
        _ecore_win32_dnd_init_count++;
        return _ecore_win32_dnd_init_count;
     }

   res = OleInitialize(NULL);
   if ((res != S_OK) && (res != S_FALSE))
     {
        EINA_LOG_ERR("OleInitialize(NULL) returned %ld.", (long) res);
        return 0;
     }

   _ecore_win32_dnd_init_count++;

   return _ecore_win32_dnd_init_count;
}

/**
 * @brief Shut down the Ecore_Win32 Drag and Drop module.
 *
 * @return 0 when the module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Drag and Drop module. It returns 0 when it has
 * been called the same number of times than ecore_win32_dnd_init(). In that case
 * it shut down the module.
 */
EAPI int
ecore_win32_dnd_shutdown()
{
   _ecore_win32_dnd_init_count--;
   if (_ecore_win32_dnd_init_count > 0) return _ecore_win32_dnd_init_count;

   OleUninitialize();

   if (_ecore_win32_dnd_init_count < 0) _ecore_win32_dnd_init_count = 0;

   return _ecore_win32_dnd_init_count;
}

/**
 * @brief Begin a DnD operation.
 *
 * @param data The name pf the Drag operation.
 * @param size The size of the name.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function start a Drag operation with the name @p data. If
 * @p data is @c NULL, @c EINA_FALSE is returned. if @p size is less than
 * @c 0, it is set to the length (as strlen()) of @p data. On success the
 * function returns @c EINA_TRUE, otherwise it returns @c EINA_FALSE.
 */
EAPI Eina_Bool
ecore_win32_dnd_begin(const char *data,
                      int         size)
{
   IDataObject *pDataObject = NULL;
   IDropSource *pDropSource = NULL;
   FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
   STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };
   Eina_Bool res = EINA_FALSE;

   if (!data)
      return EINA_FALSE;

   if (size < 0)
      size = strlen(data) + 1;

   stgmed.hGlobal = DataToHandle(data, size);

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

      res = EINA_TRUE;
   }

   _ecore_win32_dnd_data_object_free(pDataObject);
   _ecore_win32_dnd_drop_source_free(pDropSource);

   // cleanup
   ReleaseStgMedium(&stgmed);

   return res;
}

/**
 * @brief Register a Drop operation.
 *
 * @param window The destination of the Drop operation.
 * @param callback The callback called when the Drop operation
 * finishes.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * This function register a Drop operation on @p window. Once the Drop
 * operation finishes, @p callback is called. If @p window is @c NULL,
 * the function returns @c EINA_FALSE. On success, it returns @c EINA_TRUE,
 * otherwise it returns @c EINA_FALSE.
 */
EAPI Eina_Bool
ecore_win32_dnd_register_drop_target(Ecore_Win32_Window                 *window,
                                     Ecore_Win32_Dnd_DropTarget_Callback callback)
{
   Ecore_Win32_Window *wnd = (Ecore_Win32_Window *)window;

   if (!window)
      return EINA_FALSE;

   wnd->dnd_drop_target = _ecore_win32_dnd_register_drop_window(wnd->window,
                                                                callback,
                                                                (void *)wnd);
   return wnd->dnd_drop_target ? EINA_TRUE : EINA_FALSE;
}

/**
 * @brief Unregister a Drop operation.
 *
 * @param window The destination of the Drop operation.
 *
 * This function unregister a Drop operation on @p window. If
 * @p window is @c NULL, the function does nothing.
 */
EAPI void
ecore_win32_dnd_unregister_drop_target(Ecore_Win32_Window *window)
{
   Ecore_Win32_Window *wnd = (Ecore_Win32_Window *)window;

   if (!window)
      return;

   if (wnd->dnd_drop_target)
      _ecore_win32_dnd_unregister_drop_window(wnd->window, wnd->dnd_drop_target);
}

/**
 * @}
 */
