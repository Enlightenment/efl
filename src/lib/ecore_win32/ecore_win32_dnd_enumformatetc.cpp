
#include <ole2.h>

#include "ecore_win32_dnd_enumformatetc.h"


// structors

CEnumFormatEtc::CEnumFormatEtc(FORMATETC *format_etc, int formats_num)
  : ref_count_(1)
  , index_(0)
  , formats_num_(formats_num)
  , format_etc_(new FORMATETC[formats_num])
{
   // make a new copy of each FORMATETC structure
   for (unsigned int i = 0; i < formats_num_; i++)
     {
        DeepCopyFormatEtc(&format_etc_[i], &format_etc[i]);
     }
}

CEnumFormatEtc::~CEnumFormatEtc()
{
   if (format_etc_)
     {
        // first free any DVTARGETDEVICE structures
        for (ULONG i = 0; i < formats_num_; i++)
          {
             if (format_etc_[i].ptd)
               CoTaskMemFree(format_etc_[i].ptd);
          }

        // now free the main array
        delete[] format_etc_;
     }
}

// IUnknown

ULONG __stdcall CEnumFormatEtc::AddRef(void)
{
   // increment object reference count
   return InterlockedIncrement(&ref_count_);
}

ULONG __stdcall CEnumFormatEtc::Release(void)
{
   // decrement object reference count
   LONG count = InterlockedDecrement(&ref_count_);

   if (count == 0)
     {
        delete this;
        return 0;
     }
   else
     {
        return count;
     }
}

HRESULT __stdcall CEnumFormatEtc::QueryInterface(REFIID iid, void **ppvObject)
{
   // check to see what interface has been requested
   if ((iid == IID_IEnumFORMATETC) || (iid == IID_IUnknown))
     {
        AddRef();
        *ppvObject = this;
        return S_OK;
     }
   else
     {
        *ppvObject = 0;
        return E_NOINTERFACE;
     }
}

// IEnumFormatEtc

HRESULT CEnumFormatEtc::Reset(void)
{
   index_ = 0;
   return S_OK;
}

HRESULT CEnumFormatEtc::Skip(ULONG celt)
{
   index_ += celt;
   return (index_ <= formats_num_) ? S_OK : S_FALSE;
}

HRESULT CEnumFormatEtc::Clone(IEnumFORMATETC **ppEnumFormatEtc)
{
   HRESULT hResult;

   // make a duplicate enumerator
   hResult = CreateEnumFormatEtc(formats_num_, format_etc_, ppEnumFormatEtc);

   if (hResult == S_OK)
     {
        // manually set the index state
        ((CEnumFormatEtc *)*ppEnumFormatEtc)->index_ = index_;
     }

   return hResult;
}

HRESULT CEnumFormatEtc::Next(ULONG celt, FORMATETC *pFormatEtc, ULONG *pceltFetched)
{
   ULONG copied = 0;

   // validate arguments
   if ((celt == 0) || (pFormatEtc == 0))
     return E_INVALIDARG;

   // copy the FORMATETC structures into the caller's buffer
   while (index_ < formats_num_ && copied < celt)
     {
        DeepCopyFormatEtc(&pFormatEtc[copied], &format_etc_[index_]);
        copied++;
        index_++;
     }

   // store result
   if (pceltFetched != 0)
     *pceltFetched = copied;

   // did we copy all that was requested?
   return (copied == celt) ? S_OK : S_FALSE;
}

// external functions

void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source)
{
   // copy the source FORMATETC into dest
   *dest = *source;

   if (source->ptd)
     {
        // allocate memory for the DVTARGETDEVICE if necessary
        dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));

        // copy the contents of the source DVTARGETDEVICE into dest->ptd
        *(dest->ptd) = *(source->ptd);
     }
}

HRESULT CreateEnumFormatEtc(UINT cfmt, FORMATETC *afmt, IEnumFORMATETC **ppEnumFormatEtc)
{
  if((cfmt == 0) || (afmt == 0) || (ppEnumFormatEtc == 0))
     return E_INVALIDARG;

   *ppEnumFormatEtc = new CEnumFormatEtc(afmt, cfmt);

   return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
}
