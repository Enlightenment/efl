/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>

#include "ecore_win32_dnd_drop_source.h"

#include "ecore_win32_private.h"

// structors

// reference count must ALWAYS start at 1
DropSource::DropSource() : ref_count_(1)
{ }


// IUnknown

HRESULT DropSource::QueryInterface(REFIID iid, void **ppvObject)
{
   // check to see what interface has been requested
   if (iid == IID_IDropSource || iid == IID_IUnknown)
   {
      AddRef();
      *ppvObject = this;
      return S_OK;
   }
   *ppvObject = 0;
   return E_NOINTERFACE;
}

ULONG DropSource::AddRef()
{
   return InterlockedIncrement(&ref_count_);
}

ULONG DropSource::Release()
{
   LONG count = InterlockedDecrement(&ref_count_);
   if(count == 0)
   {
      delete this;
      return 0;
   }
   return count;
}


// IDropSource

HRESULT DropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    // if the Escape key has been pressed since the last call, cancel the drop
    if(fEscapePressed == TRUE)
        return DRAGDROP_S_CANCEL;

    // if the LeftMouse button has been released, then do the drop!
    if((grfKeyState & MK_LBUTTON) == 0)
        return DRAGDROP_S_DROP;

    // continue with the drag-drop
    return S_OK;
}

HRESULT DropSource::GiveFeedback(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}


// ecore_win32 private functions

void *_ecore_win32_dnd_drop_source_new()
{
   IDropSource *object = new DropSource();
   assert(object != NULL);
   return object;
}

void _ecore_win32_dnd_drop_source_free(void *drop_source)
{
   if (!drop_source)
     return;

   IDropSource *object = (IDropSource *)drop_source;
   object->Release();
}
