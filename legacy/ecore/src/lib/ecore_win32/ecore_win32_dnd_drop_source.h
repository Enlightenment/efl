#ifndef __ECORE_WIN32_DND_DROP_SOURCE_H__
#define __ECORE_WIN32_DND_DROP_SOURCE_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <ole2.h>

#include "Ecore_Win32.h"


class DropSource : public IDropSource
{
 private:

   LONG ref_count_;

 public: // structors

   DropSource();

 public: // IUnknown

   HRESULT __stdcall QueryInterface(REFIID iid, void ** ppvObject);
   ULONG   __stdcall AddRef();
   ULONG   __stdcall Release();

 public: // IDropSource

   HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
   HRESULT __stdcall GiveFeedback(DWORD dwEffect);
};


#endif /* __ECORE_WIN32_DND_DROP_SOURCE_H__ */
