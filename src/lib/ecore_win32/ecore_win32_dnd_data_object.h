#ifndef __ECORE_WIN32_DND_DATA_OBJECT_H__
#define __ECORE_WIN32_DND_DATA_OBJECT_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <objbase.h>


class DataObject : public IDataObject
{
 private:

   LONG       ref_count_;
   int        formats_num_;
   FORMATETC *format_etc_;
   STGMEDIUM *stg_medium_;

 private: // internal helper function

   int lookup_format_etc(FORMATETC *format_etc);

 public: // structors

   DataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, int count);
   ~DataObject();

 public: // IUnknown

   HRESULT __stdcall QueryInterface(REFIID iid, void **ppvObject);
   ULONG   __stdcall AddRef();
   ULONG   __stdcall Release();

 public: // IDataObject

   HRESULT __stdcall GetData(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
   HRESULT __stdcall GetDataHere(FORMATETC *pFormatEtc, STGMEDIUM *pmedium);
   HRESULT __stdcall QueryGetData(FORMATETC *pFormatEtc);
   HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut);
   HRESULT __stdcall SetData(FORMATETC *pFormatEtc, STGMEDIUM *pMedium,  BOOL fRelease);
   HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
   HRESULT __stdcall DAdvise(FORMATETC *pFormatEtc,  DWORD advf, IAdviseSink *, DWORD *);
   HRESULT __stdcall DUnadvise(DWORD dwConnection);
   HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppEnumAdvise);
};


#endif /* __ECORE_WIN32_DND_DATA_OBJECT_H__ */
