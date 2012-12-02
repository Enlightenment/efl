#ifndef __ECORE_WIN32_DND_ENUMFORMATETC_H__
#define __ECORE_WIN32_DND_ENUMFORMATETC_H__


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <objbase.h>


class CEnumFormatEtc : public IEnumFORMATETC
{
 private:

   LONG       ref_count_;        // Reference count for this COM interface
   ULONG      index_;           // current enumerator index
   ULONG      formats_num_;      // number of FORMATETC members
   FORMATETC *format_etc_;       // array of FORMATETC objects

 public: // structors

   CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);

   ~CEnumFormatEtc();

 public: // IUnknown

   HRESULT __stdcall  QueryInterface (REFIID iid, void ** ppvObject);

   ULONG   __stdcall  AddRef (void);

   ULONG   __stdcall  Release (void);

 public: // IEnumFormatEtc

   HRESULT __stdcall  Next  (ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);

   HRESULT __stdcall  Skip  (ULONG celt);

   HRESULT __stdcall  Reset (void);

   HRESULT __stdcall  Clone (IEnumFORMATETC ** ppEnumFormatEtc);
};

void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source);

HRESULT CreateEnumFormatEtc(UINT cfmt, FORMATETC *afmt, IEnumFORMATETC **ppEnumFormatEtc);


#endif /* __ECORE_WIN32_DND_ENUMFORMATETC_H__ */
