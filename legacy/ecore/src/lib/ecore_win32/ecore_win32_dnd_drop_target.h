#ifndef __ECORE_WIN32_DND_DROP_TARGET_H__
#define __ECORE_WIN32_DND_DROP_TARGET_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <ole2.h>

#include "Ecore_Win32.h"


class DropTarget : public IDropTarget
{
 private:

   LONG                                ref_count_;
   HWND                                window_;
   bool                                allow_drop_;
   Ecore_Win32_Dnd_DropTarget_Callback drop_callback_;
   void                               *drop_callback_ptr_;

 private: // internal helper function

   DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
   bool  QueryDataObject(IDataObject *pDataObject);

 public: // structors

   DropTarget(HWND hwnd, Ecore_Win32_Dnd_DropTarget_Callback callback, void *window_obj_ptr);

public: // IUnknown

   HRESULT __stdcall QueryInterface(REFIID iid, void ** ppvObject);
   ULONG   __stdcall AddRef();
   ULONG   __stdcall Release();

 public: // IDropTarget

   HRESULT __stdcall DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
   HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
   HRESULT __stdcall DragLeave();
   HRESULT __stdcall Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
};


#endif /* __ECORE_WIN32_DND_DROP_TARGET_H__ */
