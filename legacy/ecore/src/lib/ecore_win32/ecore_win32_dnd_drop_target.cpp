/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_win32_dnd_drop_target.h"

#include "ecore_win32_private.h"


// structors

DropTarget::DropTarget(HWND window, Ecore_Win32_Dnd_DropTarget_Callback callback, void *window_obj_ptr)
  : ref_count_(1)
  , window_(window)
  , allow_drop_(false)
  , drop_callback_(callback)
  ,drop_callback_ptr_(window_obj_ptr)
{ }


// IUnknown

HRESULT DropTarget::QueryInterface(REFIID iid, void **ppvObject)
{
   // check to see what interface has been requested
   if (iid == IID_IDropTarget || iid == IID_IUnknown)
   {
      AddRef();
      *ppvObject = this;
      return S_OK;
   }
   *ppvObject = 0;

   return E_NOINTERFACE;
}

ULONG DropTarget::AddRef()
{
   return InterlockedIncrement(&ref_count_);
}

ULONG DropTarget::Release()
{
   LONG count = InterlockedDecrement(&ref_count_);
   if (count == 0)
   {
      delete this;
      return 0;
   }

   return count;
}


// IDropTarget

HRESULT DropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
   // does the dataobject contain data we want?
   allow_drop_ = QueryDataObject(pDataObject) &&
      (drop_callback_ == NULL ||
      (drop_callback_(drop_callback_ptr_, ECORE_WIN32_DND_EVENT_DRAG_ENTER, pt.x, pt.y, NULL, 0) != 0));

   if (allow_drop_)
   {
      // get the dropeffect based on keyboard state
      *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
      SetFocus(window_);
      //PositionCursor(_hwnd, pt);
   }
   else
      *pdwEffect = DROPEFFECT_NONE;
   return S_OK;
}

HRESULT DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
   allow_drop_ =
     (drop_callback_ == NULL) ||
     (drop_callback_(drop_callback_ptr_, ECORE_WIN32_DND_EVENT_DRAG_OVER, pt.x, pt.y, NULL, 0) != 0);

   if (allow_drop_)
   {
      *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
      //PositionCursor(m_hWnd, pt);
   }
   else
   {
      *pdwEffect = DROPEFFECT_NONE;
   }

   return S_OK;
}

HRESULT DropTarget::DragLeave()
{
   POINT pt;

   GetCursorPos(&pt);
   if (drop_callback_ != NULL)
     drop_callback_(drop_callback_ptr_, ECORE_WIN32_DND_EVENT_DRAG_LEAVE, pt.x, pt.y, NULL, 0);

   return S_OK;
}

HRESULT DropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
   if (allow_drop_)
   {
      // construct a FORMATETC object
      FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
      STGMEDIUM stgmed;

      // See if the dataobject contains any TEXT stored as a HGLOBAL
      if (pDataObject->QueryGetData(&fmtetc) == S_OK)
      {
         // Yippie! the data is there, so go get it!
         if (pDataObject->GetData(&fmtetc, &stgmed) == S_OK)
         {
            // we asked for the data as a HGLOBAL, so access it appropriately
            PVOID data = GlobalLock(stgmed.hGlobal);
            UINT size = GlobalSize(stgmed.hGlobal);

            if (drop_callback_ != NULL)
            {
               drop_callback_(drop_callback_ptr_,
                              ECORE_WIN32_DND_EVENT_DROP,
                              pt.x, pt.y,
                              data, size);
            }

            GlobalUnlock(stgmed.hGlobal);

            // release the data using the COM API
            ReleaseStgMedium(&stgmed);
         }
      }
      *pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
   }
   else
   {
      *pdwEffect = DROPEFFECT_NONE;
   }

   return S_OK;
}


// internal helper function

DWORD DropTarget::DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
{
   DWORD dwEffect = 0;

   // 1. check "pt" -> do we allow a drop at the specified coordinates?

   // 2. work out that the drop-effect should be based on grfKeyState
   if (grfKeyState & MK_CONTROL)
   {
	   dwEffect = dwAllowed & DROPEFFECT_COPY;
   }
   else if (grfKeyState & MK_SHIFT)
   {
	   dwEffect = dwAllowed & DROPEFFECT_MOVE;
   }

   // 3. no key-modifiers were specified (or drop effect not allowed), so
   //    base the effect on those allowed by the dropsource
   if (dwEffect == 0)
   {
	   if (dwAllowed & DROPEFFECT_COPY) dwEffect = DROPEFFECT_COPY;
	   if (dwAllowed & DROPEFFECT_MOVE) dwEffect = DROPEFFECT_MOVE;
   }

   return dwEffect;
}

bool DropTarget::QueryDataObject(IDataObject *pDataObject)
{
    FORMATETC fmtetc = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

    // does the data object support CF_TEXT using a HGLOBAL?
    return pDataObject->QueryGetData(&fmtetc) == S_OK;
}


// ecore_win32 private functions

void *_ecore_win32_dnd_register_drop_window(HWND hwnd,  Ecore_Win32_Dnd_DropTarget_Callback callback, void *ptr)
{
   DropTarget *pDropTarget = new DropTarget(hwnd, callback, ptr);

   if (pDropTarget == NULL)
     return NULL;

   // acquire a strong lock
   if (FAILED(CoLockObjectExternal(pDropTarget, TRUE, FALSE)))
     {
        delete pDropTarget;
        return NULL;
     }

   // tell OLE that the window is a drop target
   if (FAILED(RegisterDragDrop(hwnd, pDropTarget)))
     {
        delete pDropTarget;
        return NULL;
     }

   return pDropTarget;
}

void _ecore_win32_dnd_unregister_drop_window(HWND hwnd, void *drop_target)
{
   IDropTarget *pDropTarget = (IDropTarget *)drop_target;

   if (drop_target == NULL)
     return;

   // remove drag+drop
   RevokeDragDrop(hwnd);

   // remove the strong lock
   CoLockObjectExternal(pDropTarget, FALSE, TRUE);

   // release our own reference
   pDropTarget->Release();
}
