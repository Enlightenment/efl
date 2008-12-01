/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <ole2.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"

#include "ecore_win32_dnd_enumformatetc.h"
#include "ecore_win32_dnd_data_object.h"


static HGLOBAL DupGlobalMem(HGLOBAL hMem)
{
   DWORD len = (DWORD)GlobalSize(hMem);
   PVOID source = GlobalLock(hMem);
   PVOID dest = GlobalAlloc(GMEM_FIXED, len);
   memcpy(dest, source, len);
   GlobalUnlock(hMem);
   return dest;
}

// structors

DataObject::DataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, int count)
{
   assert(fmtetc != NULL);
   assert(stgmed != NULL);
   assert(count > 0);

   // reference count must ALWAYS start at 1
   ref_count_ = 1;
   formats_num_ = count;

   format_etc_ = new FORMATETC[count];
   stg_medium_ = new STGMEDIUM[count];

   for(int i = 0; i < count; i++)
   {
      format_etc_[i] = fmtetc[i];
      stg_medium_[i] = stgmed[i];
   }
}

DataObject::~DataObject()
{
   delete[] format_etc_;
   delete[] stg_medium_;
}


// IUnknown

HRESULT DataObject::QueryInterface(REFIID iid, void **ppvObject)
{
   // check to see what interface has been requested
   if ((iid == IID_IDataObject) || (iid == IID_IUnknown))
   {
      AddRef();
      *ppvObject = this;
      return S_OK;
   }
   *ppvObject = 0;
   return E_NOINTERFACE;
}

ULONG DataObject::AddRef()
{
   return InterlockedIncrement(&ref_count_);
}

ULONG DataObject::Release()
{
   LONG count = InterlockedDecrement(&ref_count_);
   if(count == 0)
   {
      delete this;
      return 0;
   }
   return count;
}

// IDataObject

HRESULT DataObject::GetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium)
{
   assert(pMedium != NULL);
   int idx;

   // try to match the specified FORMATETC with one of our supported formats
   if((idx = lookup_format_etc(pFormatEtc)) == -1)
      return DV_E_FORMATETC;

   // found a match - transfer data into supplied storage medium
   pMedium->tymed = format_etc_[idx].tymed;
   pMedium->pUnkForRelease = 0;

   // copy the data into the caller's storage medium
   switch(format_etc_[idx].tymed)
   {
   case TYMED_HGLOBAL:
      pMedium->hGlobal = DupGlobalMem(stg_medium_[idx].hGlobal);
      break;

   default:
      return DV_E_FORMATETC;
   }

   return S_OK;
}

HRESULT DataObject::GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pmedium)
{
   return DATA_E_FORMATETC;
}

HRESULT DataObject::QueryGetData(FORMATETC *pFormatEtc)
{
   return (lookup_format_etc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
}

HRESULT DataObject::GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut)
{
   // Apparently we have to set this field to NULL even though we don't do anything else
   pFormatEtcOut->ptd = NULL;
   return E_NOTIMPL;
}

HRESULT DataObject::SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease)
{
   return E_NOTIMPL;
}

HRESULT DataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc)
{
   // only the get direction is supported for OLE
   if(dwDirection == DATADIR_GET)
   {
      // for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
      // to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
      return CreateEnumFormatEtc(formats_num_, format_etc_, ppEnumFormatEtc);
   }
   else
   {
      // the direction specified is not supported for drag+drop
      return E_NOTIMPL;
   }
}

HRESULT DataObject::DAdvise(FORMATETC *pFormatEtc,  DWORD advf, IAdviseSink *, DWORD *)
{
   return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT DataObject::DUnadvise(DWORD dwConnection)
{
   return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT DataObject::EnumDAdvise(IEnumSTATDATA **ppEnumAdvise)
{
   return OLE_E_ADVISENOTSUPPORTED;
}

// internal helper function

int DataObject::lookup_format_etc(FORMATETC *pFormatEtc)
{
   // check each of our formats in turn to see if one matches
   for(int i = 0; i < formats_num_; i++)
   {
      if((format_etc_[i].tymed & pFormatEtc->tymed) &&
         (format_etc_[i].cfFormat == pFormatEtc->cfFormat) &&
         (format_etc_[i].dwAspect == pFormatEtc->dwAspect))
      {
         // return index of stored format
         return i;
      }
   }

   // error, format not found
   return -1;
}

void *_ecore_win32_dnd_data_object_new(void *fmtetc, void *stgmeds, int count)
{
   IDataObject *object = new DataObject((FORMATETC *)fmtetc, (STGMEDIUM *)stgmeds, (UINT)count);
   assert(object != NULL);
   return object;
}

void _ecore_win32_dnd_data_object_free(void *data_object)
{
   if (!data_object)
     return;

   IDataObject *object = (IDataObject *)data_object;
   object->Release();
}
